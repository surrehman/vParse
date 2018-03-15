/*
 * =====================================================================================
 *
 *       Filename:  vDatatypes.c
 *
 *    Description:  Routines for handling basic data types.
 *
 *        Version:  1.0
 *        Created:  07/15/2008 12:27:48 PM
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
// ----------------------------------------------------------------------------
// Section: Dynamic array functions
// ----------------------------------------------------------------------------
struct dynArray *newDynArray(short type, short size){
    // allocate memory and initialize a dynamic array
    struct dynArray *apntr = malloc(sizeof(struct dynArray));
    assert(apntr!=NULL);
    apntr->index           = 0;
    apntr->max             = 0;
    apntr->type            = type;
    assert( dynArrayMalloc(apntr, type, size) == 0);
    return apntr;
}
int dynArrayMalloc(struct dynArray *apntr, short type, short size){
    // allocate storage memory for dynamic array's data members

    int i, delta;
    if (size == 0) delta = DYN_ARRAY_DELTA/2;
    else           delta = DYN_ARRAY_DELTA;

    if (apntr->index == 0 && apntr->max == 0){
        // first allocation: malloc()
        if (apntr->type == T_INTEGER){
            apntr->data = malloc(sizeof(int *) * delta);
            assert(apntr->data != NULL);
        }
        if (apntr->type == T_CONOBJ_PNTR){
            apntr->data = malloc(sizeof(struct conObj *) * delta);
            assert(apntr->data != NULL);
        }
        if (apntr->type==T_STRING){
            apntr->data=malloc(sizeof(char *) * delta);
            for(i=0;i<delta;i++){
                ((char **)apntr->data)[i] = malloc(sizeof(char)*STR_LEN_MAX);
                assert( ((char **)apntr->data)[i] != NULL);
                strcpy( ((char **)apntr->data)[i], "NULL");
            }
        }
        if (apntr->type==T_CONOBJ){
            apntr->data = malloc(sizeof(struct conObj *)*delta);
            assert( apntr->data != NULL);
            for(i=0;i<delta;i++)
                ((struct conObj **)apntr->data)[i]=(void *)newConObj();
        }
        if (apntr->type==T_INSTANCE){
            apntr->data=malloc(sizeof(struct instance *)*delta);
            assert( apntr->data != NULL);
            for(i=0;i<delta;i++)
                ((struct instance **)apntr->data)[i]=(void *)newInstance();
        }
        if (apntr->type==T_MODULE){
            apntr->data = malloc(sizeof(struct module *) * delta);
            assert( apntr->data != NULL);
            for(i=0;i<delta;i++)
                ((struct module **)apntr->data)[i]= (void *) newModule();
        }
        apntr->max=delta;
    }
    else{
        // not the first allocation: realloc()
        if (apntr->type == T_INTEGER){
            apntr->data=realloc(apntr->data, sizeof(int *)*((apntr->index) +
            delta));
            assert ( apntr->data != NULL);
        }
        if (apntr->type == T_CONOBJ_PNTR){
            apntr->data=realloc(apntr->data, sizeof(struct conObj *)*((apntr->index) +
            delta));
            assert ( apntr->data != NULL);
        }
        if (apntr->type == T_STRING){
            apntr->data=realloc(apntr->data, sizeof(char *)*((apntr->index) +
            delta));
            assert (apntr->data != NULL);
            for(i=apntr->index;i< (apntr->index + delta);i++){
                ((char **)apntr->data)[i]=malloc(sizeof(char)*STR_LEN_MAX);
                assert( ((char **)apntr->data)[i] != NULL);
                strcpy( ((char **)apntr->data)[i], "NULL");
            }
        }
        if (apntr->type == T_CONOBJ){
            apntr->data=realloc(apntr->data, sizeof(struct conObj *) *
            (apntr->index + delta));
            assert(apntr->data != NULL);
            for(i=apntr->index;i<(apntr->index + delta);i++){
                ((struct conObj **)apntr->data)[i]= newConObj();
            }
        }
        if (apntr->type == T_INSTANCE){
            apntr->data=realloc(apntr->data, sizeof(struct instance *) *
            (apntr->index + delta));
            assert(apntr->data != NULL);
            for(i=apntr->index;i<(apntr->index + delta);i++){
                ((struct instance **)apntr->data)[i]= newInstance();
            }
        }
        if (apntr->type == T_MODULE){
            apntr->data=realloc(apntr->data, sizeof(struct module *) *
            (apntr->index + delta));
            assert(apntr->data != NULL);
            for(i=apntr->index;i<(apntr->index + delta);i++){
                ((struct module **)apntr->data)[i]= newModule();
            }
        }
        apntr->max=apntr->index + delta;
    }
    return 0;
}
int dynArrayAppend(struct dynArray *apntr, void *data){
    // append a copy of data passed to dynamic array
    if (apntr->index < apntr->max){
        if (apntr->type == T_INTEGER){
            ((int *)apntr->data)[apntr->index]=*((int *)data);
        }
        if (apntr->type == T_CONOBJ_PNTR){
            ((struct conObj **)apntr->data)[apntr->index]= (struct conObj *)data ;
        }
        if (apntr->type == T_STRING){
            strcpy(((char **)apntr->data)[apntr->index], (char *)data);
        }
        if (apntr->type == T_CONOBJ){
            conObjCopy( ((struct conObj **)apntr->data)[apntr->index], (struct conObj *)data);
        }
        if (apntr->type == T_INSTANCE){
            instanceCopy( ((struct instance **)apntr->data)[apntr->index],
                (struct instance *)data);
        }
        if (apntr->type == T_MODULE){
            printLine(1,stdout);
            printf("-W- dynArrayAppend() called for a module list !!\n");
            printf("-W- THIS SHALL NOT YIELD DESIRED RESULTS\n");
            printLine(1,stdout);
        }
        apntr->index +=1;
    }
    else{
        dynArrayMalloc(apntr, apntr->type, 1);
        dynArrayAppend(apntr, data);
    }
    return 0;
}
void dynArrayPrint(struct dynArray *apntr, int detail){
    int i;
    char lenString[256];
    char maxString[256];
    sprintf(lenString, "-I- Number of elements: %d\n", apntr->index);
    sprintf(maxString, "-I- Capacity          : %d\n", apntr->max);
    switch (apntr->type){
        case (T_INTEGER):
            printf("-I- Type : T_INTEGER\n");
            printf("%s", lenString);
            printf("%s", maxString);
            break;
        case (T_STRING):
            printf("-I- Type : T_STRING\n");
            printf("%s", lenString);
            printf("%s", maxString);
            if (detail == DETAIL_MAX){
                for(i=0;i<apntr->index;i++){
                    printf("-I- Element %d: %s\n", i, 
                    ((char **)apntr->data)[i]);
                }
            }
            break;
        case(T_CONOBJ):
            printf("-I- Type : T_CONOBJ\n");
            printf("%s", lenString);
            printf("%s", maxString);
            for(i=0;i<apntr->index;i++){
                if ((((struct conObj **)apntr->data)[i])->isPort)
                    printf("-D- Port: ");
                else printf("-D- Wire: ");
                printf("%s",  (((struct conObj **)apntr->data)[i])->name);
                if ((((struct conObj **)apntr->data)[i])->index != 0)
                    printf("[%d]\n", 
                    (((struct conObj **)apntr->data)[i])->index);
                else printf("\n");
            }
            break;
    }
        
}
void dynArrayFree(struct  dynArray *apntr, int depth){
    // free data used by dynamic array's member and the struct itself
    int i;
    switch(apntr->type){
        case (T_INTEGER):
            free(apntr->data);
            free(apntr);
            break;
        case (T_CONOBJ_PNTR):
            free(apntr->data);
            free(apntr);
            break;
        case (T_STRING):
            for(i=0;i<apntr->max;i++)
                free(((char **)apntr->data)[i]);
            free(apntr->data);
            free(apntr);
            break;
        case (T_CONOBJ):
            for(i=0;i<apntr->max;i++)
                conObjFree(((struct conObj **)apntr->data)[i]);
            free(apntr->data);
            free(apntr);
            break;
        case (T_INSTANCE):
            for(i=0;i<apntr->max;i++){
                instanceFree( ((struct instance **)apntr->data)[i], depth);
            }
            free(apntr->data);
            free(apntr);
            break;
        case (T_MODULE):
            for(i=0;i<apntr->max;i++){
                moduleFree( ((struct module **)apntr->data)[i], depth);
            }
            free(apntr->data); 
            free(apntr);
            break;
    }
}
void dynArrayReset(struct dynArray *apntr){
    // Resest the index member back to zero, so apntr->data elements can be over
    // written.
    if (apntr->type == T_INTEGER){
        apntr->index = 0;
    }
    if (apntr->type == T_CONOBJ_PNTR){
        apntr->index = 0;
    }
    if (apntr->type == T_STRING){
        apntr->index = 0;
    }
    if (apntr->type == T_CONOBJ){
        int i;
        for(i=0;i<apntr->index;i++) 
            conObjReset( ((struct conObj **)apntr->data)[i]);
        apntr->index = 0;
    }
    if (apntr->type == T_INSTANCE){
        int i;
        for(i=0;i<apntr->index;i++)
            instanceReset( ((struct instance **)apntr->data)[i]);
        apntr->index = 0;

    }
}

void dynArrayCopy(struct dynArray *dest, struct dynArray *src){
    // Append all members of src into dest.
    assert(src->type == dest->type);
    int i;
    switch(src->type){
        case(T_INTEGER):
        for(i=0;i<src->index;i++)
            dynArrayAppend(dest, ((int *)src->data)+i);
        break;

        case(T_STRING):
        for(i=0;i<src->index;i++)
            dynArrayAppend(dest, ((char **)src->data)[i]);
        break;

        case(T_CONOBJ):
        for(i=0;i<src->index;i++)
            dynArrayAppend(dest, ((struct conObj **)src->data)[i]);
        break;

        case(T_INSTANCE):
        for(i=0;i<src->index;i++)
            dynArrayAppend(dest, ((struct instance **)src->data)[i]);
        break;

        case(T_MODULE):
        for(i=0;i<src->index;i++)
            dynArrayAppend(dest, ((struct module **)src->data)[i]);
        break;
    }
}
int dynArrayFind(struct dynArray *apntr, void *data){
    //  Return index of given data in dynamic array, if present. Else return -1.
    //  This probably the most inefficient of all search methods
    int i;
    for(i=0; i<apntr->index; i++){
        if (apntr->type==T_INTEGER){
            if (((int *)apntr->data)[i] == *((int *)data)) return i;
        }
        if (apntr->type==T_CONOBJ_PNTR){
            struct conObj *cpntr = (struct conObj *)apntr->data[i];
            if (cpntr == (struct conObj *)data ) return i;
        }
        if (apntr->type == T_STRING){
            if (strcmp( ((char **)apntr->data)[i], (char *)data) == 0) return i;
        }
        if (apntr->type == T_CONOBJ){
            if (strcmp( (((struct conObj **)apntr->data)[i] )->name, 
                ((struct conObj *)data)->name) == 0){
                if ( (((struct conObj **)apntr->data)[i])->index ==
                    ((struct conObj *)data)->index){
                    return i;
                }
            }
        }
    }
    return -1;
}

// ----------------------------------------------------------------------------
// Section: conObj functions
// ----------------------------------------------------------------------------
struct conObj *newConObj(void){
    // allocate mem, initialize a connection object
    struct conObj *cpntr=malloc( sizeof( struct conObj));
    assert(cpntr!=NULL);
    cpntr->name=malloc(sizeof( char) * STR_LEN_MAX);
    assert(cpntr->name != NULL);
    strcpy(cpntr->name, "");
    cpntr->index            = -1;
    cpntr->isPort           = -1;
    cpntr->direction        = -1;
    cpntr->isConst          = -1;
    (cpntr->slice)[0]       = -1;
    (cpntr->slice)[1]       = -1;
    cpntr->driver           = NULL;
    return cpntr;
}
void conObjReset(struct conObj *cpntr){
    strcpy(cpntr->name, "");
    cpntr->index            = -1;
    cpntr->isPort           = -1;
    cpntr->direction        = -1;
    cpntr->isConst          = -1;
    (cpntr->slice)[0]       = -1;
    (cpntr->slice)[1]       = -1;
    cpntr->driver           = NULL;
}
void conObjFree(struct conObj *cpntr){
    free(cpntr->name);
    free(cpntr);
}
void conObjCopy(struct conObj *dest, struct conObj *src){
    strcpy(dest->name, src->name);
    dest->index      = src->index;
    dest->isPort     = src->isPort;
    dest->direction  = src->direction;
    dest->isConst    = src->isConst;
    dest->slice[0]   = src->slice[0];
    dest->slice[1]   = src->slice[1];
    dest->driver     = src->driver;
}
void conObjPrint(struct conObj *cpntr, FILE *stream){
    if (cpntr->slice[0] == 0 && cpntr->slice[1] == 0)
        fprintf(stream, "%s", cpntr->name);
    else fprintf(stream, "%s[%d]", cpntr->name, cpntr->index);
}
void conObjStringRep(struct conObj *cpntr, char *target){
    // string verilog representation of a connection object. "target" must have
    // sufficient storage space before this function is called
    sprintf(target, "");
    if ((cpntr->slice[0] == 0 && cpntr->slice[1] == 0) || (cpntr->isConst ==1))
        sprintf(target, "%s", cpntr->name);
    else sprintf(target , "%s[%d]" , cpntr->name, cpntr->index);
}
// ----------------------------------------------------------------------------
// Section: instance functions
// ----------------------------------------------------------------------------
struct instance *newInstance(void){
    struct instance *ipntr=malloc(sizeof(struct instance));
    assert(ipntr!=NULL);
    ipntr->name=malloc( sizeof(char)*STR_LEN_MAX); assert( ipntr->name!=NULL);
    strcpy(ipntr->name, "NULL");
    ipntr->type=malloc( sizeof(char)*STR_LEN_MAX); assert( ipntr->type!=NULL);
    strcpy(ipntr->type, "NULL");
    ipntr->portDirectionList = newDynArray( T_INTEGER, 0);
    ipntr->formalPortList    = newDynArray( T_STRING,  0);
    ipntr->actualPortList    = newDynArray( T_CONOBJ,  0);
    return ipntr;
}
void moduleReset(struct module *mpntr){
    ;
}
void instanceFree(struct instance *ipntr, unsigned depth){
    // free module takenup by an instance. If depth==1, do a full monty else do
    // not try to free the instance's actuals.
    free(ipntr->name);
    free(ipntr->type);
    instanceFreeFormals(ipntr);
    instanceFreePortDirectionList(ipntr);
    if (depth==1){ instanceFreeActuals(ipntr); }
    else{
        int i;
        for(i=ipntr->actualPortList->index; i< ipntr->actualPortList->max; i++)
            conObjFree( ((struct conObj **)ipntr->actualPortList->data)[i]);
        free(ipntr->actualPortList->data);
        free(ipntr->actualPortList);
    }
    free(ipntr);
}
void instanceReset(struct instance *ipntr){
    // reset the instance so it can be overwritten by some other value
    strcpy(ipntr->name, "NULL");
    strcpy(ipntr->type, "NULL");
    dynArrayReset(ipntr->portDirectionList);
    dynArrayReset(ipntr->formalPortList);
    dynArrayReset(ipntr->actualPortList);
}
void instanceFreeFormals(struct instance *ipntr){
    dynArrayFree(ipntr->formalPortList, 1);
}
void instanceFreeActuals(struct instance *ipntr){
    dynArrayFree(ipntr->actualPortList, 1);
}
void instanceFreePortDirectionList(struct instance *ipntr){
    dynArrayFree(ipntr->portDirectionList, 1);    
}
void instanceCopy(struct instance *dest, struct instance *src){
    // _deep copy_ instance from src to dest. both instances are initialized
    // with newInstance()
    strcpy(dest->name, src->name);
    strcpy(dest->type, src->type);

    int            i;
    struct conObj *actual;
    char          *formal;

    for(i=0; i< src->formalPortList->index; i++){
        formal = (((char **)(src->formalPortList->data)))[i];
        dynArrayAppend(dest->formalPortList, formal);
    }
    for(i=0; i< src->actualPortList->index; i++){
        actual = ((struct conObj **)src->actualPortList->data)[i];
        dynArrayAppend(dest->actualPortList, actual);
    }
    dest->driven_net  = src->driven_net;
    for(i=0;i<src->portDirectionList->index;i++){
        dynArrayAppend(dest->portDirectionList,
        &(((int **)src->portDirectionList->data)[i]));
    }
    // FIXME: what about an instance's mInfo??
}
void instancePrint(struct instance *ipntr, FILE *stream){
    // Print all information about instance to stream
    fprintf(stream, "%s (%s)\n", ipntr->name, ipntr->type);
    int  i;
    char tString[STR_LEN_MAX];
    for(i=0;i<ipntr->formalPortList->index;i++){
        conObjStringRep( ((struct conObj **)ipntr->actualPortList->data)[i],
        tString);
        printf("%s <--> %s\n", ((char **)ipntr->formalPortList->data)[i],
        tString);
    }
    printLine(0,stream);
}
// ----------------------------------------------------------------------------
// Section: module functions
// ----------------------------------------------------------------------------
struct module *newModule(void){
    struct module *mpntr = malloc(sizeof(struct module));
    assert(mpntr!=NULL);
    mpntr->name          = malloc(sizeof(char)*STR_LEN_MAX);
    assert( mpntr->name  != NULL);
    strcpy( mpntr->name, "NULL");
    mpntr->instList      = newDynArray( T_INSTANCE, 1);
    mpntr->conObjList    = newDynArray( T_CONOBJ, 1);
    return mpntr;
}
void moduleCopy(struct module *dest, struct module *src){
    // deep copy module from src to destination

    strcpy(dest->name, src->name);
    int i;
    // connection objects
    for(i=0 ; i < src->conObjList->index ; i++){
        dynArrayAppend(dest->conObjList, 
        ((struct conObj **)src->conObjList->data)[i]);
    }
    // instance objects
    for(i=0 ; i<  src->instList->index ; i++){
        dynArrayAppend(dest->instList,
        ((struct instance **)src->instList->data)[i]);
    }
}
void moduleFree(struct module *mpntr, int depth){
    // free memory taken up by a modules, EXHAUSTIVELY 
    free(mpntr->name);
    dynArrayFree(mpntr->instList, depth);
    dynArrayFree(mpntr->conObjList, depth);
    free(mpntr); 
}
void modulePrint(struct module *mpntr, FILE *stream, int verbose){
    // print a summary of instances and nets in a module
    int i;
    int nPorts    = 0;
    int nWires    = 0;
    int nDangling = 0;
    struct conObj *cpntr;  
    if (verbose) printLine(1, stdout);
    if (verbose) 
        fprintf(stream, "Connection objects in module %s\n", mpntr->name);
    if (verbose)
        printLine(1, stdout);
    char tString[64];
    for(i=0;i<mpntr->conObjList->index;i++){
        cpntr = ((struct conObj **)mpntr->conObjList->data)[i];
        conObjStringRep(cpntr, tString);
        if (cpntr->isPort == 1){
            nPorts++; 
            if(verbose){
                fprintf(stream, "(%d) Port %-16s", i+1, tString);
                fprintf(stream, "Direction:");
                if (cpntr->direction == DIR_IN) fprintf(stream, "IN ");
                if (cpntr->direction == DIR_OUT) fprintf(stream, "OUT ");
                if (cpntr->driver != NULL) fprintf(stream,
                    " Driver: %s(%s)\n",
                    cpntr->driver->name, cpntr->driver->type);
                else fprintf(stream, "\n");
            }
        }
        else{
            nWires++;
            if (verbose) {
                fprintf(stream, "(%d) Wire %-16s", i+1, tString);
            }
            if (cpntr->driver != NULL) {
                if (verbose) fprintf(stream, "Driver = %s(%s)\n",
                cpntr->driver->name, cpntr->driver->type);
            }
            else{
                if (verbose)
                    fprintf(stream, "Driver = %16s", "NONE (Dangling net)\n");
                nDangling++;
            }

        }
    }
    if (verbose==1){
        printLine(1, stream);
        fprintf(stream, "Module Instances (toplevel: %s)\n", mpntr->name);
        for(i=0; i< mpntr->instList->index;i++){
            fprintf(stream, "Instance #%d\n", i+1);
            instancePrint(((struct instance **)mpntr->instList->data)[i],
            stream);
        }
    }
    printLine(1, stream);
    fprintf(stream, "-I- Toplevel module Summary for %s\n", mpntr->name);
    printLine(1, stream);
    fprintf(stream, "# of Module ports  : %-16d\n", nPorts);
    fprintf(stream, "# of Module nets   : %-16d\n", nWires);
    fprintf(stream, "# of Dangling nets : %-16d\n", nDangling);
    fprintf(stream, "Total              : %-16d\n", nPorts + nWires);
    fprintf(stream, "ConObjList->index  : %-16d\n" , mpntr->conObjList->index);
    fprintf(stream, "# of Instances     : %-16d\n",  mpntr->instList->index);
    printLine(1, stream);
}

void printLine(int full, FILE *stream){
    int i;
    if (!full){ 
        if (stream==stdout){
            for(i=0;i<41;i++) fprintf(stream, "-");
            fprintf(stream,"\n");
        }
        else{
            fprintf(stream, "/*");
            for(i=0;i<37;i++) fprintf(stream,"-");
            fprintf(stream, "*/\n");
        }
    }
    else{ 
        if (stream==stdout){
            for(i=0;i<81;i++) fprintf(stream,"-");
            fprintf(stream, "\n"); 
        }
        else{
            fprintf(stream, "/*");
            for(i=0;i<77;i++) fprintf(stream,"-");
            fprintf(stream, "*/\n");
 
        }
    }
}
void moduleFreeSanitizedList(struct dynArray *mList){
    // Free memory from a list of sanitized modules
    int            i;
    struct module *mpntr;
    for( i=0; i<mList->max; i++){
        mpntr = ((struct module **)mList->data)[i];
        if (i < mList->index)
            moduleFree(mpntr, 0);
    }
    free(mList->data);
    free(mList);
}

