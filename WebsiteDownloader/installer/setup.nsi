Name "setup website downloader"
OutFile "setup_website_downloader.exe"
RequestExecutionLevel admin
InstallDir "$PROGRAMFILES64\websitedownloader"

Page directory
Page instfiles

Section ""
	SetOutPath $INSTDIR
  	
  	File "D:\clients\matthudgens\WebsiteDownloader\WpfApplication1\bin\Debug\WpfApplication1.exe"
SectionEnd
