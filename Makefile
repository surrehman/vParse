# ------------------------------------------------------------------------------
# 				Makefile for MarkII
# ------------------------------------------------------------------------------

# Notes:
# 	1. Cextract, the automatic header file generation tool can be confused
#	   with complicated C syntax. With some files, using the GNU C pre-
#	   processor works fine while with some it screws up. Before adding a 
#	   new make target which invokes cextract, test it manually
#
	
CC 		= cc
#CC 		= /users/surrehma/bin/colorgcc
CFLAGS  	= -g  -pg -Wall -Wno-format-zero-length # debugging mode
#CFLAGS 	= -Wall -Wno-format-zero-length         # productive use




BINARIES 	= vDataTC v2v vparser cla_visualize cla_verify gen_sASIC_blib \
		  cla_scrub
		  
BASEOBJECTS 	= vDatatypes.o vCompiler.o strUtils.o formatUtils.o
NONBASEOBJECTS	= lex.yy.o vGrammar.tab.o sVerilog.o sASIC_LUT.o vTransform.o \


all:		strUtils.o vDatatypes.o formatUtils.o vCompiler.o lex.yy.o \
		vGrammar.tab.o sASIC_LUT.o sVerilog.o vTransform.o vDataTC \
		v2v vparser cla_visualize cla_verify cla_scrub gen_sASIC_blib tags

# ------------------------------------------------------------------------------
# Base object files
# ------------------------------------------------------------------------------
strUtils.o:	strUtils.c
		cextract -o strUtils.h strUtils.c
		$(CC) -c $(CFLAGS) strUtils.c
vDatatypes.o:	vDatatypes.c
		#gen_header vDatatypes.c
		$(CC) $(CFLAGS) -c vDatatypes.c
formatUtils.o:	formatUtils.c
		cextract -o formatUtils.h formatUtils.c
		$(CC) $(CFLAGS) -c formatUtils.c		
# ------------------------------------------------------------------------------
# Verilog compiler
# ------------------------------------------------------------------------------
vCompiler.o:	vCompiler.c vDatatypes.o
		cextract -o vCompiler.h vCompiler.c
		$(CC) -c $(CFLAGS) -c vCompiler.c
lex.yy.c:	vTokens.l
		flex vTokens.l
vGrammar.tab.c: vGrammar.y
		bison -t -d vGrammar.y
vGrammar.tab.h: vGrammar.y
		bison -t -d vGrammar.y
lex.yy.o:	lex.yy.c vGrammar.tab.c vGrammar.tab.h
		$(CC) -c $(CFLAGS) lex.yy.c
vGrammar.tab.o: vTokens.l vGrammar.y
		$(CC) -c $(CFLAGS) vGrammar.tab.c
# ------------------------------------------------------------------------------
# Structural Verilog manipulation routines
# ------------------------------------------------------------------------------
sVerilog.o:	sVerilog.c strUtils.o vConstants.h vDatatypes.o
		cextract -o sVerilog.h sVerilog.c
		$(CC) -c $(CFLAGS) -c sVerilog.c

sASIC_LUT.o:	sASIC_LUT.c vConstants.h vDatatypes.o strUtils.o
		cextract -Ycpp -o sASIC_LUT.h sASIC_LUT.c
		$(CC) -c $(CFLAGS) sASIC_LUT.c
# ------------------------------------------------------------------------------
# DAG handling and manipulation
# ------------------------------------------------------------------------------
vTransform.o:	vTransform.c vDatatypes.o strUtils.o sVerilog.o sASIC_LUT.o
		cextract -o vTransform.h vTransform.c
		$(CC) $(CFLAGS) -c vTransform.c
		
		
# ------------------------------------------------------------------------------
# Front end applications
# ------------------------------------------------------------------------------
gen_sASIC_blib: gensASICBehLib.c sASIC_LUT.o formatUtils.o strUtils.o
		$(CC) $(CFLAGS) sASIC_LUT.o formatUtils.o strUtils.o -o \
		gen_sASIC_blib gensASICBehLib.c

v2v:		verilog2verilog.c $(BASEOBJECTS) $(NONBASEOBJECTS)
		$(CC) $(CFLAGS) $(BASEOBJECTS) $(NONBASEOBJECTS)  \
		verilog2verilog.c -o v2v

vparser:	vParser.c $(BASEOBJECTS) $(NONBASEOBJECTS)
		$(CC) $(CFLAGS) $(BASEOBJECTS) $(NONBASEOBJECTS)  \
		vParser.c -o vparser

cla_visualize:	CLAVisualize.c $(BASEOBJECTS) $(NONBASEOBJECTS)
		$(CC) $(CFLAGS) $(BASEOBJECTS) $(NONBASEOBJECTS)  \
		CLAVisualize.c -o cla_visualize

cla_verify:	CLAVerify.c $(BASEOBJECTS) $(NONBASEOBJECTS)
		$(CC) $(CFLAGS) $(BASEOBJECTS) $(NONBASEOBJECTS)  \
		CLAVerify.c -o cla_verify


cla_scrub:	CLAScrub.c $(BASEOBJECTS) $(NONBASEOBJECTS)
		$(CC) $(CFLAGS) $(BASEOBJECTS) $(NONBASEOBJECTS)  \
		CLAScrub.c -o cla_scrub

# ------------------------------------------------------------------------------
# Test cases
# ------------------------------------------------------------------------------
vDataTC:	lex.yy.o vGrammar.tab.o vDatatypes.o vDataTC.c
		$(CC) $(CFLAGS) $(BASEOBJECTS) $(NONBASEOBJECTS) vDataTC.c \
		-o vDataTC


# ------------------------------------------------------------------------------
# Misc.
# ------------------------------------------------------------------------------
# valgrind --tool=callgrind vparser [netlist.v]
# kcachegrind callgrind.out.XXXX

tags: 
	ctags -R -V --links=yes --fields=+afmikKlnsStz --exclude=*.SYNC* -f tags

cleanps:	
		rm -rf *.ps
clean:		
		rm -rf $(BINARIES)
		rm -rf *.o
		rm -rf *.xml *.ps *.txt *.out
		rm -rf vGrammar.tab.c lex.yy.c vGrammar.tab.h 
		rm -rf tags
