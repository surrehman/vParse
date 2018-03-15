/*
 * =====================================================================================
 *
 *       Filename:  verilog2verilog.c
 *
 *    Description:  Read input verilog file, compile it and write the same
 *                  structural file with LUT modles. This is a sanity check for
 *                  compilation of input verilog file.
 *                  
 *
 *        Version:  1.0
 *        Created:  $(Date)
 *       Revision:  $(Revision)
 *       Compiler:  gcc
 *
 * =====================================================================================
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<assert.h>
#include"vDatatypes.h"
#include"vConstants.h"
#include"sASIC_LUT.h"
#include"sVerilog.h"
#include"build.h"


void displayUsage(void){
    printf("-I- verilog2verilog: Read input verilog, compile & write to file\n");
    printf("    Build time: %s\n", buildTime);
    printf("-I- Usage:\n");
    printf("    verilog2verilog {switches} [input] [ouput]\n");
    printf("    switches:\n");
    printf("    -m      : generate behavioural models for LUTs/ DFF\n");
    printf("    -h      : help\n");
    exit(EXIT_FAILURE);
}
int main(int argc, char **argv){
    char *optString         = "mh";
    int opt                 = 0;
    optind                  = 0;
    char **myargv           = NULL;
    int myargc              = 0;
    unsigned generateModels = 0;

    opt = getopt(argc, argv, optString);
    while(opt != -1){
        switch(opt){
            case 'h':
                displayUsage();
                break;
            case 'm':
                generateModels = 1;
        }
        opt = getopt(argc, argv, optString);
    }

    myargv = argv + optind;
    myargc = argc - optind;

    if (myargc != 2){
        printf("-E- Incorrect arguments\n");
        displayUsage();
    }
    // we have 2 arguments left : input and output netlists
    struct dynArray *mList;
    // ------------------------------------------------------------------------
    //                                               parser and compile netlist
    // ------------------------------------------------------------------------
    printf("-D- About to parse %s\n", myargv[0]); 
    mList = (struct dynArray *)parseNetlist(myargv[0]); 
    // ------------------------------------------------------------------------
    //                                     if there was trouble, exit right now
    // ------------------------------------------------------------------------
    if (mList == NULL) exit(EXIT_FAILURE);
    // ------------------------------------------------------------------------
    //                                                     write output netlist
    // ------------------------------------------------------------------------
    moduleList2Verilog(mList, myargv[1], generateModels); 
    // ------------------------------------------------------------------------
    //                                                              free memory
    // ------------------------------------------------------------------------
    int i; struct module *mpntr;
    for(i=0;i<mList->index;i++){
        mpntr = ((struct module **)mList->data)[i];
        moduleFree(mpntr, 0);
    }
    free(mList->data);
    free(mList);
    exit(EXIT_SUCCESS);
}
