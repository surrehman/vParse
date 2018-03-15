/*
 * =====================================================================================
 *
 *       Filename:  vParser.c
 *
 *    Description:  Read input verilog file, compile it and write summary of
 *                  modules in the file.
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
#include"vDatatypes.h"
#include"vConstants.h"
#include"sASIC_LUT.h"
#include"sVerilog.h"
#include"build.h"
#include"vTransform.h"

void displayUsage(void){
    printf("-I- vparser: Read input verilog, compile & print summary of modules\n");
    printf("    Build time: %s\n", buildTime);
    printf("-I- Usage:\n");
    printf("    vparser {switches} [input]\n");
    printf("    switches:\n");
    printf("    -v      : verbose\n");
    printf("    -h      : help\n");
    exit(EXIT_FAILURE);
}
int main(int argc, char **argv){
    char *optString         = "vh";
    int opt                 = 0;
    optind                  = 0;
    char **myargv           = NULL;
    int myargc              = 0;
    int verbosity           = 0;

    opt = getopt(argc, argv, optString);
    while(opt != -1){
        switch(opt){
            case 'h':
                displayUsage();
                break;
            case 'v':
                verbosity = 1;
                break;
        }
        opt = getopt(argc, argv, optString);
    }

    myargv = argv + optind;
    myargc = argc - optind;

    if (myargc != 1){
        printf("-E- Invalid arguments\n");
        displayUsage();
    }
    struct dynArray *mList;
    // ------------------------------------------------------------------------
    //                                               parser and compile netlist
    // ------------------------------------------------------------------------
    mList = (struct dynArray *)parseNetlist( myargv[0]);
    // ------------------------------------------------------------------------
    //                                     if there was trouble, exit right now
    // ------------------------------------------------------------------------
    if (mList == NULL) exit(EXIT_FAILURE);
    int i; struct module *mpntr;
    for(i=0;i<mList->index;i++){
        mpntr = ((struct module **)mList->data)[i];
        // --------------------------------------------------------------------
        //                                                         print module
        // --------------------------------------------------------------------
        modulePrint(mpntr, stdout, verbosity);
        // --------------------------------------------------------------------
        //                                              test for DAG extraction
        // --------------------------------------------------------------------
        // module2Forest(mpntr, 0, NULL);
        // --------------------------------------------------------------------
        //                                                   free() used memory 
        // --------------------------------------------------------------------
        moduleFree(mpntr, 0);
    }
    free(mList->data);
    free(mList);
    exit(EXIT_SUCCESS);
}
