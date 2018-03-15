/*
 * =====================================================================================
 *
 *       Filename:  vCompiler.c
 *
 *    Description:  Routines used in verilog compiler. Mostly related to symbol
 *                  tree manipulation and handling
 *
 *        Version:  1.0
 *        Created:  07/17/2008 06:33:05 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 * =====================================================================================
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<time.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/stat.h>

#include"vDatatypes.h"
#include"vConstants.h"
#include"vCompiler.h"

extern int yylineno;
void createModConObj(char *name, struct dynArray *targetObjList, 
    int  index, short isPort, short direction, int slice[2]){
    // create a single connection object a from the name given to given list of
    // connection objects
    struct conObj *cpntr =  newConObj();
    strcpy(cpntr->name, name);
    cpntr->index     = index;
    cpntr->isPort    = isPort;
    cpntr->direction = direction;
    cpntr->slice[0]  = slice[0];
    cpntr->slice[1]  = slice[1];
    dynArrayAppend(targetObjList, cpntr);
    conObjFree(cpntr);
}
void createModConObjs(struct dynArray *nameList, struct dynArray *targetObjList, 
    int  index, short isPort, short direction, int slice[2]){
    // create multiple connection objects representing module ports/ wires 
    // from the given name list to the given target connection object list
    int i;
    for(i=0;i<nameList->index;i++)
        createModConObj( ((char **)nameList->data)[i], targetObjList, index,
        isPort, direction, slice);
}
void createModConObjsFromRange(struct dynArray *nameList, struct dynArray
    *targetObjList, short isPort, short direction,int slice[2]){
    // Create multiple connection object (ports/ wires) for the given range of
    // indices.
    int i, start, end;
    if (slice[0] == slice[1]){
        createModConObjs(nameList, targetObjList, 0, isPort, direction,
        slice);
        return;
    }
    if (slice[0] > slice[1] ) {start = slice[1]; end = slice[0];}
    else {start = slice[0]; end = slice[1];};
    for(i= start; i<= end; i++){
        createModConObjs(nameList, targetObjList, i, isPort, direction,
        slice);
    }
}
void createSingleActual(struct dynArray *targetList, char *name, int index){
    // create a single connection object in the give targetList
    struct conObj *tObj = newConObj();
    strcpy( tObj->name, name);
    tObj->index         = index;
    dynArrayAppend(targetList, tObj);
    conObjFree(tObj);
}
void createMulActual(struct dynArray *targetList, char *name, int start, 
    int end) {
    // create multiple connection objects in the given targetList
    int i, min, max;
    if (start > end) {min = end;   max = start;}
    if (end> start)  {min = start; max = end;}
    for(i=min; i <=max; i++) createSingleActual(targetList, name, i);
}
struct portMapping *newPortMapping(){
    struct portMapping *ppntr = malloc(sizeof(struct portMapping));
    assert(ppntr != NULL);
    ppntr->formals            = newDynArray(T_STRING, 1);
    ppntr->actuals            = newDynArray(T_CONOBJ, 1);
    return ppntr;
}
void portMappingFree(struct portMapping *ppntr){
    dynArrayFree(ppntr->formals, 1);
    dynArrayFree(ppntr->actuals, 1);
    free(ppntr);
}
void portMappingReset(struct portMapping *ppntr){
    ppntr->formals->index=0;
    ppntr->actuals->index=0;
}
void createPortMapping(struct portMapping *ppntr, struct dynArray *formals,
    struct dynArray *actuals){
    // The argument "formals" passed to this function is a single port formal,
    // where as the argument "actual" is the corresponding actual.
    // If n(formals) != n(actuals), then expand the formal into bus wires, then
    // append them to the port mapping structure.
    if (formals->index != actuals->index){
        // create bus wires from the formal
        int i; char tName[STR_LEN_MAX];
        for(i=0; i< actuals->index; i++){
            sprintf(tName, "%s[%d]", 
                ((char **)formals->data)[0],i);
            dynArrayAppend(ppntr->formals,tName);
            dynArrayAppend(ppntr->actuals,((struct conObj **)actuals->data)[i]);
        }
    }
    else{
        assert(formals->index == 1 && actuals->index == 1);
        // simply append then members into port mapping structure.
        dynArrayAppend(ppntr->formals, ((char **)formals->data)[0]);
        dynArrayAppend(ppntr->actuals, ((struct conObj **)actuals->data)[0]);
    }
}
void createImplicitPortMapping(struct portMapping *ppntr, struct dynArray
*actuals){
    // dynArray is a list of actuals, based  on which, we create a port mapping
    int i;
    for(i=0;i<actuals->index;i++){
        dynArrayAppend(ppntr->formals, "IMPLIED_FORMAL");
        dynArrayAppend(ppntr->actuals, ((struct conObj **)actuals->data)[i]);
    }
}
void createInstantiation(char *name, struct portMapping *pMap, struct dynArray
    *targetList ){
    // From information in port mapping, create an instance and append it to
    // targetList
    struct instance *tInstance  = newInstance();
    struct conObj   *actualPort;
    char            *formalPort;
    assert(pMap->formals->index ==  pMap->actuals->index);
    int i;
    for(i=0;i<pMap->formals->index;i++){
        formalPort = ((char **)pMap->formals->data)[i];
        actualPort = ((struct conObj **)pMap->actuals->data)[i];
        dynArrayAppend(tInstance->formalPortList, formalPort);
        dynArrayAppend(tInstance->actualPortList, actualPort);
    }
    strcpy(tInstance->name, name);
    dynArrayAppend(targetList, tInstance);
    instanceFree(tInstance, 1);
}

int createSanitizedModule(struct dynArray *targetList, struct dynArray
    *modConObjList, struct dynArray *modInstList, char *name){
    // ------------------------------------------------------------------------
    // The following pointers used in symbol table are not free() 'ed:
    //
    //     OBJ_CONOBJLIST       G_INSTANTIATIONS    G_ASSIGNMENTS
    //
    // rather they're used as members of the list of modules that are returned
    // ------------------------------------------------------------------------
    printf("-I- Compiling module \"%s\"\n", name);
    if (targetList->index <= targetList->max){
        if (targetList->index == 0){
            // processing first module
            targetList->data = malloc(sizeof(struct module *) *
            DYN_ARRAY_DELTA);
            assert(targetList->data != NULL);
            targetList->max  = DYN_ARRAY_DELTA;
        }
        else{
            // processing subsequent modules
            targetList->data = realloc(targetList->data, 
            sizeof(struct module *) * (targetList->index + 1));
            assert(targetList->data != NULL);
            targetList->max = targetList->index + 1;
        }
    }
    int              i;
    struct module   *mpntr;
    struct instance *ipntr;
    
    targetList->data[targetList->index] = malloc(sizeof(struct module));
    mpntr                               = targetList->data[targetList->index];
    mpntr->name                         = malloc(sizeof(char)*STR_LEN_MAX);
    assert(mpntr->name != NULL);
    strcpy(mpntr->name , name);

    mpntr->conObjList                   = modConObjList;
    mpntr->instList                     = modInstList;
    // sanitize instances

    for(i = 0; i < modInstList->index; i++){
        ipntr = ((struct instance **)modInstList->data)[i];
        if(instanceSanitize(ipntr, modConObjList) == 1)
            return (1);
    }

    mpntr->isSanitized                  = 0;
    targetList->index                  += 1;
    return (0);
}
int instanceSanitize(struct instance *ipntr, struct dynArray *conObjList){
    int            i;
    struct conObj *actual;
    char          *formal;
    int            index  = 0;

    assert( ipntr->formalPortList->index == ipntr->actualPortList->index);

    for(i=0;i<ipntr->formalPortList->index;i++){
        actual = ((struct conObj **)ipntr->actualPortList->data)[i];
        formal = ((char **)ipntr->formalPortList->data)[i];
        index  = dynArrayFind(conObjList, actual);
        if (index  == -1){
            // ----------------------------------------------------------------
            // If we've encountered a verilog constant number, append it as a
            // connection object in conObjList. Similarly if there was an open
            // port mapping (conObj->name == ""), create a conObj for denoting
            // an non-present port actual
            // ----------------------------------------------------------------
            if ( strcmp(actual->name, "1'b0") == 0 
                ||
                strcmp(actual->name,"1'b1") == 0){
                struct conObj *constNet = newConObj();
                constNet->index   = 0;
                constNet->isConst = 1;
                strcpy(constNet->name, actual->name);
                dynArrayAppend(conObjList, constNet);
                conObjFree(constNet);
                free(actual->name);
                free(actual);
                ((struct conObj **)ipntr->actualPortList->data)[i] = 
                ((struct conObj **)conObjList->data)[conObjList->index -1];
            }
            else if (strcmp(actual->name, "") == 0){
                struct conObj *noNet = newConObj();
                noNet->index   = 0;
                noNet->isConst = 1;
                strcpy(noNet->name, "");
                dynArrayAppend(conObjList, noNet);
                conObjFree(noNet);
                free(actual->name);
                free(actual);
                ((struct conObj **)ipntr->actualPortList->data)[i] = 
                ((struct conObj **)conObjList->data)[conObjList->index -1];
            }
            else{
                printf("-D- Cannot find %s[%d] in module declarations\n",
                actual->name, actual->index);
                return(1);
            }
            // --------------------------------------------------------------
        }
        else{
            // ----------------------------------------------------------------
            // fix struct conObj's driver member: for this we need to know if
            // the formal port is an input/ output port
            // XXX There could be a better way of finding out if the formal is
            // an output port XXX
            // ----------------------------------------------------------------
            if (strcmp(formal, "YN") ==0 
                || 
                strcmp(formal, "Z") ==0
                || 
                strcmp(formal, "Q") ==0){
                (((struct conObj **)conObjList->data)[index])->driver = ipntr;
                (((struct conObj **)conObjList->data)[index])->isConst= 0;
            }
            free(actual->name);
            free(actual);
            ((struct conObj **)ipntr->actualPortList->data)[i] = 
            ((struct conObj **)conObjList->data)[index];
        }
    }
    return(0);
}
void dbgPrintSinglePortMapping(struct portMapping *ppntr){
    // print the port mapping for a single formal port:
    assert(ppntr->formals->index == ppntr->actuals->index);
    int i;
    for(i=0; i< ppntr->formals->index; i++){
        printf(" %s -> %s[%d]\n",
        ((char **)ppntr->formals->data)[i],
        (((struct conObj **)ppntr->actuals->data)[i])->name,
        (((struct conObj **)ppntr->actuals->data)[i])->index);
    }
}
