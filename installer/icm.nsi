Name "ingram chillin' mod v1.0-rc5" 

# define name of installer
outFile "icm-v1.0-rc5.exe"
Icon "icm\icm.ico"
# define installation directory
#installDir $DESKTOP

Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles
 
# For removing Start Menu short cut in Window 7
RequestExecutionLevel admin
 
# start default section
section
 
    # set the installation directory as the destination for the following actions
    setOutPath "$INSTDIR"

    File /r standalone\*.*
 
    # create the uninstaller
    writeUninstaller "$INSTDIR\icm\uninstall.exe"
    
    # create a shortcut named "new shortcut" in the start menu programs directory
    # point the new shortcut at the program uninstaller
    # createShortCut "$SMPROGRAMS\new shortcut.lnk" "$INSTDIR\uninstall.exe"
sectionEnd
 
# uninstaller section start
section "uninstall"

    # first, delete the uninstaller
    delete "$INSTDIR\uninstall.exe"

    RMDir /r "$INSTDIR"
    # second, remove the link from the start menu
    # delete "$SMPROGRAMS\new shortcut.lnk"
 
# uninstaller section end
sectionEnd

Function .onInit
 
  Call GetSteamAccountName
  Pop $R0 ; CaDDy-
  Pop $R1 ; E\Games\Steaaam
 
  # Check if Steam is installed
  StrCmp $R0 "" 0 +3
    StrCpy $INSTDIR "C:\half-life"
    Goto +2
  StrCpy $INSTDIR "$R1\SteamApps\$R0\half-life"
 
FunctionEnd

Function GetSteamAccountName
Push $R0
Push $R1
Push $R2
Push $R3
 
 ReadRegStr $R0 HKCU "Software\Valve\Steam" "SteamExe"
 StrCmp $R0 "" noSteam
 
 StrCpy $R1 0
  IntOp $R1 $R1 - 1
  StrCpy $R2 $R0 1 $R1
  StrCmp $R2 "" noAccount
  StrCmp $R2 "/" 0 -3
 StrCpy $R0 $R0 $R1
 
 StrCpy $R1 0
  IntOp $R1 $R1 + 1
  StrCpy $R2 $R0 1 -$R1
  StrCmp $R2 "" +8
  StrCmp $R2 "/" 0 -3
 StrCpy $R2 $R0 -$R1
 IntOp $R1 $R1 - 1
 StrCpy $R3 $R0 "" -$R1
 StrCpy $R0 "$R2\$R3"
 IntOp $R1 $R1 + 1
 Goto -9
 
 FindFirst $R1 $R2 "$R0\steamapps\*.*"
 loopFile:
  StrCmp $R2 "SourceMods" nextFile
  #added this for the newest steam (common/media is other content (like trailers and 3rd party games)
  StrCmp $R2 "common"     nextFile
  StrCmp $R2 "media"      nextFile
  StrCmp $R2 "."          nextFile
  StrCmp $R2 ".."         nextFile
 
  IfFileExists "$R0\steamapps\$R2\*.*" done
 
 nextFile:
  ClearErrors
  FindNext $R1 $R2
  IfErrors 0 loopFile
 FindClose $R1
 
 noAccount:
  StrCpy $R2 "[ACCOUNT_NAME]"
 
 done:
  StrCpy $R1 $R0
  StrCpy $R0 $R2
  Goto +3
 
 noSteam:
  StrCpy $R1 ""
  StrCpy $R0 ""
 
Pop $R3
Pop $R2
Exch $R1
Exch
Exch $R0
FunctionEnd
