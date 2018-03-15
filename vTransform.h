/*
 *   This file was automatically generated by version 1.7 of cextract.
 *   Manual editing not recommended.
 */

#ifndef __CEXTRACT__
#ifdef __STDC__

extern struct lNode *newlNode ( char type );
extern void lnodeMallocChild ( struct lNode *cnode );
extern void lNodeFree ( struct lNode *root );
extern void lNodeFreeHack ( struct lNode *ProxyRoot );
extern void lNodePrint ( struct lNode *cnode );
extern struct lNode *net2DAG ( void *data, void *parent, char type );
extern void lNodeAppendFormal ( struct lNode *cnode, char *formal );
extern struct lNode *net2lNode ( void *data, void *parent, char type );
extern void lnodeMallocChild2 ( struct lNode *cnode );
extern void releaseNode ( struct lNode *cnode, FILE *stream );
extern int module2Forest2 ( struct module *mpntr, int visualize, int verify, int scrub, char *visFilePath, char *scrubFilePath, char *verifyFilePath );
extern int lNodeScrub ( struct lNode *cnode, FILE *stream );
extern int findIndexOfChild ( struct lNode *child, struct lNode *parent );
extern char invNodeFunction ( struct lNode *cnode, char *target );
extern int invNodeInput ( struct lNode *cnode, char *formal, char *target );
extern void lNodePostfixTraversal ( struct lNode *cnode, FILE *stream, void(*callBack)( struct lNode *, FILE *) );
extern void lNodeRemoveBUFFINV ( struct lNode *cnode );
extern int lNodeInitVisVariables ( void );
extern int lNodeFreeVisVariables ( void );
extern int lNodeWriteDotHeader ( char *graphName, char *graphLabel, FILE *stream );
extern int lNodeDrawVertex ( struct lNode *root, FILE *stream );
extern int lNodeDrawEdges ( struct lNode *root, FILE *stream );
extern int lNodeGenerateNetTable ( FILE *stream );
extern int lNode2LUTs ( struct lNode *cnode, FILE *stream );
extern void lNode2Wires ( struct lNode *cnode, FILE *stream );

#else /* __STDC__ */

extern struct lNode *newlNode ( char type );
extern void lnodeMallocChild ( struct lNode *cnode );
extern void lNodeFree ( struct lNode *root );
extern void lNodeFreeHack ( struct lNode *ProxyRoot );
extern void lNodePrint ( struct lNode *cnode );
extern struct lNode *net2DAG ( void *data, void *parent, char type );
extern void lNodeAppendFormal ( struct lNode *cnode, char *formal );
extern struct lNode *net2lNode ( void *data, void *parent, char type );
extern void lnodeMallocChild2 ( struct lNode *cnode );
extern void releaseNode ( struct lNode *cnode, FILE *stream );
extern int module2Forest2 ( struct module *mpntr, int visualize, int verify, int scrub, char *visFilePath, char *scrubFilePath, char *verifyFilePath );
extern int lNodeScrub ( struct lNode *cnode, FILE *stream );
extern int findIndexOfChild ( struct lNode *child, struct lNode *parent );
extern char invNodeFunction ( struct lNode *cnode, char *target );
extern int invNodeInput ( struct lNode *cnode, char *formal, char *target );
extern void lNodePostfixTraversal ( struct lNode *cnode, FILE *stream, void(*callBack)( struct lNode *, FILE *) );
extern void lNodeRemoveBUFFINV ( struct lNode *cnode );
extern int lNodeInitVisVariables ( void );
extern int lNodeFreeVisVariables ( void );
extern int lNodeWriteDotHeader ( char *graphName, char *graphLabel, FILE *stream );
extern int lNodeDrawVertex ( struct lNode *root, FILE *stream );
extern int lNodeDrawEdges ( struct lNode *root, FILE *stream );
extern int lNodeGenerateNetTable ( FILE *stream );
extern int lNode2LUTs ( struct lNode *cnode, FILE *stream );
extern void lNode2Wires ( struct lNode *cnode, FILE *stream );

#endif /* __STDC__ */
#endif /* __CEXTRACT__ */