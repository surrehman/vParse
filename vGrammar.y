/*
 * =====================================================================================
 *
 *       Filename:  vGrammar.y
 *
 *    Description:  Bison grammar file for structured Verilog.
 *
 *        Version:  1.0
 *        Created:  Wed Jul 11 13:31:31 CEST 2007
 *       Revision:  none
 *       Compiler:  gcc
 *
 * =====================================================================================
 */

%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<sys/stat.h>
#include "vConstants.h"
#include "vDatatypes.h"
#include "vCompiler.h"
#include "vGrammar.tab.h"
extern FILE *yyin;
extern int yylineno;
extern char *yytext;
char currentFile[256];

// -----------------------------------------------------------------------------
//                                  LIMITATIONS
// -----------------------------------------------------------------------------
// 1. Verilog identifiers longer than 256 characters are not supported. To allow
//    identifiers with larger size, modify the macro STR_LEN_MAX in vConstants.h
// 2. There is limited support for concatenated variables. The compiler does not
//    support descending contatenations such as {A, B, {C, D}, {E, {F,G}}}
// ----------------------------------------------------------------------------
//                                  GLOBAL VARIABLES 
// ----------------------------------------------------------------------------
char G_NUMBER[STR_LEN_MAX];         // for handling verilog numbers
short G_DIR;                        // keep track of module's ports' direction
struct dynArray *STR_PORTNAMELIST;  // list of strings        (port declaration)
struct dynArray *OBJ_CONOBJLIST;    // list of con objs (ports &wire definition)
struct dynArray *STR_PORTLIST;      // list of names: used when more than one.
                                    // port/ wire are _defined_ on a single line
int G_SLICE[2];                     // range information in ports/ wires.
char G_SINGLEVAR[STR_LEN_MAX];      // place holder for the "single" object
struct dynArray *G_FORMAL;          // list of formals (per port basis);
struct dynArray *G_CONCATVARLIST;   // list of concatenated verilog variables
struct dynArray *P_ACTUALS;         // list of actual for single port: could be
                                    // a single actual or a number of actuals
struct portMapping *S_PMAP;         // port mapping for a single instance port
struct dynArray *G_INSTANTIATIONS;  // list of instantiations, where every thing
                                    // except the instance type is known. This
                                    // list contains instances which are
                                    // declared on a single line.
struct dynArray *G_MODULE_INST;     // list of instances which belong to a
                                    //  single module

struct dynArray *R_MODLIST;         // list of modules that are returned by 
                                    // parseNetlist(). 

void yyerror(const char *str) {
    printf("-E- Parse Error on line %d before token \'%s\'\n",yylineno, yytext);
    printf("-E- %s\n", str);
    exit(0);    /* This might be a bit too blunt */
}
void initModuleVariables(){
    STR_PORTNAMELIST = newDynArray(T_STRING, 1);
    OBJ_CONOBJLIST   = newDynArray(T_CONOBJ, 1);
    STR_PORTLIST     = newDynArray(T_STRING, 1);
    G_FORMAL         = newDynArray(T_STRING, 1);
    G_CONCATVARLIST  = newDynArray(T_STRING, 1);
    P_ACTUALS        = newDynArray(T_CONOBJ, 1);
    S_PMAP           = newPortMapping();
    G_INSTANTIATIONS = newDynArray(T_INSTANCE, 1);
    G_MODULE_INST    = newDynArray(T_INSTANCE, 1);
}
void freeModuleVariables(){
    dynArrayFree(STR_PORTNAMELIST, 1);
    // dynArrayFree(OBJ_CONOBJLIST); <-- This is free()'ed by caller!
    dynArrayFree(STR_PORTLIST, 1);
    dynArrayFree(G_FORMAL, 1);
    dynArrayFree(G_CONCATVARLIST, 1);
    dynArrayFree(P_ACTUALS, 1);
    portMappingFree(S_PMAP);
    dynArrayFree(G_INSTANTIATIONS, 1);
    //dynArrayFree(G_MODULE_INST);  <-- This is free()'ed by caller!
}
struct DynArray *parseNetlist(char *pathToFile){
    // ------------------------------------------------------------------------
    // Parse the netlist, create internal data representation of processed
    // modules. Return a dynArray of modules if successfull, NULL othewise
    // ------------------------------------------------------------------------
    struct stat stbuff;
    if( stat(pathToFile, &stbuff) !=0 ){
        printf("-E- Cannot stat file \"%s\"\n", pathToFile);
        return NULL;}
    
    printf("-W- Optimistically assuming %s is a valid netlist\n", pathToFile);
    yyin = fopen(pathToFile,"r");
    if (yyin == NULL){
        printf("-E- Could not open netlist for reading");
        printf(", cowardly refusing to proceed further.\n");
        return(NULL);
    }
    // ------------------------------------------------------------------------
    //      Initialize global variables used while compiling the module
    // ------------------------------------------------------------------------
    initModuleVariables();
    // ------------------------------------------------------------------------
    //  Prepare the processed module list to return
    // ------------------------------------------------------------------------
    R_MODLIST        = malloc(sizeof(struct dynArray));
    R_MODLIST->type  = T_MODULE;
    R_MODLIST->index = 0;
    R_MODLIST->max   = 0;
    R_MODLIST->data  = NULL;
    // ------------------------------------------------------------------------
    //                          Start the show !
    // ------------------------------------------------------------------------
    time_t startTime, endTime;
    time(&startTime);
    if (yyparse() == 0){
        printf("-D- Netlist compiled \"%s\"\n", pathToFile);
    }
    else{ 
        printf("-E- Error in compiling netlist \"%s\"\n", pathToFile);
        return (NULL);
    }
    fclose(yyin);
    freeModuleVariables();
    time(&endTime);
    printf("-I- Netlist compiled in %.4lf secs\n", difftime(endTime,
    startTime));
    return (struct dynArray *)(R_MODLIST);
}
%}
/*  overall general picture: 
    The parser works buttom-up: ports, wires and instances are detected before
    the module itself, therefore we store all that information in a symbol table.
    Once a module is detected, the symbol table is processed and a complete
    module structure is created. 
   
    In each action, the pseudo-variable $$ is the semantic value of the grouping
    that the rule is going to constuct. The semantic value of the components of
    the rule are referred to as $1, $2 and so on.
*/

/* the tokens passed by yylex() syntatically be int or char */
%union {
    int ival;
    char *string;
    char character;
}
%start sourceText
%locations
%token <string>     MODULE
%token <string>     ENDMODULE
%token <string>     INPUT
%token <string>     OUTPUT
%token <string>     INOUT
%token <string>     IDENTIFIER
%token <string>     EIDENTIFIER
%token <string>     INTEGER
%token <string>     FLOAT
%token <character>  EQUALS
%token <character>  SCOLON
%token <character>  COLON
%token <character>  POPEN
%token <character>  PCLOSE
%token <character>  COMMA
%token <character>  DOT
%token <character>  SOPEN
%token <character>  SCLOSE
%token <character>  ORTHOCORP
%token <character>  MINUS
%token <character>  PLUS
%token <character>  COPEN
%token <character>  CCLOSE
%token <character>  QMARK
%token <character>  APOSTROPHE
%token <string>     BASE
%token <string>     WIRE
%token <string>     TRI
%token <string>     SUPPLY0
%token <string>     SUPPLY1
%token <string>     SCALERED
%token <string>     VECTORED
%token <string>     ASSIGN
%token <string>     DEFPARAM
/* bison needs to know type of non-terminals */
%type <string>      sourceText
%type <string>      description
%type <string>      identifier
%type <string>      listOfPorts
%type <string>      portNamesList
%type <string>      portName
%type <string>      moduleItems
%type <string>      singleModuleItem
%type <string>      range
%type <string>      portDefinition
%type <string>      portVariableList
%type <string>      netType
%type <string>      moduleInstatiation
%type <string>      instanceType 
%type <string>      moduleInstanceList
%type <string>      instanceName
%type <string>      moduleConnectionList
%type <string>      implicitConnectionList
%type <string>      explicitConnectionList
%type <string>      concatenated
%type <string>      expressionList
%type <string>      variableName
%type <string>      number
%type <string>      unsignedNumber
%type <string>      decimalNumber

// These directives do not seem to work and one has to manually do a free($1),
// free($2) etc.
%destructor {free($$);} MODULE 
%destructor {free($$);} ENDMODULE 
%destructor {free($$);} INPUT 
%destructor {free($$);} OUTPUT 
%destructor {free($$);} INOUT 
%destructor {free($$);} IDENTIFIER 
%destructor {free($$);} EIDENTIFIER 
%destructor {free($$);} INTEGER 
%destructor {free($$);} FLOAT 
%destructor {free($$);} BASE 
%destructor {free($$);} WIRE 
%destructor {free($$);} TRI 
%destructor {free($$);} SUPPLY0
%destructor {free($$);} SUPPLY1 
%destructor {free($$);} SCALERED 
%destructor {free($$);} VECTORED 
%destructor {free($$);} ASSIGN 
%destructor {free($$);} DEFPARAM
%destructor {free($$);} identifier 
%destructor {free($$);} portName
%destructor {free($$);} range 
%destructor {free($$);} instanceName
%destructor {free($$);} variableName
%destructor {free($$);} unsignedNumber 
%destructor {free($$);} decimalNumber
%destructor {free($$);} number 
//%destructor {free($$);} expression
%%

sourceText :
    sourceText description
    |
    ;
description :
    MODULE
    identifier
    listOfPorts SCOLON
    moduleItems
    ENDMODULE {
    
    // ------------------------ Start Here -------------------------------
    // Use the following data structures to create a ``sanitized'' module:
    // OBJ_CONOBJLIST       : connection objects in current module
    // G_MODULE_INST        : instances          in current  module
    // G_ASSIGNMENTS        : assignments        in current module 
    // R_MODLIST             : DynArray of modules that is returned

    // 
    // for efficient memory utilization, contents of the symbol table are NOT
    // copied and returned, rather they are used as such to create the return
    // dynArray. 
    
    
    if (createSanitizedModule(R_MODLIST, OBJ_CONOBJLIST, G_MODULE_INST, $2))
        yyerror("Cannot sanitize module\n");

    freeModuleVariables();
    initModuleVariables();
    free($1);  
    free($2); 
    free($6);
};
identifier  :
    IDENTIFIER { 
        $$=$1;
        
    }
    |
    EIDENTIFIER {
        $$=$1;

       
    };

listOfPorts  :
    // listOfPorts: Declaration of module ports
    POPEN
    portNamesList 
    PCLOSE {
    }
;
portNamesList:
    portName{
        dynArrayAppend( STR_PORTNAMELIST, $1);
        free($1);
    }
    |
    portNamesList COMMA portName{
        dynArrayAppend( STR_PORTNAMELIST, $3);
        free($3);
    }
;
portName:
    IDENTIFIER {$$=$1};
moduleItems :
    // moduleItems is information about either the module's ports, nets,
    // instances, assignments etc.
    singleModuleItem 
    |
    moduleItems singleModuleItem;

singleModuleItem:
    portDefinition
    |
    moduleInstatiation
    |
    netDeclaration;

range   :
    SOPEN INTEGER COLON INTEGER SCLOSE{ 
        G_SLICE[0]=atoi($2);
        G_SLICE[1]=atoi($4);
        free($2);
        free($4);
};
// ----------------------------------------------------------------------------
//  Section: Module port _definitions_
// ----------------------------------------------------------------------------
portDefinition:
    direction range portVariableList SCOLON{
        // Names for ports are present in STR_PORTLIST, use it to create
        // connection objects in OBJ_CONOBJLIST
        createModConObjsFromRange(STR_PORTLIST, OBJ_CONOBJLIST, 
            1, G_DIR ,G_SLICE);
        dynArrayReset(STR_PORTLIST);
    }
    |
    direction portVariableList SCOLON {
        createModConObjsFromRange(STR_PORTLIST, OBJ_CONOBJLIST, 
            1, G_DIR ,G_SLICE);
        dynArrayReset(STR_PORTLIST);
    } 
;
portVariableList   :
    identifier{
        // is the port name here present in the declaration? Append to
        // STR_PORTNAMELIST if so, else raise error
        if (dynArrayFind(STR_PORTNAMELIST, $1) == -1){
            printf("-E- [%d]: Port not defined: %s\n",yylineno, $1); return(1);}
        else
            dynArrayAppend(STR_PORTLIST, $1);
        free($1);
    }
    |
    identifier COMMA portVariableList {
        if (dynArrayFind(STR_PORTNAMELIST, $1) == -1){
            printf("-E- [%d]: Port not defined: %s\n",yylineno, $1); return(1);}
        else{
            dynArrayAppend(STR_PORTLIST, $1);
            G_SLICE[0]=0; G_SLICE[1]=0;}
        free($1);
    }
;
direction :
    INPUT  {free($1); G_DIR = DIR_IN;}
    |
    OUTPUT {free($1); G_DIR = DIR_OUT;}
    |
    INOUT  {free($1); G_DIR = DIR_INOUT;}
;
// ----------------------------------------------------------------------------
//  Section: Module wire _declarations_
// ----------------------------------------------------------------------------
netDeclaration     :
    netType range netVariableList SCOLON {
        // net names are present in STR_PORTLIST: use them to add connection
        // objects (wires) to OBJ_CONOBJLIST.
        createModConObjsFromRange(STR_PORTLIST, OBJ_CONOBJLIST, 
            0, G_DIR ,G_SLICE);
        dynArrayReset(STR_PORTLIST);
        free($1);

    }
    |
    netType netVariableList SCOLON {
        G_SLICE[0] = 0; 
        G_SLICE[1] = 0;
        createModConObjsFromRange(STR_PORTLIST, OBJ_CONOBJLIST, 
            0, G_DIR ,G_SLICE);
        dynArrayReset(STR_PORTLIST);
        free($1);
    }
;
netVariableList   :
    identifier{
        dynArrayAppend(STR_PORTLIST, $1);
        free($1);
    }
    |
    netVariableList COMMA identifier { 
        dynArrayAppend(STR_PORTLIST, $3);
        free($3);
    }
;
netType :
    WIRE  { $$=$1;}
    |
    SUPPLY0
    |
    SUPPLY1
    |
    TRI
;
// ----------------------------------------------------------------------------
//  Section: Module instantiations
// ----------------------------------------------------------------------------
moduleInstatiation:
instanceType moduleInstanceList SCOLON{
    // Instances have been appened to G_INSTANTIATIONS, fix instance type,
    // append instances to G_MODULE_INST and reset the instances in
    // G_INSTANTIATIONS
    int i;
    for(i=0;i<G_INSTANTIATIONS->index;i++){
        strcpy( (((struct instance **)G_INSTANTIATIONS->data)[i])->type, $1);
        dynArrayAppend( G_MODULE_INST,
            ((struct instance **)G_INSTANTIATIONS->data)[i]);
    }
    dynArrayReset(G_INSTANTIATIONS); 
    free($1);
};
instanceType:
    identifier {$$=$1;        
    }
;

moduleInstanceList  :
    singleModuleInstance{

    }
    |
    moduleInstanceList COMMA singleModuleInstance {
    }
;
singleModuleInstance     :
    instanceName POPEN moduleConnectionList PCLOSE {
        // XXX: The instance's information (except the instance type is known:
        // append an instance in global variable G_INSTANTIATIONS.
        // free data structures associated with an instance's port mappings
        
        createInstantiation($1, S_PMAP, G_INSTANTIATIONS);
        portMappingReset(S_PMAP);
        dynArrayReset(G_FORMAL);
        dynArrayReset(P_ACTUALS);

        free($1);
    }
    |
    instanceName POPEN PCLOSE {
        // This instance has no port-mapping !!

    }
;
instanceName    :
    identifier {$$=$1;}
;
moduleConnectionList   :
    implicitConnectionList{ 
        printf("-W- This netlist contains implicit port mappings\n");
    }
    |
    explicitConnectionList{ 
    }
;

implicitConnectionList: 
    expression {
        // We don not know the formals, but only the actuals. The actuals have
        // been stored in P_ACTUALS.
        createImplicitPortMapping(S_PMAP, P_ACTUALS);
        dynArrayReset(P_ACTUALS);
        dynArrayReset(G_FORMAL);
    }
    |
    implicitConnectionList COMMA expression {
        createImplicitPortMapping(S_PMAP, P_ACTUALS);
        dynArrayReset(P_ACTUALS);
        dynArrayReset(G_FORMAL);
    }
;
explicitConnectionList :
    namedPortConnection
    |
    explicitConnectionList COMMA namedPortConnection
;
namedPortConnection :
    DOT IDENTIFIER POPEN expression PCLOSE { 
        // Actual(s) for this formal are in P_ACTUALS, Formal(s) in G_FORMAL;
        //
        // If number(formals) != number(actuals), concatenated actuals have been
        // specified. In order to get over this problem, the formals themselves
        // need to be expanded. The expansion is carried out in
        // createPortMapping()    
        dynArrayAppend(G_FORMAL, $2);
        createPortMapping(S_PMAP, G_FORMAL, P_ACTUALS);
        dynArrayReset(P_ACTUALS);
        dynArrayReset(G_FORMAL);
        free($2);
    }
    |
    DOT IDENTIFIER POPEN PCLOSE{
        // XXX port mapping without any actuals: open ports. This is not a
        // GoodThing: Warn on stdout
        printf("-W- No actual for port %s on line %d\n", $2, yylineno);

        createSingleActual(P_ACTUALS, "", 0);

        dynArrayAppend(G_FORMAL, $2);
        createPortMapping(S_PMAP, G_FORMAL, P_ACTUALS);
        dynArrayReset(P_ACTUALS);
        dynArrayReset(G_FORMAL);
        free($2);
    }
;
expression:
    single{
    }
    |
    concatenated{

    }
;
single:
    IDENTIFIER SOPEN INTEGER COLON INTEGER SCLOSE{
        // create multiple connection objects and append them all to P_ACTUALS
        createMulActual(P_ACTUALS, $1, atoi($3), atoi($5));
        free($1); free($3); free($5);
    }
    |
    IDENTIFIER SOPEN INTEGER SCLOSE{
        // create single connection objects and append them all to P_ACTUALS
        createSingleActual(P_ACTUALS, $1, atoi($3));
        free($1); free($3);
    }
    |
    variableName{
        // create single connection objects and append them all to P_ACTUALS
        createSingleActual(P_ACTUALS, $1, 0);
        free($1);
    }
    |
    number{
        // create single connection objects and append them all to P_ACTUALS
        createSingleActual(P_ACTUALS, G_NUMBER, 0);
        //free($1);
    }
;
concatenated:
    COPEN expressionList CCLOSE{


    }
;
expressionList:
    single{
    }
    |
    expressionList COMMA single{
    }
; 
variableName   :
    identifier {
        ($$=$1)
    }
;
number :
    unsignedNumber BASE unsignedNumber{
        //  <size><base_format><number>
        strcpy(G_NUMBER, $1);
        strcat(G_NUMBER, $2);
        strcat(G_NUMBER, $3);
        free($1); free($2); free($3);
    }
    |
    decimalNumber{
        $$=$1
    }
;
unsignedNumber :
    INTEGER {
        $$=$1
    }
;
decimalNumber :
    INTEGER {$$=$1}
    |
    PLUS INTEGER {$$=$2}
    |
    MINUS INTEGER {$$=strdup(strcat("-",$2))}
;

%%


