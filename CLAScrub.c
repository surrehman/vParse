/*
 * =====================================================================================
 *
 *       Filename:  CLAScrub.c
 *
 *    Description:  Read & compile input verilog into a forest of DAGs, remove
 *                  BUFF/ INV and write the result as a structural verilog
 *              
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
    printf("-I- cla_scrub: Read & compile input verilog into forest of DAGs");
    printf(",\nremove BUFF/ INV and write the result as structural verilog\n");
    printf("    Build time: %s\n", buildTime);
    printf("-I- Usage:\n");
    printf("               cla_scrub {switches} [input]\n");
    printf("    switches:\n");
    printf("    -h      : help\n");
    exit(EXIT_FAILURE);
}
int main(int argc, char **argv){
    char *optString         = "h";
    int opt                 = 0;
    optind                  = 0;
    char **myargv           = NULL;
    int myargc              = 0;
    char ofile[256];

    opt = getopt(argc, argv, optString);
    while(opt != -1){
        switch(opt){
            case 'h':
                displayUsage();
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
    for (i=0;i<mList->index;i++){
        mpntr = ((struct module **)mList->data)[i];
        // --------------------------------------------------------------------
        //                           DAG extraction and reconversion to verilog
        // --------------------------------------------------------------------
        strcat(mpntr->name, "_SCRUBBED");
        sprintf(ofile, "%s.v", mpntr->name);
        module2Forest2(mpntr, 0,0,1,NULL,ofile, NULL);


        // --------------------------------------------------------------------
        //                                                   free() used memory 
        // --------------------------------------------------------------------
        moduleFree(mpntr, 0);
    }
    free(mList->data);
    free(mList);
    exit(EXIT_SUCCESS);
}
