/*
 * =====================================================================================
 *
 *       Filename:  vDatatypes.h
 *
 *    Description:  This file contains definitions for primitive datatypes and
 *                  corresponding functions for their manipulation, management
 *                  and transformations.
 *                  Function prototypes at the end of this file are generated by
 *                  a script whcih calls cextract.
 *
 *        Version:  $Version$
 *        Created:  07/15/2008 12:06:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 * =====================================================================================
 */

#include"vConstants.h"
struct dynArray{
    short   type;
    int     index;
    int     max;
    void  **data;
};
struct conObj{
    char            *name;
    struct instance *driver;
    struct module   *parent;
    int              index;
    short            isPort;
    short            direction;
    short            isConst;
    int              slice[2];
};
struct instance{
    char             *name;
    char             *type;
    struct dynArray  *formalPortList;
    struct dynArray  *actualPortList;
    struct conObj    *driven_net;
    struct dynArray  *portDirectionList;
};
struct module{
    char            *name;
    struct dynArray *instList;
    struct dynArray *conObjList;
    short            isSanitized;
};
struct st_node{
    short  type;
    void  *data;
};
struct table{
    int              index;
    int              max;
    struct st_node **node_list;
};
struct lNode{
    char       type;
    void      *parent;
    void     **children;
    unsigned   nChildren;
    char     **formals;
    char     **actuals;
    char      *mData;      /* type of instance/ name of port */
};
struct portMapping{
    struct dynArray *formals;
    struct dynArray *actuals;

};
struct forestMetaData{
    struct dynArray *con_obj_list;
    struct dynArray  uInstanceTypes;
    struct dynArray *DFFList;
    int              nDAG;
};

struct linkList{
    void            *data;
    unsigned         type;
    struct linkList *next;
    struct linkList *prev;
};

// Global function(s) in vGrammar.y
struct DynArray *parseNetlist(char *pathToFile);




/*
 *   This file was automatically generated by version 1.7 of cextract.
 *   Manual editing not recommended.
 */

#ifndef __CEXTRACT__
#ifdef __STDC__

struct dynArray *newDynArray ( short type, short size );
int dynArrayMalloc ( struct dynArray *apntr, short type, short size );
int dynArrayAppend ( struct dynArray *apntr, void *data );
void dynArrayPrint ( struct dynArray *apntr, int detail );
void dynArrayFree ( struct dynArray *apntr, int depth );
void dynArrayReset ( struct dynArray *apntr );
void dynArrayCopy ( struct dynArray *dest, struct dynArray *src );
int dynArrayFind ( struct dynArray *apntr, void *data );
struct conObj *newConObj ( void );
void conObjReset ( struct conObj *cpntr );
void conObjFree ( struct conObj *cpntr );
void conObjCopy ( struct conObj *dest, struct conObj *src );
void conObjPrint ( struct conObj *cpntr, FILE *stream );
void conObjStringRep ( struct conObj *cpntr, char *target );
struct instance *newInstance ( void );
void moduleReset ( struct module *mpntr );
void instanceFree ( struct instance *ipntr, unsigned depth );
void instanceReset ( struct instance *ipntr );
void instanceFreeFormals ( struct instance *ipntr );
void instanceFreeActuals ( struct instance *ipntr );
void instanceFreePortDirectionList ( struct instance *ipntr );
void instanceCopy ( struct instance *dest, struct instance *src );
void instancePrint ( struct instance *ipntr, FILE *stream );
struct module *newModule ( void );
void moduleCopy ( struct module *dest, struct module *src );
void moduleFree ( struct module *mpntr, int depth );
void modulePrint ( struct module *mpntr, FILE *stream, int verbose );
void printLine ( int full, FILE *stream );
void moduleFreeSanitizedList ( struct dynArray *mList );

#else /* __STDC__ */

struct dynArray *newDynArray ( short type, short size );
int dynArrayMalloc ( struct dynArray *apntr, short type, short size );
int dynArrayAppend ( struct dynArray *apntr, void *data );
void dynArrayPrint ( struct dynArray *apntr, int detail );
void dynArrayFree ( struct dynArray *apntr, int depth );
void dynArrayReset ( struct dynArray *apntr );
void dynArrayCopy ( struct dynArray *dest, struct dynArray *src );
int dynArrayFind ( struct dynArray *apntr, void *data );
struct conObj *newConObj ( void );
void conObjReset ( struct conObj *cpntr );
void conObjFree ( struct conObj *cpntr );
void conObjCopy ( struct conObj *dest, struct conObj *src );
void conObjPrint ( struct conObj *cpntr, FILE *stream );
void conObjStringRep ( struct conObj *cpntr, char *target );
struct instance *newInstance ( void );
void moduleReset ( struct module *mpntr );
void instanceFree ( struct instance *ipntr, unsigned depth );
void instanceReset ( struct instance *ipntr );
void instanceFreeFormals ( struct instance *ipntr );
void instanceFreeActuals ( struct instance *ipntr );
void instanceFreePortDirectionList ( struct instance *ipntr );
void instanceCopy ( struct instance *dest, struct instance *src );
void instancePrint ( struct instance *ipntr, FILE *stream );
struct module *newModule ( void );
void moduleCopy ( struct module *dest, struct module *src );
void moduleFree ( struct module *mpntr, int depth );
void modulePrint ( struct module *mpntr, FILE *stream, int verbose );
void printLine ( int full, FILE *stream );
void moduleFreeSanitizedList ( struct dynArray *mList );

#endif /* __STDC__ */
#endif /* __CEXTRACT__ */