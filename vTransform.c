/*
 * =====================================================================================
 *
 *       Filename:  vTransform.c
 *
 *    Description:  Routines for the transformation of structural verilog to
 *                  Directed Acyclic Graphs (DAG).
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
#include<time.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/stat.h>
#include"vDatatypes.h"
#include"strUtils.h"
#include"vConstants.h"
#include"sVerilog.h"
#include"vTransform.h"
#include"sASIC_LUT.h"
//#include"sASIC_lib.h"

// ----------------------------------------------------------------------------
//          Global variables for DAG forest in a design
// ----------------------------------------------------------------------------
int               TREE_INDEX;          // index of trees in the DAG forst
int               NBUFF_SCRUBBED;      // number of BUFFs scrubbed   
int               NINV_SCRUBBED;       // number of INVs  scrubbed   
struct dynArray  *U_LEAF_NAMES;        // an array of unique leaf name in a DAG
// ----------------------------------------------------------------------------
// Section: lnode creation, management functions
// ----------------------------------------------------------------------------
struct lNode *newlNode(char type){
    struct lNode *tNode = malloc(sizeof(struct lNode));
    assert(tNode != NULL);
    tNode->type      = type;
    tNode->parent    = NULL;
    tNode->formals   = NULL;
    tNode->actuals   = NULL;
    tNode->nChildren = 0;
    tNode->mData     = NULL;
    tNode->children  = NULL;
    return (tNode);
}
void lnodeMallocChild(struct lNode *cnode){
    // malloc/ realloc lnode->children. Do not do anything with the actual
    // pointer to the node lnode->children[i]. Similarly malloc/ realloc() for
    // lnode->formals, lnode->actuals, but do not malloc lnode->formals[i].
    // Also do not increment cnode->nChildren
    if (cnode->nChildren == 0){
        cnode->children = malloc(sizeof(struct lNode *));
        assert(cnode->children != NULL);

        cnode->formals    = malloc(sizeof(char *)); 
        assert(cnode->formals != NULL);
        cnode->formals[0] = NULL;

        cnode->actuals    = malloc(sizeof(char *)); 
        assert(cnode->actuals != NULL);
        cnode->actuals[0] = NULL;

    }
    else {
        cnode->children = realloc(cnode->children, sizeof(struct lNode *) * 
        (1 + cnode->nChildren));
        assert (cnode->children != NULL);

        cnode->formals = realloc(cnode->formals, 
            sizeof(char *) * (1 + cnode->nChildren));
        assert(cnode->formals != NULL);

        cnode->actuals = realloc(cnode->actuals,
            sizeof(char *) * (1 + cnode->nChildren));
        assert(cnode->actuals != NULL);
    }
}
void lNodeFree(struct lNode *root){
    // free memory used by the DAG with given argument as root
    int i;
    for( i=0; i<root->nChildren; i++)
        lNodeFree( ((struct lNode **)root->children)[i]);
    if (root->nChildren != 0) free(root->children);
    int j;
    for( j=0; j<root->nChildren; j++){
        free( ((char **)root->formals)[j] );
        free( ((char **)root->actuals)[j] );
    }
    free( root->formals);
    free( root->actuals);
    free(root->mData);
    free(root);
}
void lNodeFreeHack(struct lNode *ProxyRoot){
    // Hack for the wart: the "real" root of the DAG is not the LUT, but rather
    // a T_CONOBJ node whcih is above the LUT. Free this LUT individually
    free(ProxyRoot->mData);
    free(ProxyRoot->children);
    free(ProxyRoot);
}
void lNodePrint(struct lNode *cnode){
    // print out the structure of the DAG on stdout
    if (cnode->type == T_INSTANCE){
        printf("Node %s: type= instance, #children: %d\n", cnode->mData,
            cnode->nChildren);
        int i;
        for(i=0;i<cnode->nChildren;i++)
            printf("%d : formal: %s, actual: %s\n", i, cnode->formals[i],
            cnode->actuals[i]);


        for(i=0;i<cnode->nChildren;i++){
            struct lNode *childNode =  ((struct lNode **)cnode->children)[i];
            lNodePrint(childNode);
        }
    }
    else if (cnode->type == T_CONOBJ){
        printf("Node %s: type= connection object, #children: %d\n",
        cnode->mData, cnode->nChildren);

    }
    else{
        printf("-E- Unknown node type!!\n");
        exit(EXIT_FAILURE);
    }
}
struct lNode *net2DAG(void *data, void *parent, char type){
    // Grow a tree by postfix traversal. Data is a pointer to either an instance
    // or a connection object. Return the grown tree. Memory allocation is _NOT_
    // done by caller.

    // For the first call, data is always a pointer to an instance. 

    struct lNode *cnode = newlNode(type);
    int nodeIndex;
    cnode->type         = type;
    cnode->parent       = parent;

    if (type == T_INSTANCE){
        // make sure that the instance is not a DFF !
        struct instance *ipntr = (struct instance *)data;
        assert( strcmp(ipntr->name, "sASIC_DFF") != 0);
        cnode->mData = malloc(sizeof(char)*(4+strlen(ipntr->type)));
        assert (cnode->mData != NULL);
        strcpy(cnode->mData, ipntr->type);

        // for all nets at the input pins of the instance which are NOT driven
        // by DFFs, grow the DAG. If the net is driven by a DFF: terminate the
        // tree.
        int i ; char *formal; struct conObj *actual;
        for( i=0; i < ipntr->formalPortList->index; i++){
            formal = ((char **)ipntr->formalPortList->data)[i];
            if ( strcmp(formal,"YN")==0 || strcmp(formal, "Z") == 0) continue;
            else{
                // here create child nodes if the inputs are not driven by input
                // ports or by DFF ouputs
                actual = ((struct conObj **)ipntr->actualPortList->data)[i];
                nodeIndex = cnode->nChildren;
                struct instance *driver = actual->driver;
                if (actual->isPort == 1 ){
                    // terminal node here
                    lnodeMallocChild(cnode);
                    cnode->children[nodeIndex]=net2DAG(actual, cnode, T_CONOBJ);
                    // set current node's formals and actuals
                    cnode->formals[nodeIndex] = malloc(
                    sizeof(char) * (4 + strlen(formal)));
                    assert(cnode->formals[nodeIndex] != NULL);
                    strcpy(cnode->formals[nodeIndex], formal);

                    cnode->actuals[nodeIndex] = malloc(
                    sizeof(char) * (4 + strlen(actual->name)));
                    assert(cnode->actuals[nodeIndex] != NULL);
                    strcpy(cnode->actuals[nodeIndex], actual->name);

                    cnode->nChildren +=1;

                }
                else if (driver != NULL){
                    if( strcmp(driver->type, "sASIC_DFF") == 0 && 
                        actual->isPort != 1){
                        // terminal node here
                        lnodeMallocChild(cnode);
                        cnode->children[i]=net2DAG(actual, cnode, T_CONOBJ);

                        // set current node's formals and actuals
                        cnode->formals[nodeIndex] = malloc(
                        sizeof(char) * (4 + strlen(formal)));
                        assert(cnode->formals[nodeIndex] != NULL);
                        strcpy(cnode->formals[nodeIndex], formal);

                        cnode->actuals[nodeIndex] = malloc(
                        sizeof(char) * (4 + strlen(actual->name)));
                        assert(cnode->actuals[nodeIndex] != NULL);
                        strcpy(cnode->actuals[nodeIndex], actual->name);

                        cnode->nChildren +=1;
                    }
                    else{
                        // non-terminal node here
                        lnodeMallocChild(cnode);
                        cnode->children[nodeIndex] = net2DAG(driver, cnode, T_INSTANCE);

                        // set current node's formals and actuals
                        cnode->formals[nodeIndex] = malloc(
                        sizeof(char) * (4 + strlen(formal)));
                        assert(cnode->formals[nodeIndex] != NULL);
                        strcpy(cnode->formals[nodeIndex], formal);

                        cnode->actuals[nodeIndex] = malloc(
                        sizeof(char) * (4 + strlen(actual->name)));
                        assert(cnode->actuals[nodeIndex] != NULL);
                        strcpy(cnode->actuals[nodeIndex], actual->name);

                        cnode->nChildren +=1;
                    }
                }
            }
        }
    }
    if (type == T_CONOBJ){
        // No need to grow down. Terminate here
        struct conObj *cpntr = (struct conObj *)data;
        char tmpStr[64];
        conObjStringRep(cpntr, tmpStr);

        cnode->mData         = malloc(sizeof(char)* (4+strlen(tmpStr)));
        assert(cnode->mData  != NULL);
        strcpy(cnode->mData, tmpStr);

    }
    return cnode;

}
void lNodeAppendFormal(struct lNode *cnode, char *formal){
    // malloc mem. a node's formal member and copy given formal name
    cnode->formals[cnode->nChildren] = malloc(sizeof(char)*(2+strlen(formal)));
    assert(cnode->formals[cnode->nChildren] != NULL);
    strcpy(cnode->formals[cnode->nChildren], formal);

}
struct lNode *net2lNode(void *data, void *parent, char type){
    // Given a root object (net/ instance) grow a DAG rooted by the object. The
    // memory allocation is NOT done by the caller of this function. The root
    // and the leaves of the DAG are always non-reduceable nets: inputs,
    // outputs, DFF outputs or constant values. 
    struct lNode    *cnode  = newlNode(type);
    struct instance *tInstance;
    struct conObj *tConObj;
    cnode->parent = parent;
    if (type == T_INSTANCE){
        // For each net at input, grow tree recursively.
        tInstance = (struct instance *)data;
        assert(strcmp(tInstance->type, sASIC_DFF_NAME) != 0);
        cnode->mData = malloc(sizeof(char)*(2+strlen(tInstance->type)));
        assert(cnode->mData != NULL);
        strcpy(cnode->mData, tInstance->type);
        tInstance = (struct instance *)data;

        int i;
        for (i=0; i<tInstance->formalPortList->index; i++){
            char *formal = ((char **)tInstance->formalPortList->data)[i];
            if (strcmp(formal, "YN") == 0 || strcmp(formal, "Z") == 0) continue;
            // for each of the inputs, recursively call net2lNode
            lnodeMallocChild2(cnode);
            // Append formals:
            lNodeAppendFormal(cnode, formal);

            // examine drivers for the instance's inputs
            tConObj  = ((struct conObj **)tInstance->actualPortList->data)[i];
            if (tConObj->isPort == 1  || tConObj->isConst == 1){
                cnode->children[i] = net2lNode(tConObj, cnode, T_CONOBJ);
            }
            else if (tConObj->driver != NULL){
                if ( strcmp(tConObj->driver->type, sASIC_DFF_NAME) == 0){
                    cnode->children[i] = net2lNode(tConObj, cnode, T_CONOBJ);
                }
                else{
                    cnode->children[i] = net2lNode(tConObj->driver, cnode, 
                    T_INSTANCE);
                }
            }
            else{
                fprintf(stdout, "-E- Irrecoverable error in net2lNode\n");
                exit(1);
            }
            cnode->nChildren +=1;
        }
    }
    if (type == T_CONOBJ){
        // if it is a root grow, else terminate
        tConObj = (struct conObj *)data;
        cnode->mData = malloc(sizeof(char)*(8+strlen(tConObj->name)));
        assert (cnode->mData != NULL);
        conObjStringRep(tConObj, cnode->mData);
        if (parent == NULL){
            // root node-- find out driver and grow.
            tInstance = tConObj->driver;
            assert( tInstance != NULL);
            assert( strcmp(tInstance->type, sASIC_DFF_NAME)  != 0);
            lnodeMallocChild2(cnode);
            cnode->children[0] = net2lNode(tInstance, cnode, T_INSTANCE);
            cnode->nChildren +=1;
        }
        else{
            ;
            // non-root-- terminate tree
        }
    }
    return cnode;
}
void lnodeMallocChild2(struct lNode *cnode){
    // malloc/ realloc lnode->children. Do not do anything with the actual
    // pointer to the node lnode->children[i]. Similarly malloc/ realloc() for
    // lnode->formals, lnode->actuals, but do not malloc lnode->formals[i].
    // Also do not increment cnode->nChildren
    if (cnode->nChildren == 0){
        cnode->children = malloc(sizeof(struct lNode *));
        assert(cnode->children != NULL);

        cnode->formals    = malloc(sizeof(char *)); 
        assert(cnode->formals != NULL);
        cnode->formals[0] = NULL;


    }
    else {
        cnode->children = realloc(cnode->children, sizeof(struct lNode *) * 
        (1 + cnode->nChildren));
        assert (cnode->children != NULL);

        cnode->formals = realloc(cnode->formals, 
            sizeof(char *) * (1 + cnode->nChildren));
        assert(cnode->formals != NULL);

    }
}
void releaseNode(struct lNode *cnode, FILE *stream){
    // Free node memory
    if (cnode->mData != NULL) free(cnode->mData);
    int i;
    if (cnode->nChildren != 0){
        for(i = 0; i < cnode->nChildren; i++)
            free(cnode->formals[i]);
        free(cnode->formals);
        free(cnode->children);
    }
    free(cnode);
}
int module2Forest2(struct module *mpntr, int visualize, int verify, int scrub,
    char *visFilePath, char *scrubFilePath, char *verifyFilePath){
    // Convert CLA in modules to DAGs. Perform certain other tasks such as
    // generation of visualization files,conversion to behavioural or 
    // structural verilog statements, etc.

    int              i, j;
    struct conObj   *cpntr;
    struct conObj   *actual;
    struct instance *ipntr;
    char            *formal;
    struct lNode    *tmpNode;

    struct dynArray *extrctNets=newDynArray(T_CONOBJ_PNTR, 1);
    struct dynArray *uConObjPntrs; // for both module ports and wires
    struct dynArray *uPortNames;
    struct dynArray *uConObjNames;

    FILE *visFilePntr;
    FILE *scrubFilePntr;
    FILE *verifyFilePntr;

    if (visualize){
        visFilePntr = fopen(visFilePath, "w");
        if (visFilePntr == NULL){
            printf("-E- Cannot open %s for writing\n", visFilePath);
            return 1;
        }
    }
    if (verify || scrub){
    uConObjPntrs = newDynArray(T_CONOBJ_PNTR, 1);
    uPortNames   = newDynArray(T_STRING, 1);
    uConObjNames = newDynArray(T_STRING, 1);
    TREE_INDEX   = 0;
        if (verify){
            verifyFilePntr= fopen(verifyFilePath, "w");
            if (verifyFilePntr== NULL){
                printf("-E- Cannot open %s for writing\n", verifyFilePath);
                return 1;
            }
        }
        if (scrub){
            scrubFilePntr= fopen(scrubFilePath, "w");
            if (scrubFilePntr== NULL){
                printf("-E- Cannot open %s for writing\n", scrubFilePath);
                return 1;
            }
        }
    }
    //-------------------------------------------------------------------------
    //
    //  Inialize variables: Which nets should be extracted: 
    //      1. Output ports whcih are not driven by DFFs.       
    //      2. DFF inputs whcih are not driven by DFFs.         
    //                                                     
    //  (1) Module output ports not driven by DFFs
    //-------------------------------------------------------------------------
    for (i=0; i< mpntr->conObjList->index; i++){
        // To generate verilog, find out unique port names
        cpntr = ((struct conObj **)mpntr->conObjList->data)[i];
        if (verify || scrub){
            if (cpntr->isPort == 1){
                if (dynArrayFind(uPortNames, cpntr->name) == -1){
                    dynArrayAppend(uConObjPntrs, cpntr);
                    dynArrayAppend(uPortNames, cpntr->name);

                }
            }
        }
        // Find out nets to extact:
        if (cpntr->isPort == 1 && cpntr->direction == DIR_OUT){
            if (cpntr->driver == NULL){
                fprintf(stdout, "-W- Port %s[%d] has no driver\n", cpntr->name,
                cpntr->index);
            }
            else{
                if (strcmp(cpntr->driver->type, "sASIC_DFF") == 0) continue;
                else{
                    if ( dynArrayFind(extrctNets, cpntr) == -1 ){
                        dynArrayAppend(extrctNets, cpntr);
                    }
                }
           }
        }
    }
    //-------------------------------------------------------------------------
    //  (2) DFF inputs driven by LUTs
    //-------------------------------------------------------------------------
    for (i = 0; i<mpntr->instList->index; i++){
        ipntr = ((struct instance **)mpntr->instList->data)[i];
        if (strcmp(ipntr->type, "sASIC_DFF") != 0) continue;
        for (j=0; j<ipntr->formalPortList->index; j++){
            formal = ((char **)ipntr->formalPortList->data)[j];
            actual = ((struct conObj **)ipntr->actualPortList->data)[j];
            // to generate veriog, find out actuls to DFFs
            if (verify || scrub){
                if (actual->isPort != 1 && actual->isConst != 1){
                    if (dynArrayFind(uConObjNames, actual->name) == -1 
                    && dynArrayFind(uConObjPntrs, actual) == -1){
                        dynArrayAppend(uConObjPntrs, actual);
                        dynArrayAppend(uConObjNames, actual->name);
                    }
                }
            }
            if (strcmp(formal, "Q") == 0) continue;
            if (actual->driver == NULL ) continue;
            if (strcmp(actual->driver->type, "sASIC_DFF") == 0) continue;
            if ( dynArrayFind(extrctNets, actual) == -1){
                dynArrayAppend(extrctNets, actual);
                
            }
        }
    }
    // -------------------------------------------------------------------------
    // Nets forming CLA roots known, generate visualizations/ verilog, etc. etc.
    // -------------------------------------------------------------------------
    if (visualize){
        for (i = 0;i < extrctNets->index; i++){
            cpntr = ((struct conObj **)extrctNets->data)[i];

            tmpNode = net2lNode(cpntr, NULL, T_CONOBJ);
            
            lNodeInitVisVariables(); 
           
             char graphName[64];
             char graphLabel[64];
             conObjStringRep(actual, graphName);
             sprintf(graphLabel, "Design: %s, CLA Root: %s", 
             mpntr->name, tmpNode->mData);
             lNodeWriteDotHeader(graphName, graphLabel, visFilePntr);
            

            lNodePostfixTraversal(tmpNode, visFilePntr, 
                (void *) &lNodeDrawEdges);

            lNodePostfixTraversal(tmpNode, 
                visFilePntr, (void *) & lNodeDrawVertex);

            lNodeGenerateNetTable(visFilePntr);
            fprintf(visFilePntr, "}\n");
            lNodeFreeVisVariables();

            lNodePostfixTraversal(tmpNode, stdout, (void *)&releaseNode);
        }
        fclose(visFilePntr);
        char cmdString[256];
        printf("-I- Number of CLAs: %d\n", extrctNets->index);
        fprintf(stdout,
            "-I- Generating postscript for CLAs in design %s as %s.ps\n", 
            mpntr->name, visFilePath);
        sprintf(cmdString, "dot -q1 -Tps %s -o %s.ps", visFilePath,
        visFilePath);
        if (system(cmdString) != 0){
            fprintf(stdout,
                "-W- Could not convert internal data to postscript\n");
            fprintf(stdout, "-W- Error in file %s\n", visFilePath);
        }
        else{
            if (remove(visFilePath) != 0)
                fprintf(stdout, "-W- Could not remove dot file %s\n",
                    visFilePath);
        }
    }
    if (verify || scrub){
        // Operations common to verify and scrub
        // Write module interface, DFF instantiations in design
        int            i;
        struct conObj *cpntr;
        struct instance *ipntr;
        if (verify){
            PntrList2Module(uConObjPntrs, mpntr->name, verifyFilePntr);
            fprintf(verifyFilePntr, "/* DFFs in the design */\n");
            for (i=0;i<mpntr->instList->index; i++){
                ipntr = ((struct instance **)mpntr->instList->data)[i];
                if (strcmp(ipntr->type, "sASIC_DFF") == 0){
                    declareInstance(ipntr, verifyFilePntr);
                }
            }
            for (i=0; i< extrctNets->index; i++){
                cpntr = ((struct conObj **)extrctNets->data)[i];
                tmpNode = net2lNode(cpntr, NULL, T_CONOBJ); 
                fprintf(verifyFilePntr, "// Generating DAG #%d\n", i);
                fprintf( verifyFilePntr,
                    "/* Structured Verilog for CLA */\n");
                lNodePostfixTraversal(tmpNode, verifyFilePntr,
                    (void *) & lNode2Wires);
                lNodePostfixTraversal(tmpNode, verifyFilePntr,
                    (void *) &  lNode2LUTs);
                lNodePostfixTraversal(tmpNode, stdout, (void *)&releaseNode);
                TREE_INDEX ++;
            }
            // closing actions
            fprintf(verifyFilePntr, "endmodule\n");
            fclose(verifyFilePntr);
            fprintf(stdout, "-I- Module description written to %s\n", 
            verifyFilePath);
        }
        if (scrub){ // Scrub DAGs, generate equivalent structural verilog
            NBUFF_SCRUBBED= 0;
            NINV_SCRUBBED= 0;
            PntrList2Module(uConObjPntrs, mpntr->name, scrubFilePntr);
            fprintf(scrubFilePntr, "/* DFFs in the design */\n");
            for (i=0;i<mpntr->instList->index; i++){
                ipntr = ((struct instance **)mpntr->instList->data)[i];
                if (strcmp(ipntr->type, "sASIC_DFF") == 0){
                    declareInstance(ipntr, scrubFilePntr);
                }
            }
            for (i=0; i< extrctNets->index; i++){
                cpntr = ((struct conObj **)extrctNets->data)[i];
                tmpNode = net2lNode(cpntr, NULL, T_CONOBJ); 

                // Scrub the DAG, carefully
                lNodePostfixTraversal(tmpNode, scrubFilePntr,
                    (void *) & lNodeScrub );
                fprintf( scrubFilePntr,
                    "/* Structured Verilog for CLA */\n");
                lNodePostfixTraversal(tmpNode, scrubFilePntr,
                    (void *) & lNode2Wires);
                lNodePostfixTraversal(tmpNode, scrubFilePntr,
                    (void *) &  lNode2LUTs);
                lNodePostfixTraversal(tmpNode, stdout, (void *)&releaseNode);
                TREE_INDEX ++;
            }
            // closing actions
            fprintf(scrubFilePntr, "endmodule\n");
            fprintf(stdout, "-I- Number of BUFFs scrubbed: %d\n", NBUFF_SCRUBBED);
            fprintf(stdout, "-I- Number of INVs scrubbed: %d\n", NINV_SCRUBBED);
            fprintf(stdout, "-I- Module description written to %s\n", 
            scrubFilePath);
            fclose(scrubFilePntr);
        }
        dynArrayFree(uConObjPntrs,1);
        dynArrayFree(uPortNames, 1);
        dynArrayFree(uConObjNames, 1);
    }
    // ------------------------------------------------------------------------
    // Final operations common to different tasks
    // ------------------------------------------------------------------------
    dynArrayFree(extrctNets, 1);
    return 0;

}
int lNodeScrub (struct lNode *cnode, FILE *stream){
    // Callback function for scrubbing a DAG
    /*
    // Condition for feedthrough: 
    // -------------------------- 
    // If by removing a BUFF, the tree is reduced to 2 nodes: BUFF's parent and
    // BUFF's child[0], then we've got a feedthrough. The sensible solution to a
    // feed through is to replace BUFF' parent by BUFF's child[0] for all
    // occurances of BUFF's parent in the module. However this has not been 
    // implimented _yet_.
    //
    //
    // BUFF can be removed without any change of logic in the BUFF tree. In
    // order to remove a BUFF N in the DAG, the following operations are
    // required


    // i.  N->parent->children[i] = N->children[0]
    // ii. N->children[0]->parent = N->parent

    // Where i satisfies the expression:
    //     N->parent->children[i] = N                                   */
    int buffIndex ;
    char tmpStr[16];
    char *formal;
    struct lNode *childNode;
    struct lNode *parentNode; 
    if (isLUTBUFF(cnode->mData) || isLUTINV(cnode->mData)){
        parentNode = (struct lNode *)cnode->parent;
        childNode  = ((struct lNode **)cnode->children)[0];
        buffIndex  =  findIndexOfChild(cnode, cnode->parent);
    }
    else return 1;
    assert(cnode->parent != NULL);                     // EXTENDED PARANOIA
    assert(buffIndex != -1);                           // EXTENDED PARANOIA
    if (isLUTBUFF(cnode->mData)){
        // This is BUFF: next check feedthrough 
        if (parentNode->type == T_CONOBJ && childNode->type == T_CONOBJ){
            fprintf(stdout, "-W- Feedthrough for %s b/w %s and %s...\n",
            cnode->mData, parentNode->mData, childNode->mData);
            fprintf(stdout,"    cowardly refusing to remove\n");
            fprintf(stdout, "-W- Run clean_feedthru on this design before scubbing!\n");
            return 0;
        }
        // feedthrough check ok: proceed to remove BUFF
        ((struct lNode **)parentNode->children)[buffIndex] = childNode;
        childNode->parent                                  = parentNode;
        NBUFF_SCRUBBED ++;
        releaseNode(cnode, NULL);
        return 0;
    }
    else if (isLUTINV(cnode->mData)){
        // first try to absorb INV into childNode, and then parentNode
        if (childNode->type == T_INSTANCE){
            // changing function for childNode
            if  (invNodeFunction(childNode, tmpStr) == 0){
                strcpy(childNode->mData, tmpStr);
                ((struct lNode **)parentNode->children)[buffIndex] = childNode;
                childNode->parent                                  = parentNode;
                NINV_SCRUBBED++;
                releaseNode(cnode, NULL);
                return 0;
            }
        }
        else if (parentNode->type == T_INSTANCE){
            // changing function for parentNode
            formal = parentNode->formals[buffIndex];
            if (invNodeInput(parentNode, formal, tmpStr) == 0){
                //fprintf(stdout, "-D- Old: %s, new:%s\n", cnode->mData, tmpStr);
                strcpy(parentNode->mData, tmpStr);
                ((struct lNode **)parentNode->children)[buffIndex] = childNode;
                childNode->parent                                  = parentNode;
                NINV_SCRUBBED ++;
                releaseNode(cnode, NULL);
                return 0;
            }
        }
        else{
            fprintf(stdout, "-W- Cannot remove INV b/w nets %s %s\n", 
            childNode->mData, parentNode->mData);
        }
    }
    return 0;
}
int findIndexOfChild(struct lNode *child, struct lNode *parent){
    // Given child, parent, what is the index of child in parent->children.
    // Return -1 if the child is not in parent->children!
    int i;
    for( i=0; i< parent->nChildren; i++){
        if (parent->children[i] == child) return i;
    }
    return -1;
}
// ----------------------------------------------------------------------------
// Section: lnode logic manipulation functions
// ----------------------------------------------------------------------------
char invNodeFunction(struct lNode *cnode, char *target){
    // Write type of inverted logic function of the given lNode. Return 0 if
    // result was written successfully or 1 if not
    assert(strstr(cnode->mData, "LUT2_MUX_")!=NULL);
    int LUTindex=atoi(cnode->mData + strlen("LUT2_MUX_"));
    int rIndex= -1;
    assert(LUTindex<= 16) ;
    switch(LUTindex){
        case(0):  return 1;
        case(1):  rIndex=2; break;
        case(2):  rIndex=1; break;
        case(3):  rIndex=4; break;
        case(4):  rIndex=3; break;
        case(5):  rIndex=6; break;
        case(6):  rIndex=5; break;
        case(7):  rIndex=8; break;
        case(8):  rIndex=7; break;
        case(9):  rIndex=10; break;
        case(10): rIndex=9; break;
        case(11): rIndex=12; break;
        case(12): rIndex=11; break;
        case(13): rIndex=14; break;
        case(14): rIndex=13; break;
        case(16): return 1;
    }
    assert(rIndex!= -1);
    sprintf(target, "LUT2_MUX_%02d", rIndex);
    return 0;
}
int invNodeInput(struct lNode *cnode, char *formal, char *target){
    // Given a LUT node and one of its formals write LUT type into target which
    // is requivalent to the orignal node, if the input at the given formal is
    // inverted
    assert(strstr(cnode->mData, "LUT2_MUX_")!=NULL);

    int LUTindex=atoi(cnode->mData+ strlen("LUT2_MUX_"));
    int rIndex= -1;
    int findex=-1;
    assert(LUTindex<= 16) ;
    if (strcmp(formal, "A0")==0) findex=0;
    if (strcmp(formal, "A1")==0) findex=1;
    assert(findex!= -1);

    switch(LUTindex){
        case(0):
            return 1;
        case(1):
            if (findex==0) rIndex=7; 
            if (findex==1) rIndex=3; 
            break;
        case(2):
            if (findex==0) rIndex=8; 
            if (findex==1) rIndex=4;  
            break;
        case(3):
            if (findex==0) rIndex=14; 
            if (findex==1) rIndex=1;  
            break;
        case(4):
            if (findex==0) rIndex=13;
            if (findex==1) rIndex=2;
            break;
        case(5):
            if (findex==0) rIndex=6;
            if (findex==1) return 1;
            break;
        case(6):
            if (findex==0) rIndex=5;
            if (findex==1) return 1;
            break;
        case(7):
            if (findex==0) rIndex=1;
            if (findex==1) rIndex=14;
            break;
        case(8):
            if (findex==0) rIndex=2;
            if (findex==1) rIndex=13;
            break;
        case(9):
            if (findex==0) return 1;
            if (findex==1) rIndex=10;
            break;
        case(10):
            if (findex==0) return 1;
            if (findex==1) rIndex=9;
            break;
        case(11):
            if (findex==0) rIndex=12;
            if (findex==1) rIndex=12;
            break;
        case(12):
            if (findex==0) rIndex=11;
            if (findex==1) rIndex=11;
            break;
        case(13):
            if (findex==0) rIndex=4;
            if (findex==1) rIndex=8;
            break;
        case(14):
            if (findex==0) rIndex=3;
            if (findex==1) rIndex=7;
            break;
        case(15): return 1;
        case(16): return 1;
    }
    assert(rIndex!= -1);
    sprintf(target, "LUT2_MUX_%02d", rIndex);
    return 0;
}
void lNodePostfixTraversal(struct lNode *cnode, FILE *stream ,
    void(*callBack)( struct lNode *, FILE *)){
    //Do a postfix traversal on the given node, with callback function operation
    //applied on each node.
    int i;
    for(i=0;i<cnode->nChildren;i++)
        lNodePostfixTraversal(cnode->children[i], stream, *callBack);
    (*callBack)(cnode, stream);
}   
void lNodeRemoveBUFFINV(struct lNode *cnode){
    // Call function for removing all BUFF/ INV from a design. For BUFF nodes,
    // removal does not require any calculation: simply set the node's parent
    // and child member
    int i; 
    for(i = 0; i < cnode->nChildren; i++){
        lNodeRemoveBUFFINV(cnode->children[i]);
    }
    if (cnode->type == T_INSTANCE){
        if (isLUTBUFF(cnode->mData)==1){
            /* This node can be removed without any consideration: Perform the
               following transformations:
               cnode->parent->children[i] = cnode->children[0]
               cnode->children[0]->parent = cnode->parent

               where i is defined by
               cnode->parent->children[i] == cnode
            */
            int nodeIndex ;
            nodeIndex = findIndexOfChild(cnode, cnode->parent);
            assert(nodeIndex != -1);
        
            struct lNode *buffParent = (struct lNode *)cnode->parent;
            struct lNode *buffChild  = ((struct lNode **)cnode->children)[0];             

            assert(buffParent->children[nodeIndex] == cnode);
            assert(buffChild->parent == cnode);

            buffParent->children[nodeIndex] = buffChild;
            buffChild->parent = buffParent;
            
            printf("=D= Manipulation done, freeing cnode\n");
            free( cnode->formals[0] );
            free( cnode->actuals[0] );
            free( cnode->formals);
            free( cnode->actuals);
            free( cnode->mData);
            free(cnode);


        }
    }
}
// ----------------------------------------------------------------------------
// Visualization functions
// ----------------------------------------------------------------------------
int lNodeInitVisVariables(void){
    // Initialize per-tree visualization variables
    U_LEAF_NAMES        = newDynArray(T_STRING, 1);
    return 0;
}
int lNodeFreeVisVariables(void){
    // Free per-tree visualization variables
    dynArrayFree(U_LEAF_NAMES, 1);
    return 0;
}
int lNodeWriteDotHeader(char *graphName, char *graphLabel, FILE *stream){
    fprintf(stream, "digraph \"%s\"{ranksep=equally; center=true;" ,graphName);
    fprintf(stream, "size=\"11,16\"; label=\"%s\"\n", graphLabel);
    return 0;
}
int lNodeDrawVertex(struct lNode *root, FILE *stream){
    // Write dot code for the DAG vertex (node) to the given file pointer
    fprintf(stream, "\"%p\"", root);
    if (root->type == T_CONOBJ){
        fprintf(stream, "[height=.2, shape=circle, fontcolor=black,");
        fprintf(stream, "fixedsize=true,group=\"DAG_root\", style=filled,");
        // Unique netnames have same label
        int leafIndex = dynArrayFind(U_LEAF_NAMES, root->mData);
        if (leafIndex == -1){
            dynArrayAppend(U_LEAF_NAMES, root->mData);
            leafIndex = U_LEAF_NAMES->index -1;
        }
        fprintf(stream, "label = \"%d\",", leafIndex);
        fprintf(stream, "fontsize=6,fixedsize=true];\n");
    }
    else if (root->type == T_INSTANCE){
        fprintf(stream, "[height=.2,shape=circle,style=filled,");
        fprintf(stream, "fontsize=6,fixedsize=true,");
        if (strcmp(root->mData, "LUT2_MUX_06") ==0 
            || strcmp(root->mData, "LUT2_MUX_10") ==0)
            fprintf(stream, "fillcolor=red, fontcolor=white,");
        else if (strcmp(root->mData, "LUT2_MUX_05") ==0 
            || strcmp(root->mData, "LUT2_MUX_09") ==0)
            fprintf(stream, "fillcolor=green,fontcolor=black,");
        else
            fprintf(stream, "fillcolor=blue, fontcolor=white,");
        fprintf(stream, "label = \"%s\"];\n", root->mData + 9);
    }
    else {
        fprintf(stdout, "-E- Cannot determine node type for edge %s\n",
        root->mData);
        return 1;
    }
    return 0;
}
int lNodeDrawEdges(struct lNode *root, FILE *stream){
    // Create dot code for edges b/w a node and it's children
    int i;
    for( i=0; i<root->nChildren; i++)
        fprintf(stream, "    \"%p\" -> \"%p\" [arrowsize=0.45]\n", 
            root->children[i], root);
    return 0;
}
int lNodeGenerateNetTable(FILE *stream){
    // Generate a 'key-table' of the leaf nodes for DAG. Each DAG has a unique
    // key-table. This is done by creating a node in the graph, which contains
    // the corresponding information in its label
    int   i;
    char *oldLabel;
    char *newLabel = malloc(sizeof(char) * STR_LEN_MAX);
    fprintf(stream, "LEAF_NODE_KEY[shape=box,group=\"LEAF_NODE_KEY\",");
    fprintf(stream, "fillcolor=grey,fontcolor=black,color=white, ");
    fprintf(stream, "fontname=Courier,style=filled,fontsize=8,label=\"");
    assert(newLabel != NULL);
    for(i=0;i<U_LEAF_NAMES->index;i++){
        oldLabel = ((char **)U_LEAF_NAMES->data)[i];
        strcpy(newLabel, oldLabel);
        rinplace(newLabel, "[","\\[");
        rinplace(newLabel, "]","\\]");
        fprintf(stream, "%d = %s\\l", i,
        newLabel);
    }
    fprintf(stream, "\"]");
    free(newLabel);
    return 0;
}
// -----------------------------------------------------------------------------
//                      Net naming convention in DAGs
// -----------------------------------------------------------------------------
/*
                                        *
                                        |
                                        N.parent
                                       / \
                                      N   *
                                     /   / \
                          N.children[0] *   *
  
  
    A net in the DAG, is represented by
    (a) An edge b/w 2 nodes, given neither of the two nodes is a leaf nor the
        DAG root .
    (b) A node which is either a leaf node or a root node.
        
    The equivalent name of a net of type (a) in structural verilog is a function
    of the parent's memory address and the global variable TREE_INDEX. For type
    (b) nodes, corresponding net names follow the convention: "net_%i_%p",
    where 
        %d = index of the formal port 
        %p = node's address in memory
  
   The net name of a type (b) net is obtained from the node's mData member.
*/
// ----------------------------------------------------------------------------
// structual verilog generation functions
// ----------------------------------------------------------------------------
int lNode2LUTs(struct lNode *cnode, FILE *stream){
    // Callback function to write structural verilog code for a node
    // representing a LUT. Note the depndency of this function on global
    // variable TREE_INDEX
    int i;
    if (cnode->type == T_INSTANCE) {
        fprintf(stream, "    %s U_%p_%d (", cnode->mData, cnode, TREE_INDEX);
        char actualStr[64]; struct lNode *childNode;
        for(i=0; i<cnode->nChildren;i++){
            // port mapping for inputs (cnode's children): Check if child is of 
            // type T_CONOBJ or T_INSTANCE. If it is T_CONOBJ, use it's mData
            // member as the net name, else create a netname from rule.
            childNode = ((struct lNode **)cnode->children)[i];
            if (childNode->type == T_CONOBJ)
                strcpy(actualStr, childNode->mData);
            else if (childNode->type == T_INSTANCE)
                sprintf(actualStr, "net_%d_%p_%d", i, cnode, TREE_INDEX);
            else {  // Extended paranoia
                printLine(1, stdout);
                printf("-E- FATAL ERROR: Cannot determine node type!\n");
                printLine(1, stdout); exit(EXIT_FAILURE);
            }
            // write port mapping statement:
            fprintf(stream, " .A%d( %s),", i, actualStr);
        }
        // Time for output port (cnode's parent). Parent could be the root or an
        // intermideate node. In the former case, user parent's mData member as
        // port's actual. In the later case, first find out what is the index if
        // cnode in it's parent's children. Using that index, create the netname
        struct lNode *pNode;
        pNode = (struct lNode *)cnode->parent;
        if (pNode->type == T_INSTANCE){
            int nIndex = findIndexOfChild(cnode, pNode);
            assert(nIndex != -1);
            sprintf(actualStr, "net_%d_%p_%d", nIndex, cnode->parent, TREE_INDEX);
        }
        else if (pNode->type == T_CONOBJ){
            strcpy(actualStr, pNode->mData);    
        }
        else {  // Extended paranoia
            printLine(1, stdout);
            printf("-E- FATAL ERROR: Cannot determine node type!\n");
            printLine(1, stdout); exit(EXIT_FAILURE);
        }
        char outputFormal[8];  strcpy(outputFormal, "UNINIT");
        writeLUTOutputFormal(cnode->mData, outputFormal);
        fprintf(stream, ".%s(%s));\n", outputFormal, actualStr);
    }
    return 0;
}
void lNode2Wires(struct lNode *cnode, FILE *stream){
    // Callback function for generating structural verilog (wires) for the given
    // node and it's children. Note its dependency on global variable TREE_INDEX
    if (cnode->type == T_INSTANCE){
        int i; struct lNode *childNode;
        for (i=0; i<cnode->nChildren; i++){
            childNode = ((struct lNode **)cnode->children)[i];
            if (childNode->type == T_INSTANCE){
                fprintf(stream, "    wire net_%d_%p_%d;\n", i,cnode, TREE_INDEX);
            }
        }
    }
}

