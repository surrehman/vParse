/********************************************************************************
* Auto generated code: manual editing not recommended
* Time stamp: Thu Mar 15 10:12:46 2018
*
* Behavioural description of sASIC library (LUT2+DFF)
********************************************************************************/
primitive U_FD_P_RB_NO (Q, D, CP, RB, NOTIFIER_REG); 

    output Q;  
    input  NOTIFIER_REG,
           D, CP, RB;
    reg    Q; 
    
// FUNCTION : POSITIVE EDGE TRIGGERED D FLIP-FLOP WITH ACTIVE LOW
//            ASYNCHRONOUS CLEAR ( Q OUTPUT UDP ).


    table
 
    //  D   CP      RB     NOTIFIER_REG  :   Qt  :   Qt+1

        1   (01)    1         ?          :   ?   :   1;  // clocked data
        0   (01)    1         ?          :   ?   :   0;

        0   (01)    x         ?          :   ?   :   0;  // pessimism
        1   (01)    x         ?          :   ?   :   x;
        0    ?      x         ?          :   0   :   0;  // pessimism
        ?    ?      x         ?          :   1   :   x;

        1   (x1)    1         ?          :   1   :   1;  // reducing pessimism
        0   (x1)    1         ?          :   0   :   0;                          
        1   (0x)    1         ?          :   1   :   1;  
        0   (0x)    1         ?          :   0   :   0;  

        ?   ?       0         ?          :   ?   :   0;  // asynchronous clear

        ?   (?0)    ?         ?          :   ?   :   -;  // ignore falling clock
        ?   (1x)    ?         ?          :   ?   :   -;  // ignore falling clock
        *    ?      ?         ?          :   ?   :   -;  // ignore the edges on data
        ?    ?      *         ?          :   ?   :   -;  // ignore the edges on clear 

        ?    ?      ?         *          :   ?   :   x;
 
    endtable
endprimitive






// ----------------------------------------------------------------------------
// Description  : sASIC DFF with rising edge clock, low clear, Q output
// Library 	    : 
// Date         : 12-FEB-2002
// Version      : 
// ----------------------------------------------------------------------------
`resetall
`ifdef verifault
    `suppress_faults
    `enable_portfaults
`endif

`timescale 1ns / 1ps

`delay_mode_path

`celldefine




module sASIC_DFF(D, CP, RN, Q);

output  Q;
input   RN,CP,D;

reg notifier;

`ifdef DELAYED_SIGNALS
  wire delD,delCP,delRN;
  U_FD_P_RB_NO #0.01 f1n (Q,delD,delCP,delRN,notifier);
`else
  U_FD_P_RB_NO #0.01 f1n (Q,D,CP,RN,notifier);
`endif

specify
  specparam def_tchk = 0;
  specparam def_del  = 0.01;
 
// Violation constraints
`ifdef DELAYED_SIGNALS
  $setuphold (posedge CP &&& RN,negedge D,def_tchk,def_tchk,notifier,,,delCP,delD);
  $setuphold (posedge CP &&& RN,posedge D,def_tchk,def_tchk,notifier,,,delCP,delD);
  $setuphold (posedge CP &&& D, posedge RN,def_tchk,def_tchk,notifier,,,delCP,delRN);
`else
  $setuphold (posedge CP &&& RN,negedge D,def_tchk,def_tchk,notifier);
  $setuphold (posedge CP &&& RN,posedge D,def_tchk,def_tchk,notifier);
  $setuphold (posedge CP &&& D, posedge RN,def_tchk,def_tchk,notifier);
`endif
 $width (posedge CP &&& RN,def_tchk,def_tchk,notifier);
 $width (negedge CP &&& RN,def_tchk,def_tchk,notifier);
 $width (negedge RN,def_tchk,def_tchk,notifier);
// Delays
// path RN => Q
if (CP == 1'b0) (RN => Q) = def_del;
if (CP == 1'b1) (RN => Q) = def_del;
  (posedge CP => (Q : D )) = def_del;
  (RN => Q) = def_del;
endspecify

endmodule


`endcelldefine

`ifdef verifault
    `disable_portfaults
    `nosuppress_faults
`endif
// ----------------------------------------------------------------------------
module LUT2_MUX_00(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=1;
endmodule 
module LUT2_MUX_01(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=(( A0 ) || ( A1 ));
endmodule 
module LUT2_MUX_02(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=(~(( A0 ) || ( A1 )));
endmodule 
module LUT2_MUX_03(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=(( A0 ) || (~( A1 )));
endmodule 
module LUT2_MUX_04(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=(~( A0 ) && ( A1 ));
endmodule 
module LUT2_MUX_05(A0,YN);
    input A0;
    output YN;
    assign YN=( A0 );
endmodule 
module LUT2_MUX_06(A0,YN);
    input A0;
    output YN;
    assign YN=(~( A0 ));
endmodule 
module LUT2_MUX_07(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=((~ A0 ) || ( A1 ));
endmodule 
module LUT2_MUX_08(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=(( A0 )&& (~ A1 ));
endmodule 
module LUT2_MUX_09(A1,YN);
    input A1;
    output YN;
    assign YN=( A1 );
endmodule 
module LUT2_MUX_10(A1,YN);
    input A1;
    output YN;
    assign YN=(~( A1 ));
endmodule 
module LUT2_MUX_11(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=(~( A0  ^  A1 ));
endmodule 
module LUT2_MUX_12(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=( A0  ^  A1 );
endmodule 
module LUT2_MUX_13(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=( A0  &&  A1 );
endmodule 
module LUT2_MUX_14(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=(~( A0  &&  A1 ));
endmodule 
module LUT2_MUX_15(A0, A1, YN);
    input A0, A1;
    output YN;
    assign YN=0;
endmodule 
module LUT2_MUX_16(A0, A1, S, YN);
    input A0, A1, S;
    output YN;
    assign YN=((~( S )&&( A0 )) || (( S )&&( A1 )));
endmodule 
