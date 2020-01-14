# Makefile to build the NetEngineBCC.exe NetLib4Games example.
# Intended for Borland C++ v5.5

NetEngineBCC.exe: Main.obj NetHasErrorState.obj NetLayer1.obj NetLayer2.obj NetLog.obj NetLowLevel.obj NetMiscRoutines.obj NetThreadUtils.obj NetUtils.obj UDP\NetLowLevelUDP.obj
	bcc32 -eNetEngineBCC.exe $**

.cpp.obj:
	bcc32 -c -IUDP -o$*.obj $<