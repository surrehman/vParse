/*
 * =====================================================================================
 *
 *       Filename:  sASIC_lib.h
 *
 *    Description:  Constants related to structured ASIC library
 *
 *        Version:  1.0
 *        Created:  07/24/2008 04:49:41 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 * =====================================================================================
 */


const char fLUT4[256] = {"(~( A3 ) && ((~( A2 ) && ( f0 )) || (( A2 )  && ( f1)))) || ( ( A3 )  && ((!( A2 ) &&  ( f2 )) || ( ( A2 ) && ( f3 ))))"};
const char fLUT3[256] = {"((~( A2 ) && ( f0 )) || (( A2 )  && ( f1 )))"};
const char fLUT2Verilog[17][64]={
    "1",                                       // 00
    "(( A0 ) || ( A1 ))",                      // 01
    "(~(( A0 ) || ( A1 )))",                   // 02
    "(( A0 ) || (~( A1 )))",                   // 03
    "(~( A0 ) && ( A1 ))",                     // 04
    "( A0 )",                                  // 05
    "(~( A0 ))",                               // 06
    "((~ A0 ) || ( A1 ))",                     // 07
    "(( A0 )&& (~ A1 ))",                      // 08
    "( A1 )",                                  // 09
    "(~( A1 ))",                               // 10
    "(~( A0  ^  A1 ))",                        // 11
    "( A0  ^  A1 )",                           // 12
    "( A0  &&  A1 )",                          // 13
    "(~( A0  &&  A1 ))",                       // 14
    "0",                                       // 15
    "((~( S )&&( A0 )) || (( S )&&( A1 )))"};  // 16

