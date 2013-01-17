Name "ingram chillin' mod v1.0-rc5 standalone" 

# define name of installer
outFile "icm-v1.0-rc5-standalone.exe"
Icon "standalone\icm\icm.ico"
# define installation directory
#installDir $DESKTOP
SetCompressor /SOLID lzma
#SetCompressor lzma

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
    writeUninstaller "$INSTDIR\uninstall.exe"
	WriteRegStr HKCU "Software\Valve\Half-Life\Settings" "EngineDLL" "hw.dll"
    WriteRegDWORD HKCU "Software\Valve\Half-Life\Settings" "ScreenWindowed" 0x00000001
    WriteRegDWORD HKCU "Software\Valve\Half-Life\Settings" "CrashInitializingVideoMode" 0x00000000
    WriteRegDWORD HKCU "Software\Valve\Half-Life\Settings" "ScreenWidth" 0x00000320
    WriteRegDWORD HKCU "Software\Valve\Half-Life\Settings" "ScreenHeight" 0x00000258
    WriteRegDWORD HKCU "Software\Valve\Half-Life\Settings" "ScreenBPP" 0x00000020
    WriteRegDWORD HKCU "Software\Valve\Half-Life\Settings" "EngineD3D" 0x00000001

    # create a shortcut named "new shortcut" in the start menu programs directory
    # point the new shortcut at the program uninstaller
    CreateDirectory "$SMPROGRAMS\ingram chillin' mod"
    createShortCut "$SMPROGRAMS\ingram chillin' mod\uninstall icm.lnk" "$INSTDIR\uninstall.exe"
    createShortCut "$SMPROGRAMS\ingram chillin' mod\icm.lnk" "$INSTDIR\icm.cmd" "" "$INSTDIR\icm\icm.ico"
    createShortCut "$SMPROGRAMS\ingram chillin' mod\icm chillwiese.lnk" "$INSTDIR\icm-chillwiese.cmd" "" "$INSTDIR\icm\icm.ico"
sectionEnd
 
# uninstaller section start
section "uninstall"

    # first, delete the uninstaller
    delete "$INSTDIR\uninstall.exe"

    RMDir /r "$INSTDIR"
    # second, remove the link from the start menu
    delete "$SMPROGRAMS\ingram chillin' mod\uninstall icm.lnk"
    delete "$SMPROGRAMS\ingram chillin' mod\icm.lnk"
    delete "$SMPROGRAMS\ingram chillin' mod\icm chillwiese.lnk"
	RMDir "$SMPROGRAMS\ingram chillin' mod"
 
# uninstaller section end
sectionEnd

Function .onInit
 
  StrCpy $INSTDIR "$PROGRAMFILES32\icm-hl"
 
FunctionEnd


