/*
 * =====================================================================================
 *
 *       Filename:  testcase.c
 *
 *    Description:  File for checking out code functionality
 *
 *        Version:  1.0
 *        Created:  07/15/2008 10:15:29 PM
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
#include"sVerilog.h"
#include"vGrammar.tab.h"

void simpleModulePrint(struct module *mpntr){
    printf("-D- Module information for \"%s\":\n", mpntr->name);
    int i, j;
    struct conObj *cpntr;
    for(i=0;i<mpntr->conObjList->index;i++){
        cpntr=((struct conObj **)mpntr->conObjList->data)[i];
        if (cpntr->index == 0 && cpntr->slice[1] == 0 && cpntr->slice[0] == 1)
            printf("-D- ConObj #%2d: %s\n", i, cpntr->name);
        else
            printf("-D- ConObj #%2d: %s[%d]\n", i, cpntr->name, cpntr->index);
        
    }
    struct instance *ipntr;
    char            *formal;
    struct conObj   *actual;
    for(i=0;i<mpntr->instList->index;i++){
        ipntr=((struct instance **)mpntr->instList->data)[i];
        printf("-D- Instance mapping for instance \"%s\" type: %s\n",
        ipntr->name, ipntr->type);
        for(j=0;j<ipntr->formalPortList->index;j++){
            formal = ((char **)ipntr->formalPortList->data)[j];
            actual = ((struct conObj **)ipntr->actualPortList->data)[j];
            printf("\t%s --> %s[%d]\n", formal, actual->name, actual->index);

        }
    }
}

void printModuleConObjInfo(struct module *mpntr){
    // print information related to a module's connection objects
    int i; struct conObj *cpntr;
    for(i=0; i<mpntr->conObjList->index; i++){
        cpntr = ((struct conObj**)mpntr->conObjList->data)[i];
        if (cpntr->isConst ==1) continue;
        printf("-I- ConObj # %3d ", i);
        if (cpntr->isPort ==1)
            printf(" PORT %10s", cpntr->name);
        else printf(" WIRE %10s", cpntr->name);
        if (!(cpntr->slice[0] == 0 && cpntr->slice[1] == 0))
            printf("[%d]", cpntr->index);
        if (cpntr->isPort == 1){
            if (cpntr->direction == DIR_IN)
                printf(" Direction : input\n");
            else if (cpntr->direction == DIR_OUT)
                printf(" Direction : output\n");
            else printf(" UNKNOWN DIRECTION\n");
        }
        else    printf(" Driver: %p\n", cpntr->driver);
    }
}

int main(int argc, char **argv){
    int i;
    for(i=0;i<33;i++) {                         //
        if (i<strlen("DataType")) printf(" ");  //
        else printf("-"); }                     //
    printf("\n");                               //      Formatting
    printf("%16s %16s\n", "DataType", "Size");  //
    for(i=0;i<33;i++) {                         //
        if (i<strlen("DataType")) printf(" ");  //
        else printf("-"); }                     //
    printf("\n");                               //
    printf("%16s %16ld\n", "char", sizeof(char));
    printf("%16s %16ld\n", "short", sizeof(short));
    printf("%16s %16ld\n", "unsigned", sizeof(unsigned));
    printf("%16s %16ld\n", "int", sizeof(int));
    printf("%16s %16ld\n", "float", sizeof(float));
    printf("%16s %16ld\n", "long", sizeof(long));
    printf("%16s %16ld\n", "long int", sizeof(long int));
    printf("%16s %16ld\n", "double", sizeof(double));


    printf("%16s %16ld\n", "struct dynArray", sizeof(struct dynArray));
    printf("%16s %16ld\n", "struct module", sizeof(struct module));
    printf("%16s %16ld\n", "struct instance", sizeof(struct instance));
    printf("%16s %16ld\n", "struct conObj", sizeof(struct conObj));
    printf("%16s %16ld\n", "struct lnode", sizeof(struct lNode));


    printf("%16s %16ld\n", "int *", sizeof(int *));
    printf("%16s %16ld\n", "struct dynArray *", sizeof(struct dynArray *));
    printf("%16s %16ld\n", "struct module *", sizeof(struct module *));
    printf("%16s %16ld\n", "struct instance *", sizeof(struct instance *));
    printf("%16s %16ld\n", "struct conObj *", sizeof(struct conObj *));
    printf("%16s %16ld\n", "struct lnode *", sizeof(struct lNode *));
    return (EXIT_SUCCESS);
    /* test for char datatypes */
}


