/*
 * =====================================================================================
 *
 *       Filename:  vTokens.c
 *
 *    Description:  Description of verilog tokens which would be produced by the
 *                  lexical analyzor
 *
 *        Version:  
 *        Created:  
 *       Revision:  none
 *      Generator:  flex
 *       Compiler:  gcc
 *
 * =====================================================================================
 */
%{
#include<stdio.h>
#include <string.h>
#include "vGrammar.tab.h"
extern YYSTYPE yylval;
extern YYLTYPE yylloc;
long int n_lines=0;

%}
    /* options for lexical generator */
%option yylineno
%option noyymore
%x comment
%%
    int line_num=1;
"/*"                        BEGIN(comment);
<comment>[^*\n]*       /* eat anything that is not a '*' */
<comment>"*"+[^*/\n]*  /* eat up '*'s followed by '/'s   */
<comment>\n            ++line_num;
<comment>"*"+"/"       BEGIN(INITIAL);
module                 yylval.string=strdup(yytext);   return MODULE;
endmodule              yylval.string=strdup(yytext);   return ENDMODULE; 
input                  yylval.string=strdup(yytext);   return INPUT; 
output                 yylval.string=strdup(yytext);   return OUTPUT; 
inout                  yylval.string=strdup(yytext);   return INOUT; 
wire                   yylval.string=strdup(yytext);   return WIRE;
tri                    yylval.string=strdup(yytext);   return TRI;
supply0                yylval.string=strdup(yytext);   return SUPPLY0;
supply1                yylval.string=strdup(yytext);   return SUPPLY1;
scalared               yylval.string=strdup(yytext);   return SCALERED;
vectored               yylval.string=strdup(yytext);   return VECTORED;
assign                 yylval.string=strdup(yytext);   return ASSIGN;  
defparam               yylval.string=strdup(yytext);   return DEFPARAM;  
[0-9]+                 yylval.string=strdup(yytext);   return INTEGER;
[0-9]+\.[0-9]+         yylval.string=strdup(yytext);   return FLOAT;
=                      yylval.character=atoi(yytext);  return EQUALS;
\(                     yylval.character=atoi(yytext);  return POPEN;
\)                     yylval.character=atoi(yytext);  return PCLOSE;
;                      yylval.character=atoi(yytext);  return SCOLON;
:                      yylval.character=atoi(yytext);  return COLON;
,                      yylval.character=atoi(yytext);  return COMMA;
\.                     yylval.character=atoi(yytext);  return DOT;
\[                     yylval.character=atoi(yytext);  return SOPEN;
\]                     yylval.character=atoi(yytext);  return SCLOSE;
#                      yylval.character=atoi(yytext);  return ORTHOCORP; 
-                      yylval.character=atoi(yytext);  return MINUS; 
\+                     yylval.character=atoi(yytext);  return PLUS; 
\{                     yylval.character=atoi(yytext);  return COPEN; 
\}                     yylval.character=atoi(yytext);  return CCLOSE; 
\"                     yylval.character=atoi(yytext);  return QMARK; 
'                      yylval.character=atoi(yytext);  return APOSTROPHE;
'b                     yylval.string=strdup(yytext);   return BASE;
'B                     yylval.string=strdup(yytext);   return BASE;
[a-zA-Z_][a-zA-Z0-9_$]* yylval.string=strdup(yytext); return IDENTIFIER;
\\[^ \t\b\f\r]+        yylval.string=strdup(yytext);  return EIDENTIFIER;
[\n]                   ; n_lines++;
"//".*                 ; /* simple //style comments are easiler to handle */
[[:blank:]]            ; /* ignore white space/ blank spaces */
[[:space:]]            ; 


%%
int yywrap(void){
    /* what to do after the tokens have finished comming ? */
    printf("-I- Input tokens finished: %ld lines read\n", n_lines);
    return 1;
}
