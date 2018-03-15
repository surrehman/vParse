/*
 * =====================================================================================
 *
 *       Filename:  gensASICBehLib.c
 *
 *    Description:  generate behavioural Verilog models for sASIC LUTs/ DFFs
 *                  
 *
 *        Version:  1.0
 *        Created:  
 *       Revision:  none
 *       Compiler:  gcc
 *
 * =====================================================================================
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/stat.h>
#include"sASIC_LUT.h"
#include"formatUtils.h"

int main(int argc, char **argv){
    int i;
    char LUTtype[16];
    FILE *outFilePointer;

    outFilePointer = fopen("sASIC_lib.v", "w");
    writeBlockComment("Behavioural description of sASIC library (LUT2+DFF)",
    outFilePointer);


    writeDFFModel(outFilePointer);
    for( i=0 ; i<17;i++){
        sprintf(LUTtype, "LUT2_MUX_%02d", i);
        writeLUTModel(LUTtype, outFilePointer);
    }
    fclose(outFilePointer);
    return 0;
}



