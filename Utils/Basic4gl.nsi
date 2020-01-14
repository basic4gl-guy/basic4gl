; Basic4GLv2.nsi
;
; Basic4GL v 2 install script for NSIS
; Hacked from the StartMenu.nsi example file


;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !define MUI_VERSION "2.6.4"
  
;--------------------------------
;Icons
  
  !define MUI_ICON "Basic4GL\Images\FileIcon.ico"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "Basic4GL\Images\FileIcon2.bmp"
  !define MUI_HEADERIMAGE_RIGHT  
  
;--------------------------------
;General

  ;Name and file
  Name "Basic4GL"
  OutFile "Setup Basic4GL v${MUI_VERSION}.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Basic4GL"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Tom\Basic4GL" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  !define MUI_FINISHPAGE_RUN "$INSTDIR\Basic4GL.exe"

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "ReleaseImage\License.txt"
;  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Basic4GL"  
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Misc

  ;!insertmacro GetSize 
  
;--------------------------------
;Installer Sections

Section "Basic4GL Application" SecMain

  ; Deploy files
  SetShellVarContext all  
  !include "write.txt"
  
  ; Create start menu
  SetOutPath $INSTDIR
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Basic4GL.lnk" "$INSTDIR\Basic4GL.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Demo Programs.lnk" "$DOCUMENTS\Basic4GL"
  
  !insertmacro MUI_STARTMENU_WRITE_END
	
  ;Store installation folder
  WriteRegStr HKCU "Software\Tom\Basic4GL" "" $INSTDIR

     ; Create file type association
    WriteRegStr HKLM "Software\CLASSES\Basic4GL" "" 'Basic4GL Program'
    WriteRegStr HKLM "Software\CLASSES\Basic4GL\shell\open" "" '&Run'
    WriteRegStr HKLM "Software\CLASSES\Basic4GL\shell\open\command" "" '"$INSTDIR\Basic4GL.exe" "%1"'
    WriteRegStr HKLM "Software\CLASSES\Basic4GL\shell\edit" "" '&Edit'
    WriteRegStr HKLM "Software\CLASSES\Basic4GL\shell\edit\command" "" '"$INSTDIR\Basic4GL.exe" -e "%1"'
    WriteRegStr HKLM "Software\CLASSES\Basic4GL\DefaultIcon" "" '"$INSTDIR\Basic4GL.exe",1'
    WriteRegStr HKLM "Software\CLASSES\.gb" "" "Basic4GL"
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ;Add uninstaller to add/remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
                 "DisplayName" "Basic4GL"  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
                 "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
                 "QuietUninstallString" "$\"$INSTDIR\Uninstall.exe$\" /S"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
                 "DisplayIcon" "$\"$INSTDIR\Basic4GL.exe$\",0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
                 "Publisher" "Tom Mulgrew"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
                 "URLInfoAbout" "http://www.basic4gl.net"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
                 "DisplayVersion" "${MUI_VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
                 "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
                 "NoRepair" "1"

  ;Estimated sizes
  ;${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  ;IntFmt $0 "0x%08X" $0
  ;WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL" \
  ;               "EstimatedSize" "$0"
			 
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecMain ${LANG_ENGLISH} "This bit is Basic4GL.."

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\Uninstall.exe"  
  RMDir /r "$INSTDIR"		; Nuke directory and all files it contains

  SetShellVarContext all  
  !include "delete.txt"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
  Delete "$SMPROGRAMS\$StartMenuFolder\Basic4GL.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Demo Programs.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

    ; Remove file type association
    DeleteRegKey HKLM "Software\CLASSES\.gb"
    DeleteRegKey HKLM "Software\CLASSES\Basic4GL"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL"
	
  ;Remove the uninstaller from add/remove programs
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Basic4GL"
  
SectionEnd
