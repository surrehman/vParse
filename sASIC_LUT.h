/*
 *   This file was automatically generated by version 1.7 of cextract.
 *   Manual editing not recommended.
 */

#ifndef __CEXTRACT__
#ifdef __STDC__

extern int writeDFFModel ( FILE *fp );
extern int LUTFunctionFromType ( char *type, char *expr );
extern int writeLUTModel ( char *type, FILE *fp );
extern int writeLUTOutputFormal ( char *LUTType, char *targetStr );
extern int isLUTINV ( char *LUTType );
extern int isLUTBUFF ( char *LUTType );

#else /* __STDC__ */

extern int writeDFFModel ( FILE *fp );
extern int LUTFunctionFromType ( char *type, char *expr );
extern int writeLUTModel ( char *type, FILE *fp );
extern int writeLUTOutputFormal ( char *LUTType, char *targetStr );
extern int isLUTINV ( char *LUTType );
extern int isLUTBUFF ( char *LUTType );

#endif /* __STDC__ */
#endif /* __CEXTRACT__ */