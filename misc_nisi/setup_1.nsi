!include x64.nsh

Name "ThisApp - Setup"
OutFile "ThisApp_Installer.exe"
RequestExecutionLevel admin
InstallDir "$PROGRAMFILES\ThisApp"

!define SERVICE_DIR "..."
!define APP_DIR ".."

Page directory
Page instfiles


Section ".init"
	# your code here
SectionEnd

Section ""
	SetOutPath $INSTDIR
  	
	CreateDirectory "$LOCALAPPDATA\ThisApp\"
	WriteRegStr HKLM "Software\ThisApp" "appWritePath" "$LOCALAPPDATA\ThisApp\"
	MessageBox MB_OK "LOG PATH IS [$LOCALAPPDATA\ThisApp\]"
	
  	File "${SERVICE_DIR}ThisAppLibrary.dll"
	File "${SERVICE_DIR}ThisAppService.exe"
	File "${APP_DIR}ThisApp.exe"
	File "${APP_DIR}ThisApp.exe.config"
	
	writeUninstaller "$INSTDIR\uninstall.exe"
	
	;install service
	SimpleSC::InstallService "ThisAppService" "ThisApp Service" "16" "2" "$INSTDIR\ThisAppService.exe" "" "" ""
	Pop $0
	SimpleSC::StartService "ThisAppService" "" 30
SectionEnd

section "uninstall"
	SimpleSC::StopService "ThisAppService" 1 30
	SimpleSC::RemoveService "ThisAppService"
	Pop $0

	delete "$INSTDIR\*"
	rmdir "$INSTDIR"
	
	delete "$LOCALAPPDATA\ThisApp\*"
	rmdir "$LOCALAPPDATA\ThisApp\"
	
	DeleteRegKey HKLM "Software\ThisApp"
sectionEnd