# Microsoft Developer Studio Project File - Name="Compiler" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Compiler - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Compiler.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Compiler.mak" CFG="Compiler - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Compiler - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Compiler - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Compiler - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../vm" /I "../compiler" /I "../Routines" /I "../FunctionLibs" /I "../Standalone" /I "../CoronaLibrary" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1409 /d "NDEBUG"
# ADD RSC /l 0x1409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib comdlg32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib corona.lib ws2_32.lib /nologo /subsystem:windows /machine:I386 /out:"Release/B4GL.exe" /libpath:"../CoronaLibrary/Msvc/Release"

!ELSEIF  "$(CFG)" == "Compiler - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../vm" /I "../compiler" /I "../Routines" /I "../FunctionLibs" /I "../Standalone" /I "../CoronaLibrary" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1409 /d "_DEBUG"
# ADD RSC /l 0x1409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib comdlg32.lib gdi32.lib opengl32.lib glu32.lib winmm.lib corona.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/B4GL.exe" /pdbtype:sept /libpath:"../CoronaLibrary/Msvc/Debug"

!ENDIF 

# Begin Target

# Name "Compiler - Win32 Release"
# Name "Compiler - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Compiler\compFunction.cpp
# End Source File
# Begin Source File

SOURCE=..\Compiler\compParse.cpp
# End Source File
# Begin Source File

SOURCE=..\Routines\EmbeddedFiles.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\GLBasicLib_gl.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\GLBasicLib_glu.cpp
# End Source File
# Begin Source File

SOURCE=..\Routines\glspriteengine.cpp
# End Source File
# Begin Source File

SOURCE=..\Routines\glTextGrid.cpp
# End Source File
# Begin Source File

SOURCE=..\Routines\glWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Vm\HasErrorState.cpp
# End Source File
# Begin Source File

SOURCE=..\Routines\LoadImage.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=..\Vm\Misc.cpp
# End Source File
# Begin Source File

SOURCE=..\Standalone\Standalone.cpp
# End Source File
# Begin Source File

SOURCE=..\Compiler\TomComp.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomFileIOBasicLib.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomJoystickBasicLib.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomLibRoutines.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomNetBasicLib.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomOpenGLBasicLib.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomStdBasicLib.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomTextBasicLib.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomTrigBasicLib.cpp
# End Source File
# Begin Source File

SOURCE=..\Vm\TomVM.cpp
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomWindowsBasicLib.cpp
# End Source File
# Begin Source File

SOURCE=..\Vm\vmCode.cpp
# End Source File
# Begin Source File

SOURCE=..\Vm\vmData.cpp
# End Source File
# Begin Source File

SOURCE=..\Vm\vmDebugger.cpp
# End Source File
# Begin Source File

SOURCE=..\Vm\vmFunction.cpp
# End Source File
# Begin Source File

SOURCE=..\Vm\vmTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\Vm\vmVariables.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Compiler\compFunction.h
# End Source File
# Begin Source File

SOURCE=..\Compiler\compParse.h
# End Source File
# Begin Source File

SOURCE=..\Routines\EmbeddedFiles.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\GLBasicLib_gl.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\GLBasicLib_glu.h
# End Source File
# Begin Source File

SOURCE=..\Routines\glspriteengine.h
# End Source File
# Begin Source File

SOURCE=..\Routines\glTextGrid.h
# End Source File
# Begin Source File

SOURCE=..\Routines\glWindow.h
# End Source File
# Begin Source File

SOURCE=..\Vm\HasErrorState.h
# End Source File
# Begin Source File

SOURCE=..\Routines\LoadImage.h
# End Source File
# Begin Source File

SOURCE=..\Vm\Misc.h
# End Source File
# Begin Source File

SOURCE=..\Standalone\Standalone.h
# End Source File
# Begin Source File

SOURCE=..\Compiler\TomComp.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomFileIOBasicLib.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomJoystickBasicLib.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomLibRoutines.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomNetBasicLib.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomOpenGLBasicLib.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomStdBasicLib.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomTextBasicLib.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomTrigBasicLib.h
# End Source File
# Begin Source File

SOURCE=..\Vm\TomVM.h
# End Source File
# Begin Source File

SOURCE=..\FunctionLibs\TomWindowsBasicLib.h
# End Source File
# Begin Source File

SOURCE=..\Vm\vmCode.h
# End Source File
# Begin Source File

SOURCE=..\Vm\vmData.h
# End Source File
# Begin Source File

SOURCE=..\Vm\vmDebugger.h
# End Source File
# Begin Source File

SOURCE=..\Vm\vmFunction.h
# End Source File
# Begin Source File

SOURCE=..\Vm\vmTypes.h
# End Source File
# Begin Source File

SOURCE=..\Vm\vmVariables.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
