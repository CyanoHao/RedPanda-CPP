####################################################################
# Startup

CRCCheck on
SetCompressor /SOLID /FINAL lzma
SetCompressorDictSize 128
SetDatablockOptimize on
Unicode True

!define /ifndef APP_NAME_EN "Red Panda C++"
!define /ifndef APP_NAME_ZH_CN "小熊猫 C++"
!define DISPLAY_NAME "$(StrAppName) ${VERSION} (${DISPLAY_ARCH})"

!define /ifndef FINALNAME "redpanda-cpp-${VERSION}-${DISPLAY_ARCH}.exe"

!define EXE_NAME "RedPandaIDE"
!define INSTALL_NAME "RedPanda-CPP"
!define REGISTRY_PROGRAM_ID "RedPanda-C++"
!define REGISTRY_CLASS_ID "DevCpp"

!define UNINSTKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGISTRY_PROGRAM_ID}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${UNINSTKEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "CurrentUser"
!define MULTIUSER_INSTALLMODE_INSTDIR "${INSTALL_NAME}"
!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE

!if "${ARCH}" != "x86"
!define MULTIUSER_USE_PROGRAMFILES64
!endif

!include "Integration.nsh"
!include "LogicLib.nsh"
!include "MUI2.nsh"
!include "MultiUser.nsh"
!include "WinVer.nsh"
!include "WordFunc.nsh"
!include "x64.nsh"

!include "lang.nsh"

!define MUI_CUSTOMFUNCTION_GUIINIT myGuiInit

####################################################################
# Installer Attributes

Name "${DISPLAY_NAME}"
OutFile "${FINALNAME}"
Caption "${DISPLAY_NAME}"

LicenseData "LICENSE"

####################################################################
# Interface Settings

ShowInstDetails show
AutoCloseWindow false
SilentInstall normal
SetOverwrite try
XPStyle on
ManifestDPIAware true

InstType "$(StrInstTypeFull)"    ;1
InstType "$(StrInstTypeMinimal)" ;2
InstType "$(StrInstTypeSafe)"    ;3

####################################################################
# Pages

!define MUI_ICON "devcpp.ico"
!define MUI_UNICON "devcpp.ico"
!define MUI_ABORTWARNING
!define MUI_LANGDLL_ALLLANGUAGES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${EXE_NAME}.exe"
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_COMPONENTSPAGE_SMALLDESC

!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

####################################################################
# Languages

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "SimpChinese"

####################################################################
# Silently uninstall previous domain-compatible version

Section "" SecUninstallPrevious
  SetRegView 32
  Call UninstallExisting
  SetRegView 64
  Call UninstallExisting
!if "${ARCH}" == "x86"
  SetRegView 32
!endif
SectionEnd

####################################################################
# Files, by option section

Section "$(SectionMainName)" SectionMain
  SectionIn 1 2 3 RO

  SetOutPath $INSTDIR

  ; Allways create an uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr ShCtx "${UNINSTKEY}" "DisplayName" "$(StrAppName) (${DISPLAY_ARCH})"
  WriteRegStr ShCtx "${UNINSTKEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr ShCtx "${UNINSTKEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr ShCtx "${UNINSTKEY}" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
  WriteRegStr ShCtx "${UNINSTKEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr ShCtx "${UNINSTKEY}" "DisplayIcon" "$INSTDIR\${EXE_NAME}.exe"
  WriteRegStr ShCtx "${UNINSTKEY}" "Publisher" "Roy Qu (royqh1979@gmail.com)"
  WriteRegStr ShCtx "${UNINSTKEY}" $MultiUser.InstallMode 1

  ; Write required files
  File "${EXE_NAME}.exe"
  File "consolepauser.exe"
  File "redpanda-win-git-askpass.exe"
  File "astyle.exe"
  File "qt.conf"
  File "LICENSE"
  File "NEWS.md"
  File "README.md"
!ifdef HAVE_OPENCONSOLE
  File "OpenConsole.exe"
!endif
!ifdef HAVE_COMPILER_HINT
  File "compiler_hint.lua"
!endif

  ; Write required paths
  SetOutPath $INSTDIR\templates
  File /nonfatal /r "templates\*"
SectionEnd

!ifdef HAVE_MINGW32
Section "$(SectionMingw32Name)" SectionMingw32
  SectionIn 1 3
  SetOutPath $INSTDIR\mingw32
  File /nonfatal /r "mingw32\*"
SectionEnd
!endif

!ifdef HAVE_MINGW64
Section "$(SectionMingw64Name)" SectionMingw64
  SectionIn 1 3
  SetOutPath $INSTDIR\mingw64
  File /nonfatal /r "mingw64\*"
SectionEnd
!endif

!ifdef HAVE_LLVM
Section "$(SectionLlvmName)" SectionLlvm
  SectionIn 1 3
  SetOutPath $INSTDIR\llvm-mingw
  File /nonfatal /r "llvm-mingw\*"
SectionEnd
!endif

####################################################################
# File association

!macro WriteFileAssoc ext name icon
  WriteRegStr ShCtx "Software\Classes\.${ext}\OpenWithProgids" "${REGISTRY_CLASS_ID}.${ext}" ""
  WriteRegStr ShCtx "Software\Classes\${REGISTRY_CLASS_ID}.${ext}" "" "${name}"
  WriteRegStr ShCtx "Software\Classes\${REGISTRY_CLASS_ID}.${ext}\DefaultIcon" "" "$INSTDIR\${EXE_NAME}.exe,${icon}"
  WriteRegStr ShCtx "Software\Classes\${REGISTRY_CLASS_ID}.${ext}\shell\open\command" "" '"$INSTDIR\${EXE_NAME}.exe" "%1"'

  WriteRegStr ShCtx "Software\${REGISTRY_PROGRAM_ID}\Capabilities\FileAssociations" ".${ext}" "${REGISTRY_CLASS_ID}.${ext}"

  WriteRegStr ShCtx "Software\Classes\Applications\${EXE_NAME}.exe\SupportedTypes" ".${ext}" ""
!macroend

Section "$(SectionFileAssocsName)" SectionFileAssocs
  SectionIn 1 3

  !insertmacro WriteFileAssoc "dev" "$(StrAppName) $(StrProjectFile)" 3
  !insertmacro WriteFileAssoc "c" "C/C++ $(StrSourceFile)" 4
  !insertmacro WriteFileAssoc "cc" "C++ $(StrSourceFile)" 5
  !insertmacro WriteFileAssoc "cp" "C++ $(StrSourceFile)" 5
  !insertmacro WriteFileAssoc "cpp" "C++ $(StrSourceFile)" 5
  !insertmacro WriteFileAssoc "cxx" "C++ $(StrSourceFile)" 5
  !insertmacro WriteFileAssoc "c++" "C++ $(StrSourceFile)" 5
  !insertmacro WriteFileAssoc "h" "C/C++ $(StrHeaderFile)" 6
  !insertmacro WriteFileAssoc "hh" "C++ $(StrHeaderFile)" 7
  !insertmacro WriteFileAssoc "hp" "C++ $(StrHeaderFile)" 7
  !insertmacro WriteFileAssoc "hpp" "C++ $(StrHeaderFile)" 7
  !insertmacro WriteFileAssoc "hxx" "C++ $(StrHeaderFile)" 7
  !insertmacro WriteFileAssoc "h++" "C++ $(StrHeaderFile)" 7
  !insertmacro WriteFileAssoc "tcc" "C++ $(StrHeaderFile)" 7  ; template cc

  WriteRegstr ShCtx "Software\${REGISTRY_PROGRAM_ID}\Capabilities" "ApplicationName" "$(StrAppName)"
  WriteRegstr ShCtx "Software\${REGISTRY_PROGRAM_ID}\Capabilities" "ApplicationDescription" "$(StrAppName)"
  WriteRegStr ShCtx "Software\RegisteredApplications" "${REGISTRY_PROGRAM_ID}" "Software\${REGISTRY_PROGRAM_ID}\Capabilities"

  WriteRegStr ShCtx "Software\Classes\Applications\${EXE_NAME}.exe" "FriendlyAppName" "$(StrAppName)"
  ${NotifyShell_AssocChanged}
SectionEnd

; kick out default app if installer is targeting Windows 8 or later
!if ${REQUIRED_WINDOWS_BUILD} < 9200
!macro CreateSectionForDefaultApp ext displayExt
  Section "${displayExt}" SectionDefaultApp_${ext}
    SectionIn 1 3
    WriteRegStr ShCtx "Software\Classes\.${ext}" "" "${REGISTRY_CLASS_ID}.${ext}"
    ${NotifyShell_AssocChanged}
  SectionEnd
!macroend

SectionGroup "$(SectionDefaultAppName)" SectionDefaultApp
  !insertmacro CreateSectionForDefaultApp "dev" ".dev"
  !insertmacro CreateSectionForDefaultApp "c" ".c/.C"
  !insertmacro CreateSectionForDefaultApp "cc" ".cc"
  !insertmacro CreateSectionForDefaultApp "cp" ".cp"
  !insertmacro CreateSectionForDefaultApp "cpp" ".cpp"
  !insertmacro CreateSectionForDefaultApp "cxx" ".cxx"
  !insertmacro CreateSectionForDefaultApp "c++" ".c++"
  !insertmacro CreateSectionForDefaultApp "h" ".h/.H"
  !insertmacro CreateSectionForDefaultApp "hh" ".hh"
  !insertmacro CreateSectionForDefaultApp "hp" ".hp"
  !insertmacro CreateSectionForDefaultApp "hpp" ".hpp"
  !insertmacro CreateSectionForDefaultApp "hxx" ".hxx"
  !insertmacro CreateSectionForDefaultApp "h++" ".h++"
  !insertmacro CreateSectionForDefaultApp "tcc" ".tcc"
SectionGroupEnd
!endif

####################################################################
# Shortcuts
SectionGroup "$(SectionShortcutsName)" SectionShortcuts
  Section "$(SectionMenuLaunchName)" SectionMenuLaunch
    SectionIn 1 3

    StrCpy $0 $SMPROGRAMS ; start menu Programs folder
    CreateDirectory "$0\$(StrAppName)"
    CreateShortCut "$0\$(StrAppName)\$(StrAppName).lnk" "$INSTDIR\${EXE_NAME}.exe"
    CreateShortCut "$0\$(StrAppName)\License.lnk" "$INSTDIR\LICENSE"
    CreateShortCut "$0\$(StrAppName)\$(StrUninstallAppName).lnk" "$INSTDIR\uninstall.exe"
  SectionEnd

  Section "$(SectionDesktopLaunchName)" SectionDesktopLaunch
    SectionIn 1 3

    CreateShortCut "$DESKTOP\$(StrAppName).lnk" "$INSTDIR\${EXE_NAME}.exe"
  SectionEnd
SectionGroupEnd

Section "$(SectionConfigName)" SectionConfig
  SectionIn 3

  RMDir /r "$APPDATA\${EXE_NAME}"
SectionEnd

####################################################################

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${SectionMain}        "$(MessageSectionMain)"
!ifdef HAVE_MINGW32
!insertmacro MUI_DESCRIPTION_TEXT ${SectionMingw32}     "$(MessageSectionMingw32)"
!endif
!ifdef HAVE_MINGW64
!insertmacro MUI_DESCRIPTION_TEXT ${SectionMingw64}     "$(MessageSectionMingw64)"
!endif
!ifdef HAVE_LLVM
!insertmacro MUI_DESCRIPTION_TEXT ${SectionLlvm}        "$(MessageSectionLlvm)"
!endif
!insertmacro MUI_DESCRIPTION_TEXT ${SectionShortcuts}   "$(MessageSectionShortcuts)"
!insertmacro MUI_DESCRIPTION_TEXT ${SectionFileAssocs}  "$(MessageSectionFileAssocs)"
!if ${REQUIRED_WINDOWS_BUILD} < 9200
!insertmacro MUI_DESCRIPTION_TEXT ${SectionDefaultApp}  "$(MessageSectionDefaultApp)"
!endif
!insertmacro MUI_DESCRIPTION_TEXT ${SectionConfig}      "$(MessageSectionConfig)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

####################################################################
# Functions, utilities

Var /GLOBAL sectionDepFlag
Var /GLOBAL sectionDepTemp
Var /GLOBAL sectionDepIsDefaultSet

!macro CheckDefaultSet ext
  SectionGetFlags ${SectionDefaultApp_${ext}} $sectionDepFlag
  ${If} $sectionDepFlag & ${SF_SELECTED}
    StrCpy $sectionDepIsDefaultSet "1"
  ${EndIf}
!macroend

!macro DisableSection section
  SectionGetFlags ${section} $sectionDepFlag
  IntOp $sectionDepTemp ${SF_SELECTED} ~
  IntOp $sectionDepFlag $sectionDepFlag & $sectionDepTemp
  IntOp $sectionDepFlag $sectionDepFlag | ${SF_RO}
  SectionSetFlags ${section} $sectionDepFlag
!macroend

!macro CheckSectionDeps
  ${If} ${AtLeastBuild} 9200
    !insertmacro DisableSection ${SectionDefaultApp}
    !insertmacro DisableSection ${SectionDefaultApp_dev}
    !insertmacro DisableSection ${SectionDefaultApp_c}
    !insertmacro DisableSection ${SectionDefaultApp_cc}
    !insertmacro DisableSection ${SectionDefaultApp_cp}
    !insertmacro DisableSection ${SectionDefaultApp_cpp}
    !insertmacro DisableSection ${SectionDefaultApp_cxx}
    !insertmacro DisableSection ${SectionDefaultApp_c++}
    !insertmacro DisableSection ${SectionDefaultApp_h}
    !insertmacro DisableSection ${SectionDefaultApp_hh}
    !insertmacro DisableSection ${SectionDefaultApp_hp}
    !insertmacro DisableSection ${SectionDefaultApp_hpp}
    !insertmacro DisableSection ${SectionDefaultApp_hxx}
    !insertmacro DisableSection ${SectionDefaultApp_h++}
    !insertmacro DisableSection ${SectionDefaultApp_tcc}
  ${Else}
    StrCpy $sectionDepIsDefaultSet "0"
    !insertmacro CheckDefaultSet dev
    !insertmacro CheckDefaultSet c
    !insertmacro CheckDefaultSet cc
    !insertmacro CheckDefaultSet cp
    !insertmacro CheckDefaultSet cpp
    !insertmacro CheckDefaultSet cxx
    !insertmacro CheckDefaultSet c++
    !insertmacro CheckDefaultSet h
    !insertmacro CheckDefaultSet hh
    !insertmacro CheckDefaultSet hp
    !insertmacro CheckDefaultSet hpp
    !insertmacro CheckDefaultSet hxx
    !insertmacro CheckDefaultSet h++
    !insertmacro CheckDefaultSet tcc
    ${If} $sectionDepIsDefaultSet == "1"
      IntOp $sectionDepFlag ${SF_SELECTED} | ${SF_RO}
      SectionSetFlags ${SectionFileAssocs} $sectionDepFlag
    ${Else}
      ; remove read-only status
      SectionGetFlags ${SectionFileAssocs} $sectionDepFlag
      IntOp $sectionDepFlag $sectionDepFlag & ${SF_SELECTED}
      SectionSetFlags ${SectionFileAssocs} $sectionDepFlag
    ${EndIf}
  ${EndIf}
!macroend

!macro CheckArmMingw64
  ${If} ${IsNativeARM64}
  ${AndIfNot} ${AtLeastBuild} 22000
    !insertmacro DisableSection ${SectionMingw64}
  ${EndIf}
!macroend

!macro ActionOnSelectionChange
!if ${REQUIRED_WINDOWS_BUILD} < 9200
  !insertmacro CheckSectionDeps
!endif
!if ${ARCH} == "arm64"
  !ifdef HAVE_MINGW64
  !insertmacro CheckArmMingw64
  !endif
!endif
!macroend

Function .onInit
  !insertmacro MULTIUSER_INIT
  !insertmacro MUI_LANGDLL_DISPLAY
!if "${ARCH}" != "x86"
  SetRegView 64
!endif
FunctionEnd

Function .onSelChange
  !insertmacro ActionOnSelectionChange
FunctionEnd

Function myGuiInit
  Var /GLOBAL osArch
  ${If} ${IsNativeIA32}
    StrCpy $osArch "x86"
  ${ElseIf} ${IsNativeAMD64}
    StrCpy $osArch "x64"
  ${ElseIf} ${IsNativeARM64}
    StrCpy $osArch "arm64"
  ${Else}
    StrCpy $osArch "unknown"
  ${EndIf}

  ; Qt prerequisite check
  ${IfNot} ${AtLeastBuild} ${REQUIRED_WINDOWS_BUILD}
    MessageBox MB_OK|MB_ICONSTOP "$(ErrorWindowsBuildRequired)"
    Abort
  ${EndIf}

  ; architecture check
!ifdef HAVE_OPENCONSOLE
  ; OpenConsole.exe is expected to run on native architecture only.
  ; although OpenConsole.exe x64 can run on arm64, here we do not support it.
  ${If} $osArch != "${ARCH}"
    MessageBox MB_OK|MB_ICONSTOP "$(ErrorArchMismatch)"
    Abort
  ${EndIf}
!else
  !if "${ARCH}" == "x64"
  ; x64 cannot be installed on arm64 prior to Windows 11, or x86.
  ${If} ${IsNativeARM64}
  ${AndIfNot} ${AtLeastBuild} 22000
  ${OrIf} ${IsNativeIA32}
    MessageBox MB_OK|MB_ICONSTOP "$(ErrorArchMismatch)"
    Abort
  ${EndIf}
  !else if "${ARCH}" == "arm64"
  ${IfNot} ${IsNativeARM64}
    MessageBox MB_OK|MB_ICONSTOP "$(ErrorArchMismatch)"
    Abort
  ${EndIf}
  !endif
  ${If} $osArch != "${ARCH}"
    MessageBox MB_OK|MB_ICONINFORMATION "$(WarningArchMismatch)"
  ${EndIf}
!endif

  ; Dev C++ based installer check
!if "${ARCH}" != "arm64"
  SetRegView 32
  Call UninstallV2Installer
  SetRegView 64
  Call UninstallV2Installer
  !if "${ARCH}" == "x86"
  SetRegView 32
  !endif
!endif

  !insertmacro ActionOnSelectionChange
FunctionEnd

Function un.onInit
  !insertmacro MULTIUSER_UNINIT
  !insertmacro MUI_UNGETLANGUAGE
!if "${ARCH}" != "x86"
  SetRegView 64
!endif
FunctionEnd

Function UninstallExisting
  Var /GLOBAL UninstallExisting_uninstallString
  Var /GLOBAL UninstallExisting_installLocation

  ReadRegStr $UninstallExisting_uninstallString ShCtx "${UNINSTKEY}" "UninstallString"
  ${If} $UninstallExisting_uninstallString != ""
    ReadRegStr $UninstallExisting_installLocation ShCtx "${UNINSTKEY}" "InstallLocation"
    DetailPrint "$(MessageUninstallingExisting)"
    ; uninstallString already quoted; NSIS requires installLocation unquoted
    ExecWait '$UninstallExisting_uninstallString /S _?=$UninstallExisting_installLocation'
    Delete "$UninstallExisting_installLocation\uninstall.exe"
    RMDir $UninstallExisting_installLocation
  ${EndIf}
FunctionEnd

; Check old installer before v3 refactoring (2.9900)
Function UninstallV2Installer
  Var /GLOBAL UninstallV2Installer_version
  Var /GLOBAL UninstallV2Installer_versionCompareResult
  Var /GLOBAL UninstallV2Installer_uninstaller
  Var /GLOBAL UninstallV2Installer_installLocation
  ReadRegStr $UninstallV2Installer_version HKLM "${UNINSTKEY}" "DisplayVersion"
  ${If} $UninstallV2Installer_version != ""
    ${VersionCompare} "2.9900" "$UninstallV2Installer_version" $UninstallV2Installer_versionCompareResult
    ${If} "$UninstallV2Installer_versionCompareResult" == "1"
      ReadRegStr $UninstallV2Installer_uninstaller HKLM "${UNINSTKEY}" "UninstallString"
      GetFullPathName $UninstallV2Installer_installLocation "$UninstallV2Installer_uninstaller\.." ; remove \uninstall.exe
      MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(MessageUninstallExisting)" /SD IDNO IDOK uninst
      Abort
    uninst:
      ClearErrors
      HideWindow
      ClearErrors
      ExecWait '"$UninstallV2Installer_uninstaller" _?=$UninstallV2Installer_installLocation'
      Delete $UninstallV2Installer_uninstaller
      RMDir $UninstallV2Installer_installLocation
      BringToFront
    ${EndIf}
  ${EndIf}
FunctionEnd

####################################################################
# uninstall

UninstallText "$(MessageUninstallText)"
ShowUninstDetails show

!macro RemoveAssoc ext
  DeleteRegValue ShCtx "Software\Classes\.${ext}\OpenWithProgids" "${REGISTRY_CLASS_ID}.${ext}"
  DeleteRegKey ShCtx "Software\Classes\${REGISTRY_CLASS_ID}.${ext}"
!macroend

Section "Uninstall"
  ; Remove uninstaller
  Delete "$INSTDIR\uninstall.exe"

  ; Remove start menu stuff
  RMDir /r "$SMPROGRAMS\${APP_NAME_EN}"
  RMDir /r "$SMPROGRAMS\${APP_NAME_ZH_CN}"

  ; Remove desktop stuff
  Delete "$QUICKLAUNCH\${APP_NAME_EN}.lnk"
  Delete "$QUICKLAUNCH\${APP_NAME_ZH_CN}.lnk"
  Delete "$DESKTOP\${APP_NAME_EN}.lnk"
  Delete "$DESKTOP\${APP_NAME_ZH_CN}.lnk"

  !insertmacro RemoveAssoc dev
  !insertmacro RemoveAssoc c
  !insertmacro RemoveAssoc cc
  !insertmacro RemoveAssoc cp
  !insertmacro RemoveAssoc cpp
  !insertmacro RemoveAssoc cxx
  !insertmacro RemoveAssoc c++
  !insertmacro RemoveAssoc h
  !insertmacro RemoveAssoc hh
  !insertmacro RemoveAssoc hp
  !insertmacro RemoveAssoc hpp
  !insertmacro RemoveAssoc hxx
  !insertmacro RemoveAssoc h++
  !insertmacro RemoveAssoc tcc

  DeleteRegKey ShCtx "Software\${REGISTRY_PROGRAM_ID}"
  DeleteRegValue ShCtx "Software\RegisteredApplications" "${REGISTRY_PROGRAM_ID}"

  DeleteRegKey ShCtx "Software\Classes\Applications\${EXE_NAME}.exe"
  ${NotifyShell_AssocChanged}

  Delete "$INSTDIR\NEWS.md"
  Delete "$INSTDIR\${EXE_NAME}.exe"
  Delete "$INSTDIR\consolepauser.exe"
  Delete "$INSTDIR\OpenConsole.exe"
  Delete "$INSTDIR\redpanda-win-git-askpass.exe"
  Delete "$INSTDIR\astyle.exe"
  Delete "$INSTDIR\qt.conf"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\compiler_hint.lua"

  RMDir /r "$INSTDIR\templates"
  RMDir /r "$INSTDIR\mingw32"
  RMDir /r "$INSTDIR\mingw64"
  RMDir /r "$INSTDIR\llvm-mingw"

  RMDir "$INSTDIR"

  ; Remove registry keys
  DeleteRegKey ShCtx "${UNINSTKEY}"

  MessageBox MB_YESNO "$(MessageRemoveConfig)" /SD IDNO IDNO SkipRemoveConfig
  RMDir /r "$APPDATA\${EXE_NAME}"
SkipRemoveConfig:
SectionEnd
