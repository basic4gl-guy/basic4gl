# Basic4GL command line compiler makefile for Borland C++ / C++ Builder
#
# Created 16-Dec-2004 by Tom Mulgrew (tmulgrew@slingshot.co.nz)
#
# Compiler and default flags



# Change this to your Borland C or C++ builder path.
BCCPATH = "f:\Program files\bcc55"



comp 	= bcc32
rescomp = brcc32
link	= ilink32
mainlibs= $(BCCPATH)\lib;$(BCCPATH)\lib\Psdk
libpath = $(mainlibs);..\3rdparty\coronaLibrary\CBuilder
inclpath= "..\vm;..\compiler;..\routines;..\functionlibs;..\standalone;..\3rdparty\coronalibrary;..\NetLib4Games;e:\prog\bcc55\include;..\SoundEngine\B4GLSound;..\DLLInterface;..\DLL SDK\Include"
#libs 	= kernel32.lib user32.lib comdlg32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib corona.lib ws2_32.lib
libs	= corona.lib

#cflags  = -5 -b -RT -W -x -xd -Od -v -D_DEBUG;VM_STATE_STREAMING;STREAM_NAMES -I$(inclpath) -L$(libpath)               # Debug build
cflags  = -5 -b -RT -W -x -xd -O2 -I$(inclpath) -L$(libpath) -DNDEBUG;VM_STATE_STREAMING;STREAM_NAMES	# Release build


.cpp.obj:
	&$(comp) -c -n$: $(cflags) $**

B4GL.exe: main.obj 				\
	..\vm\haserrorstate.obj 		\
	..\vm\misc.obj 				\
	..\vm\tomvm.obj 			\
	..\vm\vmcode.obj 			\
	..\vm\vmdata.obj 			\
	..\vm\vmDebugger.obj 			\
	..\vm\vmFunction.obj 			\
	..\vm\vmTypes.obj 			\
	..\vm\vmVariables.obj 			\
	..\vm\streaming.obj			\
	..\vm\vmStackFrame.obj	\
	..\vm\vmCodeBlock.obj	\
	..\compiler\compFunction.obj		\
	..\compiler\compParse.obj		\
	..\compiler\TomComp.obj			\
	..\compiler\compPreprocessor.obj	\
	..\compiler\DiskFile.obj			\
	..\routines\EmbeddedFiles.obj 		\
	..\routines\glspriteengine.obj 		\
	..\routines\glTextGrid.obj 		\
	..\routines\glWindow.obj 		\
	..\routines\LoadImage.obj 		\
	..\routines\FilenameRoutines.obj	\
	..\FunctionLibs\GLBasicLib_gl.obj 	\
	..\FunctionLibs\GLBasicLib_glu.obj 	\
	..\FunctionLibs\TomFileIOBasicLib.obj 	\
	..\FunctionLibs\TomJoystickBasicLib.obj \
	..\FunctionLibs\TomLibRoutines.obj 	\
	..\FunctionLibs\TomOpenGLBasicLib.obj	\
	..\FunctionLibs\TomStdBasicLib.obj	\
	..\FunctionLibs\TomTextBasicLib.obj	\
	..\FunctionLibs\TomTrigBasicLib.obj	\
	..\FunctionLibs\TomWindowsBasicLib.obj	\
	..\FunctionLibs\TomNetBasicLib.obj	\
	..\FunctionLibs\TomCompilerBasicLib.obj	\
	..\FunctionLibs\TomSoundBasicLib.obj	\
	..\Standalone\Standalone.obj		\
	..\NetLib4Games\NetHasErrorState.obj	\
	..\NetLib4Games\NetLayer1.obj		\
	..\NetLib4Games\NetLayer2.obj		\
	..\NetLib4Games\NetLog.obj		\
	..\NetLib4Games\NetLowLevel.obj		\
	..\NetLib4Games\UDP\NetLowLevelUDP.obj	\
	..\NetLib4Games\NetMiscRoutines.obj	\
	..\NetLib4Games\NetThreadUtils.obj	\
	..\NetLib4Games\NetUtils.obj		\
	..\DLLInterface\TomVMDLLAdapter.obj		\
	..\DLLInterface\PluginDLL.obj			\
	..\DLLInterface\PluginDataStructures.obj
	$(comp) $(cflags) -eB4GL.exe $** $(libs)
