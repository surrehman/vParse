/*
 * =====================================================================================
 *
 *       Filename:  sVerilog.c
 *
 *    Description:  Routines for operations on structural Verilog data type.
 *                  Note: These functions operate on the C structure "module"
 *                  and the like, not with structural verilog files.
 *
 *        Version:  1.0
 *        Created:  07/24/2008 11:16:27 AM
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
#include"sASIC_LUT.h"
#include"sVerilog.h"


struct dynArray *uINames ;
int moduleList2Verilog(struct dynArray *mList, char *outFilePath, int
    createLUTModels){
    // Write structural verilog for the list of modules passed
    int            i;
    struct module *mpntr;
    FILE          *outFilePointer = fopen(outFilePath, "w");

    if (outFilePointer == NULL){
        printf("-E- Could not write open file \"%s\"\n", outFilePath);
        return(1);
    }
    uINames  = newDynArray(T_STRING, 1);
    for(i=0; i<mList->index; i++){
        mpntr = ((struct module **)mList->data)[i];
        if (module2verilog(mpntr, outFilePointer, createLUTModels) == 1){
            printf("-E- Could not write structural verilog for module \"%s\"\n",
            mpntr->name);
            fclose(outFilePointer);
            return (1);
        }
        else{
            printf("-I- Structural verilog (%s) written to \"%s\"\n", 
            mpntr->name, outFilePath);
        }
    }
    return 0;
    // ------------------------------------------------------------------------
    // cleanup memory
    // ------------------------------------------------------------------------
    dynArrayFree(uINames,1);
}

int module2verilog(struct module *mpntr, FILE *outFilePointer, 
    int createLUTModels){
    // write structural verilog for the single module passed. 
    // THIS METHOD IS NOT TO BE CALLED ON IT'S OWN: use moduleList2Verilog()
    int           i;
    int           tIndex;
    struct conObj *cpntr;
    struct instance *ipntr;

    assert(outFilePointer != NULL);
    struct dynArray *uPortIndices = newDynArray(T_INTEGER, 1);
    struct dynArray *uWireIndices = newDynArray(T_INTEGER, 1);
    struct dynArray *uConObjNames = newDynArray(T_STRING , 1);
    // ------------------------------------------------------------------------
    // if required, generate LUT behavioural models
    // ------------------------------------------------------------------------
    if (createLUTModels==1){
        for(i=0;i<mpntr->instList->index;i++){
            ipntr = ((struct instance **)mpntr->instList->data)[i];
            if (dynArrayFind(uINames, ipntr->type) == -1){
                dynArrayAppend(uINames, ipntr->type);
                writeLUTModel(ipntr->type, outFilePointer);
            }
        }
    }
    // ------------------------------------------------------------------------
    // find out unique net names: wires and ports
    // ------------------------------------------------------------------------
    for(i=0;i<mpntr->conObjList->index;i++){
        cpntr = ((struct conObj **)mpntr->conObjList->data)[i];
        if (cpntr->isConst==1) continue;
        if (dynArrayFind(uConObjNames, cpntr->name) == -1){
            if (cpntr->isPort == 1)
                dynArrayAppend(uPortIndices, &i);
            else
                dynArrayAppend(uWireIndices, &i); 
            dynArrayAppend(uConObjNames, cpntr->name);
        }
    }
    // ------------------------------------------------------------------------
    // write module interface, declare ports 
    // ------------------------------------------------------------------------
    fprintf(outFilePointer, "module %s(\n", mpntr->name);
    for(i=0;i<uPortIndices->index;i++){
        tIndex = ((int *)uPortIndices->data)[i];
        cpntr  = ((struct conObj **)mpntr->conObjList->data)[tIndex];
        fprintf(outFilePointer, "    %s", cpntr->name);
        if (i==uPortIndices->index -1) fprintf(outFilePointer, ");\n\n");
        else fprintf(outFilePointer, ",\n");
    }
    // ------------------------------------------------------------------------
    // define ports
    // ------------------------------------------------------------------------
    for(i=0;i<uPortIndices->index;i++){
        tIndex = ((int *)uPortIndices->data)[i];
        cpntr  = ((struct conObj **)mpntr->conObjList->data)[tIndex];
        char dir[16];
        if (cpntr->direction == DIR_IN)  strcpy(dir, "    input");
        if (cpntr->direction == DIR_OUT) strcpy(dir, "    output");

        if (cpntr->slice[0] == 0 && cpntr->slice[1] == 0)
            fprintf(outFilePointer , "%s %s;\n", dir, cpntr->name);
        else
            fprintf(outFilePointer, "%s [%d:%d] %s;\n", dir, 
            cpntr->slice[0], cpntr->slice[1], cpntr->name);
    }
    fprintf(outFilePointer, "\n");
    // ------------------------------------------------------------------------
    // define wires
    // ------------------------------------------------------------------------
    for(i=0;i<uWireIndices->index;i++){
        tIndex = ((int *)uWireIndices->data)[i];
        cpntr  = ((struct conObj **)mpntr->conObjList->data)[tIndex];
        fprintf(outFilePointer, "    wire %s", cpntr->name);
        if (cpntr->slice[0] == 0 && cpntr->slice[1] == 0)
            fprintf(outFilePointer, ";\n");
        else fprintf(outFilePointer, "[%d:%d];\n", cpntr->slice[0],
            cpntr->slice[1]);
    }
    // ------------------------------------------------------------------------
    // define instances
    // ------------------------------------------------------------------------
    for(i=0;i<mpntr->instList->index;i++){
        ipntr=((struct instance **)mpntr->instList->data)[i];
        declareInstance(ipntr, outFilePointer);
    }
    // ------------------------------------------------------------------------
    // close module
    // ------------------------------------------------------------------------
    fprintf(outFilePointer, "endmodule\n");
    // ------------------------------------------------------------------------
    // cleanup memory
    // ------------------------------------------------------------------------
    dynArrayFree(uPortIndices, 1);
    dynArrayFree(uWireIndices, 1);
    dynArrayFree(uConObjNames, 1);

    return(0);
}
void declareInstance(struct instance *ipntr, FILE *fp){
    // Write the instance to the file pointer
    assert(ipntr->formalPortList->index == ipntr->actualPortList->index);
    fprintf(fp, "   %s %s(\n", ipntr->type, ipntr->name);
    int            i;
    char          *formal;
    struct conObj *actual;
    char          actualString[STR_LEN_MAX];
    for(i=0;i<ipntr->formalPortList->index;i++){
        formal=((char **)ipntr->formalPortList->data)[i];
        actual=((struct conObj **)ipntr->actualPortList->data)[i];
        conObjStringRep(actual, actualString);
        fprintf(fp, "        .%s(%s)", formal, actualString);
        if (i == ipntr->formalPortList->index -1) fprintf(fp, ");\n");
        else fprintf(fp, ",\n");
    }
}
int instance2Verilog(struct instance *ipntr, FILE *fp){
    // Write the given instance into the given stream

    int   i;
    char  tmpStr[STR_LEN_MAX];
    char *formal;

    fprintf(fp, "    %s %s(", ipntr->type, ipntr->name);
    if (ipntr->formalPortList->index != ipntr->actualPortList->index)
        return 1;
    for (i=0; i<ipntr->formalPortList->index;i++){
        formal = ((char **)ipntr->formalPortList->data)[i];
        conObjStringRep( ((struct conObj **)ipntr->actualPortList->data)[i],
        tmpStr);
        if (i != ipntr->formalPortList->index - 1)
            fprintf(fp, ".%s( %s ), ", formal, tmpStr);
        else fprintf(fp, ".%s( %s ));\n", formal, tmpStr);
    }
    return 0;
}
void PntrList2Module(struct dynArray *uConObjList, char *moduleName, FILE *stream){
    // Given a list of unique connection objects used in a design, write
    // module interface to give stream.
    int            i;
    struct conObj *cpntr;
    char           dir[8];

    struct dynArray *portNames = newDynArray(T_STRING, 1);

    // module interface
    fprintf(stream, "module %s (\n", moduleName);
    for (i=0; i<uConObjList->index; i++){
        cpntr = ((struct conObj **)uConObjList->data)[i];
        if (cpntr->isPort == 1) dynArrayAppend(portNames, cpntr->name);
    }
    for (i=0;i< portNames->index; i++){
        fprintf(stream, "    %s", ((char **)portNames->data)[i]);
        if (i == portNames->index -1) fprintf(stream, ");\n\n");
        else fprintf(stream, ",\n");
    }
    dynArrayFree(portNames,1);
    // module port and wire definitions
    for (i=0; i< uConObjList->index; i++){
        cpntr = ((struct conObj **)uConObjList->data)[i];
        if (cpntr->isPort == 1){
            if (cpntr->direction == DIR_IN) strcpy(dir, "input");
            if (cpntr->direction == DIR_OUT) strcpy(dir, "output");
        }
        else strcpy(dir, "wire");
        if (cpntr->slice[0] == 0 && cpntr->slice[1] == 0)
                fprintf(stream, "    %s %s;\n", dir, cpntr->name);
        else
            fprintf(stream, "    %s [%d:%d] %s;\n", dir, cpntr->slice[0],
            cpntr->slice[1], cpntr->name);
    }
}

