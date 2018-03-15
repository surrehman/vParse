/*
 * =====================================================================================
 *
 *       Filename:  sASIC_LUT.c
 *
 *    Description:  Routines for handling and operating on sASIC LUTs.
 *
 *        Version:  1.0
 *        Created:  07/24/2008 04:01:27 PM
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
#include"sASIC_lib.h"
#include"strUtils.h"


int writeDFFModel(FILE *fp){
    // Copy DFF model in sASIC_DFF.v to given file pointer. 
    struct stat stbuff;
    if(stat("sASIC_DFF.v", &stbuff) != 0){
        printf("-E- Cannot stat file \"sASIC_DFF.v\"\n");
        return 1;
    }
    char *DFFdata = calloc( (stbuff.st_size + 1), sizeof(char));
    // Why stbuff.st_size + 1 and not just stbuff.st_size ?
    assert(DFFdata != NULL);
    FILE *DFFfp   = fopen("sASIC_DFF.v","r"); assert(DFFfp   != NULL);

    fread(DFFdata, sizeof(char), stbuff.st_size, DFFfp);
    fprintf(fp,"%s",DFFdata);
    free(DFFdata);
    return 0; 
}
int  LUTFunctionFromType(char *type, char *expr){
    // Given LUT type, write verilog expression for its function to given string
    // Return value of 1 denotes a failure.
    if (strcmp(type, sASIC_DFF_NAME)==0) return 1;
    int findex0,findex1,findex2,findex3;  int n_LUT;
    char i0[3],i1[3],i2[3],i3[3];
    if (strncmp(type, "LUT2_MUX", 8) ==0)   n_LUT=2;
    if (strncmp(type, "LUT3_"   , 5) ==0)   n_LUT=3;
    if (strncmp(type, "LUT4_"   , 5) ==0)   n_LUT=4;
    switch(n_LUT){
    case(2):
        // net expression for LUT2. (beware of inverters and buffers)
        findex0=atoi((type)+9);
        strcpy(expr,fLUT2Verilog[findex0]);
        break;
    case(3):
        i0[0]   = type[5];i0[1]  = type[6]; i0[2] = '\0';
        i1[0]   = type[7];i1[1]  = type[8]; i1[2] = '\0';
        findex0 = atoi(i0);
        findex1 = atoi(i1);
        strcpy(expr, fLUT3);
        rinplace(expr, " f0 " , fLUT2Verilog[findex0]);
        rinplace(expr, " f1 " , fLUT2Verilog[findex1]);
        break;
    case(4):
        i0[0]   = type[5]; i0[1]  = type[6];  i0[2] = '\0';
        i1[0]   = type[7]; i1[1]  = type[8];  i1[2] = '\0';
        i2[0]   = type[9]; i2[1]  = type[10]; i2[2] = '\0';
        i3[0]   = type[11];i3[1]  = type[12]; i3[2] = '\0';
        findex0 = atoi(i0); findex1 = atoi(i1);
        findex2 = atoi(i2); findex3 = atoi(i3);
        strcpy(expr, fLUT4);
        rinplace(expr, " f0 " , fLUT2Verilog[findex0]);
        rinplace(expr, " f1 " , fLUT2Verilog[findex1]);
        rinplace(expr, " f2 " , fLUT2Verilog[findex2]);
        rinplace(expr, " f3 " , fLUT2Verilog[findex3]);
        break;
    }
    return 0;
}
int writeLUTModel(char *type, FILE *fp){
    // Given the instance type, write verilog behavioural model to given file
    // pointer. The instance could be a LUT or a DFF
    char expr[STR_LEN_MAX];
    if (strcmp(type, sASIC_DFF_NAME)==0) {
        if (writeDFFModel(fp)!=0){
            printf("-W- Unable to include register model from sASIC_DFF.v!\n");
            return 1; }
        else return 0;
    } 
    fprintf(fp, "module %s(", type);
    if (strncmp(type, "LUT2_MUX", 8)==0){
        if ( strcmp(type, "LUT2_MUX_05")==0 || strcmp(type, "LUT2_MUX_06")==0 ){
            // input port: A0
            fprintf(fp, "A0,YN);\n    input A0;\n    output YN;\n    ");
            // expression for LUT2
            assert(LUTFunctionFromType(type, expr)!=1);
            fprintf(fp, "assign YN=%s;\n", expr);
        }
        else if(strcmp(type, "LUT2_MUX_09")==0 || strcmp(type, "LUT2_MUX_10")==0){
            // input port: A1
            fprintf(fp, "A1,YN);\n    input A1;\n    output YN;\n    ");
            // expression for LUT2
            assert(LUTFunctionFromType(type, expr)!=1);
            fprintf(fp, "assign YN=%s;\n", expr);
        }
        else if(strcmp(type, "LUT2_MUX_16")==0){
            // input port: A0, A1, D
            fprintf(fp, "A0, A1, S, YN);\n");
            fprintf(fp, "    input A0, A1, S;\n");
            fprintf(fp, "    output YN;\n    ");
            // expression for LUT2
            assert(LUTFunctionFromType(type, expr)!=1);
            fprintf(fp, "assign YN=%s;\n", expr);
        }

        else{
            // input port: A0, A1
            fprintf(fp, "A0, A1, YN);\n");
            fprintf(fp, "    input A0, A1;\n    output YN;\n    ");
            // expression for LUT2
            assert(LUTFunctionFromType(type, expr)!=1);
            fprintf(fp, "assign YN=%s;\n", expr);
        }
    }
    if (strncmp(type, "LUT3_", 5)==0){
        fprintf(fp, "A0, A1, A2, Z);\n");
        fprintf(fp, "    input A0, A1;\n    input A2;\n");
        fprintf(fp, "    output Z;\n    ");
        // expression for LUT3
        assert(LUTFunctionFromType(type, expr)!=1);
        fprintf(fp, "assign YN=%s\n", expr);
    }
    if (strncmp(type, "LUT4_", 5)==0){
        printf("-W- Verilog model for %s NOT written\n",type);
        return 1;
    }

    fprintf(fp, "endmodule \n");
    return 0;
}
int writeLUTOutputFormal(char *LUTType, char *targetStr){
    // Write the output of the LUT's output port
    if (strstr(LUTType, "LUT2_MUX_") != NULL) {
        strcpy(targetStr, "YN"); return 0;
    }
    if (strstr(LUTType, "LUT3_") != NULL) {
        strcpy(targetStr, "Z"); return 0;
    }
    if (strstr(LUTType, "LUT4_") != NULL) {
        strcpy(targetStr, "Z"); return 0;
    }
    return 1;
}


int isLUTINV(char *LUTType){
    // Is the given LUT type an inverter?
    if (strcmp(LUTType, "LUT2_MUX_06") == 0 || 
        strcmp(LUTType, "LUT2_MUX_10") == 0)
        return 1;
    else
        return 0;
}
int isLUTBUFF(char *LUTType){
    // Is the given LUT type a buffer
    if (strcmp(LUTType, "LUT2_MUX_05") == 0 || 
        strcmp(LUTType, "LUT2_MUX_09")== 0 )
        return 1;
    else
        return 0;
}
