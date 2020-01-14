NetEngineBCC.exe: Main.obj NetHasErrorState.obj NetLayer1.obj NetLayer2.obj NetLog.obj NetLowLevel.obj NetMiscRoutines.obj NetThreadUtils.obj NetUtils.obj UDP\NetLowLevelUDP.obj
	bcc32 -eNetEngineBCC.exe $**

.cpp.obj:
	bcc32 -c -IUDP -o$*.obj $<

# ---------------------------------------------------------------------------
#!if !$d(BCB)
#BCB = $(MAKEDIR)\..
#!endif
#
# ---------------------------------------------------------------------------
# IDE SECTION
# ---------------------------------------------------------------------------
# The following section of the project makefile is managed by the BCB IDE.
# It is recommended to use the IDE to change any of the values in this
# section.
# ---------------------------------------------------------------------------
#
#VERSION = BCB.06.00
## ---------------------------------------------------------------------------
#PROJECT = NetEngineBCB.exe
#OBJFILES = Main.obj NetHasErrorState.obj NetLayer1.obj NetLayer2.obj NetLog.obj \
#    NetLowLevel.obj NetMiscRoutines.obj NetThreadUtils.obj NetUtils.obj \
#    UDP\NetLowLevelUDP.obj
#RESFILES = NetEngineBCB.res
#MAINSOURCE = NetEngineBCB.bpf
#RESDEPEN = $(RESFILES)
#LIBFILES = 
#IDLFILES = 
#IDLGENFILES = 
#LIBRARIES = 
##PACKAGES = 
#SPARELIBS = 
#DEFFILE = 
##OTHERFILES = 
## ---------------------------------------------------------------------------
##DEBUGLIBPATH = $(BCB)\lib\debug
#RELEASELIBPATH = $(BCB)\lib\release
##USERDEFINES = _DEBUG
#SYSDEFINES = NO_STRICT;_NO_VCL;_RTLDLL
#INCLUDEPATH = "C:\Projects\NetLib4Games for SourceForge\UDP";"C:\Projects\NetLib4Games for SourceForge";$(BCB)\include;$(BCB)\include\vcl
#LIBPATH = "C:\Projects\NetLib4Games for SourceForge\UDP";"C:\Projects\NetLib4Games for SourceForge";$(BCB)\lib\obj;$(BCB)\lib
#WARNINGS= -w-par
#PATHCPP = .;UDP
#PATHASM = .;
#PATHPAS = .;
#PATHRC = .;
##PATHOBJ = .;$(LIBPATH)
## ---------------------------------------------------------------------------
#CFLAG1 = -Od -H=$(BCB)\lib\vcl60.csm -Hc -Vx -Ve -RT- -X- -r- -a8 -b- -k -y -v -vi- \
#    -tWC -tWM -c
#IDLCFLAGS = 
#PFLAGS = -$YD -$W -$O- -$A8 -v -JPHNE -M
#RFLAGS = 
#AFLAGS = /mx /w2 /zd
#LFLAGS = -D"" -ap -Tpe -x -Gn -v
## ---------------------------------------------------------------------------
#ALLOBJ = c0x32.obj $(OBJFILES)
#ALLRES = $(RESFILES)
#ALLLIB = $(LIBFILES) $(LIBRARIES) import32.lib cw32mti.lib
### ---------------------------------------------------------------------------
#!ifdef IDEOPTIONS
#
#[Version Info]
#IncludeVerInfo=0
#AutoIncBuild=0
#MajorVer=1
#MinorVer=0
#Release=0
#Build=0
#Debug=0
#PreRelease=0
#Special=0
#Private=0
#DLL=0
#
#[Version Info Keys]
#CompanyName=
#FileDescription=
#FileVersion=1.0.0.0
#InternalName=
#LegalCopyright=
#LegalTrademarks=
#OriginalFilename=
#ProductName=
#ProductVersion=1.0.0.0
#Comments=
#
#[Debugging]
#DebugSourceDirs=$(BCB)\source\vcl
#
#!endif
#
#
#
#
#
## ---------------------------------------------------------------------------
## MAKE SECTION
# ---------------------------------------------------------------------------
# This section of the project file is not used by the BCB IDE.  It is for
# the benefit of building from the command-line using the MAKE utility.
# ---------------------------------------------------------------------------
#
#.autodepend
## ---------------------------------------------------------------------------
#!if "$(USERDEFINES)" != ""
#AUSERDEFINES = -d$(USERDEFINES:;= -d)
#!else
#AUSERDEFINES =
#!endif
#
##!if !$d(BCC32)
#BCC32 = bcc32
#!endif
#
#!if !$d(CPP32)
#CPP32 = cpp32
#!endif
#
#!if !$d(DCC32)
#DCC32 = dcc32
#!endif
#
#!if !$d(TASM32)
#TASM32 = tasm32
#!endif
#
##!if !$d(LINKER)
#LINKER = ilink32
#!endif
##
#!if !$d(BRCC32)
#BRCC32 = brcc32
#!endif
#
#
## ---------------------------------------------------------------------------
#!if $d(PATHCPP)
#.PATH.CPP = $(PATHCPP)
#.PATH.C   = $(PATHCPP)
#!endif
#
#!if $d(PATHPAS)
#.PATH.PAS = $(PATHPAS)
#!endif
#
#!if $d(PATHASM)
#.PATH.ASM = $(PATHASM)
#!endif
#
#!if $d(PATHRC)
#.PATH.RC  = $(PATHRC)
#!endif
#
#!if $d(PATHOBJ)
#.PATH.OBJ  = $(PATHOBJ)
#!endif
## ---------------------------------------------------------------------------
#$(PROJECT): $(OTHERFILES) $(IDLGENFILES) $(OBJFILES) $(RESDEPEN) $(DEFFILE)
#    $(BCB)\BIN\$(LINKER) @&&!
#    $(LFLAGS) -L$(LIBPATH) +
#    $(ALLOBJ), +
#    $(PROJECT),, +
#    $(ALLLIB), +
#    $(DEFFILE), +
#    $(ALLRES)
#!
# ---------------------------------------------------------------------------
#.pas.hpp:
#    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }
#
#.pas.obj:
#    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }
#
#.cpp.obj:
#    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }
#
#.c.obj:
#    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }
#
#.c.i:
#    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }
#
#.cpp.i:
#    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }
#
#.asm.obj:
#    $(BCB)\BIN\$(TASM32) $(AFLAGS) -i$(INCLUDEPATH:;= -i) $(AUSERDEFINES) -d$(SYSDEFINES:;= -d) $<, $@
#
#.rc.res:
#    $(BCB)\BIN\$(BRCC32) $(RFLAGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -fo$@ $<
#
#
#
# ---------------------------------------------------------------------------




