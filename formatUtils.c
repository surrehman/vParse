/*
 * =====================================================================================
 *
 *       Filename:  formatUtils.c
 *
 *    Description:  Routines for formatting text printed to stdout/ verilog
 *                  files
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
#include<assert.h>
#include<time.h>

int writeBlockComment(char *message, FILE *fp){
    // write message inside block (C/C++ style) to given file
    int i;
    char *start=message; int linelength=0;
    char *line=malloc(sizeof(char)*87); assert(line!=NULL);strcpy(line, "");
    char *oline=line;
    fprintf(fp, "/");for(i=0;i<80;i++)fprintf(fp, "*");
    fprintf(fp, "\n* Auto generated code: manual editing not recommended");
    time_t mytime = time(0);
    fprintf(fp, "\n* Time stamp: %s", ctime(&mytime));
    fprintf(fp, "*\n");
    for(i=0;i<strlen(message);i++){
        if (message[i]=='\n'){
            strncpy(line, start, message+i - start);
            line[message+i-start]='\0';
            while (line[0] == ' ') line++; // no blank at start of line
            fprintf(fp, "* %s \n", line);
            linelength=0;
            start=message+i+1;
            linelength++;
            continue;
        }
        if (linelength==77){
            strncpy(line, start, sizeof(char)*77);
            line[78]='\0';
            while (line[0] == ' ') line++; // no blank at start of line
            fprintf(fp, "* %s\n", line);
            start += 77;
            linelength=0;
        }
        linelength++;
    }
    while (start[0] == ' ') start++; // no blank at start of line
    fprintf(fp, "* %s\n", start);
    for(i=0;i<80;i++) fprintf(fp,"*");
    fprintf(fp, "/\n");
    free(oline);
    return(0);
}
