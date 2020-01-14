# Basic4GL command line compiler makefile for Borland C++ / C++ Builder
#
# Created 16-Dec-2004 by Tom Mulgrew (tmulgrew@slingshot.co.nz)
#
comp 	= bcc32
mainlibs= c:\prog\bcc55\lib;c:\prog\bcc55\lib\Psdk
libpath = $(mainlibs);..\coronaLibrary\CBuilder
inclpath= ..\vm;..\compiler;..\routines;..\functionlibs;..\standalone;..\coronalibrary
libs 	= kernel32.lib user32.lib comdlg32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib corona.lib ws2_32.lib

#cflags  = -5 -b -RT -tWC -x -xd -Od -v -D_DEBUG -I$(inclpath) -L$(libpath)	# Debug build
cflags 	= -5 -b -RT -tWC -x -xd -O1 -I$(inclpath) -L$(libpath)			# Release build

.cpp.obj:
	&$(comp) -c -n$: $(cflags) $**

Simple.exe: main.obj 				\
	..\vm\haserrorstate.obj 		\
	..\vm\misc.obj 				\
	..\vm\tomvm.obj 			\
	..\vm\vmcode.obj 			\
	..\vm\vmdata.obj 			\
	..\vm\vmDebugger.obj 			\
	..\vm\vmFunction.obj 			\
	..\vm\vmTypes.obj 			\
	..\vm\vmVariables.obj 			\
	..\compiler\compFunction.obj 		\
	..\compiler\compParse.obj 		\
	..\compiler\TomComp.obj 		
#	..\Standalone\Standalone.obj
	$(comp) $(cflags) -eSimple.exe $** $(libs)
