/*
 * =====================================================================================
 *
 *       Filename:  vConstants.h
 *
 *    Description:  This file defines global constants used in the sASIC front
 *                  end (Verilog Compiler)
 *
 *        Version:  1.0
 *        Created:  07/15/2008 12:24:53 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 * =====================================================================================
 */

#define LIST_INIT_SIZE  10
#define LIST_DELTA      10

#define STR_LEN_MAX     256
#define N_INST_INIT     50
#define N_CON_OBJ_INIT  100
#define N_IPORT_INIT    2
#define INIT_N_MPORT    20

#define DYN_ARRAY_DELTA 10

#define DIR_IN          0
#define DIR_OUT         1
#define DIR_INOUT       2

#define T_STRING        0
#define T_INTEGER       1
#define T_CONOBJ        2
#define T_INSTANCE      3
#define T_MODULE        4
#define T_LNODE         5
#define T_CONOBJ_PNTR   6

#define ST_DELTA        10
#define MAGIC_VSS       1
#define MAGIC_VDD       0

#define BEXPR_LEN       2048


#define NODE_CON_OBJ    2
#define NODE_LUT        3
#define NODE_DFF        1
#define NODE_PORT       4
#define NODE_CONSTANT   5


#define DETAIL_MAX      1
#define DETAIL_MIN      0
#define sASIC_DFF_NAME "sASIC_DFF"


