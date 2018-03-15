/* name:            strutils.c
   purpose:         generic string manipulation functions
   author:          urRehman
   dependencies:    None (apart from ISO C libs)
*/
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<assert.h>
#include<math.h>
#include"strUtils.h"

void rinplace(char *haystack, const char *needle, const char *sub){
    /* replace "in-place" *needle in the *haystack by *sub. *haystack is assumed
     * large enough to store the result.
       Internally the function mallocs a buffer and stores the result of the
       substitution. The orignal haystack is overwritten with this buffer's
       contents. Memory used up by the buffer is freed before the function
       returns */
    if (strstr(haystack,needle)==NULL) return;
    assert(strcmp(needle,""));
    char *p; 
    char *mybuffer=malloc(sizeof(char)*(5 *strlen(haystack))); 
    assert(mybuffer!=NULL);mybuffer[0]='\0';
    char *start=(char *)haystack;
    p=strstr(start,needle);
    while(p!=NULL){
        strncat(mybuffer,start,p-start);
        strcat(mybuffer,sub);
        start=p+strlen(needle);
        p=strstr(start,needle);
    }
    strcat(mybuffer,start);
    strcpy(haystack, mybuffer);
    free(mybuffer);
}

void rinplace_huge(char *haystack, const char *needle, const char *sub){
    /* This version of rinplace is the same as the previous version, it
     * allocates a massive memory chunk for tmp buffer*/
    if (strstr(haystack,needle)==NULL) return;
    assert(strcmp(needle,""));
    char *p; 
    char *mybuffer=malloc(sizeof(char)*(5L* strlen(sub) *strlen(haystack))); 
    //printf("-D- Buffersize requested: %ld\n",  sizeof(char)*(1000 *strlen(haystack)));
    while (mybuffer==NULL){
        mybuffer=malloc(sizeof(char)*(strlen(sub) *strlen(haystack))); 
        if (mybuffer!= NULL) break;
    }
    assert(mybuffer!=NULL);
    mybuffer[0]='\0';
    char *start=(char *)haystack;
    p=strstr(start,needle);
    while(p!=NULL){
        strncat(mybuffer,start,p-start);
        strcat(mybuffer,sub);
        start=p+strlen(needle);
        p=strstr(start,needle);
    }
    strcat(mybuffer,start);
    strcpy(haystack, mybuffer);
    free(mybuffer);
}

