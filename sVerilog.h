/*
 *   This file was automatically generated by version 1.7 of cextract.
 *   Manual editing not recommended.
 */

#ifndef __CEXTRACT__
#ifdef __STDC__

extern int moduleList2Verilog ( struct dynArray *mList, char *outFilePath, int createLUTModels );
extern int module2verilog ( struct module *mpntr, FILE *outFilePointer, int createLUTModels );
extern void declareInstance ( struct instance *ipntr, FILE *fp );
extern int instance2Verilog ( struct instance *ipntr, FILE *fp );
extern void PntrList2Module ( struct dynArray *uConObjList, char *moduleName, FILE *stream );

#else /* __STDC__ */

extern int moduleList2Verilog ( struct dynArray *mList, char *outFilePath, int createLUTModels );
extern int module2verilog ( struct module *mpntr, FILE *outFilePointer, int createLUTModels );
extern void declareInstance ( struct instance *ipntr, FILE *fp );
extern int instance2Verilog ( struct instance *ipntr, FILE *fp );
extern void PntrList2Module ( struct dynArray *uConObjList, char *moduleName, FILE *stream );

#endif /* __STDC__ */
#endif /* __CEXTRACT__ */
