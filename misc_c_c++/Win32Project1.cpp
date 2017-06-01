// Win32Project1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32Project1.h"
#include <Shlobj.h>
#include <string>
#include "DriverServer.h"
#include <Logger.h>
#include <curl/curl.h>
#include <regex>
#include "powebservice.h"
#include <vector>
#include <locale>
#include <codecvt>
#include <MacUtils.h>

#define SERVICE_NAME L"ProxyOneService"

struct {
	std::wstring write_folder_path;
} ProgramOptions;

DriverServer driverServer;
Logger Mlogger;
BOOL isTimerDriverStatusGoing;
BOOL isTimerCheckVersionGoing;
BOOL TimerDriverStatusStopped;
bool isTimerPatternUpdateDoing;
HWND hwndStartButton, hwndDlg, hwndWebServiceUrl, hwndStatus, hwndLocalServerPort;

// {899D432C-2DCD-482C-833D-9B2C961187C4}
static const GUID GuidCallOut =
{ 0x899d432c, 0x2dcd, 0x482c, { 0x83, 0x3d, 0x9b, 0x2c, 0x96, 0x11, 0x87, 0xc4 } };

// {1E8027D7-FD3E-42D1-BEDE-8D6B782ECB78}
static const GUID GuidCalloutFilter =
{ 0x1e8027d7, 0xfd3e, 0x42d1, { 0xbe, 0xde, 0x8d, 0x6b, 0x78, 0x2e, 0xcb, 0x78 } };



int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HANDLE hMutex = CreateMutex(NULL, TRUE, L"{fiehvuuihiuefhiue}");
	if (hMutex) {
		DWORD lastError = GetLastError();
		if (lastError == ERROR_ALREADY_EXISTS) {
			MessageBox(NULL, L"There is already an instance running.", L"", 0);
			return 0;
		}
	}

	if (!MacUtils::IsUserAdmin()) {
		MessageBox(NULL, L"You must start this app as an admin; right click -> Run as administrator", L"", 0);
		return 0;
	}

	PWSTR ws_folder_path;
	HRESULT hResult = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &ws_folder_path);
	if (hResult != S_OK) {
		MessageBox(NULL, L"Error getting writable folder", L"", 0);
		CloseHandle(hMutex);
		return 0;
	}
	
	ProgramOptions.write_folder_path = ws_folder_path;
	ProgramOptions.write_folder_path.append(L"\\driver_app_gui\\");
	CoTaskMemFree(ws_folder_path);

	if (!CreateDirectory(ProgramOptions.write_folder_path.c_str(), NULL)) {
		DWORD err = GetLastError();
		if (ERROR_ALREADY_EXISTS != err) {
			MessageBox(NULL, L"Error creating the directory to write to", L"", 0);
			CloseHandle(hMutex);
			return 0;
		}
	}

	std::wstring certFolder = ProgramOptions.write_folder_path + L"certs\\";
	if (!CreateDirectory(certFolder.c_str(), NULL)) {
		DWORD err = GetLastError();
		if (ERROR_ALREADY_EXISTS != err) {
			MessageBox(NULL, L"Error creating the directory to write certs to", L"", 0);
			CloseHandle(hMutex);
			return 0;
		}
	}

	Mlogger.path = ProgramOptions.write_folder_path + L"log2.txt";
	Mlogger.Write("program starting...");
	
	//MessageBox(NULL, ProgramOptions.write_folder_path.c_str(), L"", 0);

	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	DriverServer::mutexLogger = CreateMutex(NULL, FALSE, L"{fiehvuuihiuefhiowuuwfhiuhriufue}");
	DriverServer::writeDir = converterX.to_bytes(ProgramOptions.write_folder_path);
	DriverServer::MLogger = &Mlogger;
	powebservice::Logger = Log;

	WSADATA wsaData;
	int status = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (status) {
		MessageBox(NULL, L"WSAStartUp Failed", L"", 0);
		CloseHandle(hMutex);
		return 0;
	}

	curl_global_init(CURL_GLOBAL_ALL);


	HWND hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), 0, MainDialogProc, 0);
	if (!hDlg) {
		MessageBox(NULL, L"Could not create the main window.", L"", 0);
		WSACleanup();
		CloseHandle(hMutex);
		return 0;
	}

	Mlogger.Write("showing the window...");
	ShowWindow(hDlg, nCmdShow);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

	MSG msg;

	Mlogger.Write("starting the message loop");
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			if (!IsDialogMessage(hDlg, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	
	WSACleanup();
	CloseHandle(hMutex);
	return (int) msg.wParam;
}

int Log(std::string text) {
	return Mlogger.Write(text);
}

void ButtonClickStart(HWND hDlg) {
	HANDLE thread = CreateThread(NULL, 0, ThreadStartService, NULL, 0, NULL);
	if (!thread) {
		MessageBox(hDlg, L"Error creating thread to start the service", L"", 0);
	}
}

void UpdateStatus(std::wstring text) {
	SetWindowText(hwndStatus, text.c_str());
}

DWORD WINAPI ThreadUpdatePatterns(LPVOID lpParameter) {
	HWND hwndButton = GetDlgItem(hwndDlg, IDC_BUTTON_REFRESH_PATTERNS);
	if (hwndButton) {
		UpdateStatus(L"Updating the black list...");
		UpdateBlackList();

		UpdateStatus(L"Updating the white list...");
		UpdateWhiteList();
		

		UpdateStatus(L"Ready");
		EnableWindow(hwndButton, true);
	}

	return 0;
}

DWORD WINAPI ThreadStartService(LPVOID lpParameter) {
	UpdateStatus(L"Loading...");

	/* get web service url*/
	int webServiceUrlLength = (int)SendMessage(hwndWebServiceUrl, WM_GETTEXTLENGTH, 0, 0);
	webServiceUrlLength += 10;
	char *webServiceUrlC = new char[webServiceUrlLength + 1];
	GetDlgItemTextA(hwndDlg, IDC_EDIT_WEB_SERVICE_URL, webServiceUrlC, webServiceUrlLength);

	std::string webServiceUrl;
	webServiceUrl.append(webServiceUrlC);
	delete webServiceUrlC;
	MacUtils::SaveRegStrValue(REG_PATH, L"web_service_url", std::wstring(webServiceUrl.begin(), webServiceUrl.end()));

	/* get the local server port */
	int serverPortLength = (int)SendMessage(hwndLocalServerPort, WM_GETTEXTLENGTH, 0, 0);
	serverPortLength += 10;
	char *serverPortC = new char[serverPortLength + 1];
	GetDlgItemTextA(hwndDlg, IDC_EDIT_LOCAL_SERVER_PORT, serverPortC, serverPortLength);

	std::string serverPort;
	serverPort.append(serverPortC);
	delete serverPortC;
	MacUtils::SaveRegStrValue(REG_PATH, L"local_server_port", std::wstring(serverPort.begin(), serverPort.end()));

	powebservice::Url = webServiceUrl;
	
	driverServer.port = serverPort;

	std::string sslServerPort = "1003";
	
	Mlogger.Write("starting the server on port [" + serverPort + "]...");
	UpdateStatus(L"starting the local server...");
	int serverStatus = driverServer.Init();
	if (serverStatus != DS_ERROR_SUCCESS) {
		std::wstring errMsg;
		switch (serverStatus) {
			case DS_ERROR_MISSING_PORT:
				errMsg = L"The port is missing.";
			break;

			case DS_ERROR_SOCKET:
				errMsg = L"There was a socket error";
			break;

			case DS_ERROR_ADDR:
				errMsg = L"maybe the port is inuse?";
			break;

			default:
				errMsg = L"Unknown error";
			break;
		}
		//MessageBox(hwndDlg, errMsg.c_str(), L"Error starting the local server", 0);
		UpdateStatus(L"Error starting the local server");
		return 0;
	}

	UpdateStatus(L"updating the black list...");
	UpdateBlackList();

	UpdateStatus(L"updating the white list...");
	UpdateWhiteList();

	UpdateStatus(L"starting the windows service...");
	SC_HANDLE scHandleManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (scHandleManager) {
		SC_HANDLE scHandleService = OpenService(scHandleManager, SERVICE_NAME, SC_MANAGER_ALL_ACCESS);
		if (scHandleService) {
			if (!StartService(scHandleService, 0, NULL)) {
				MessageBox(NULL, L"error starting the service...", L"", 0);
				EnableWindow(hwndStartButton, true);
				driverServer.Stop();
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_STOP), true);
			}

			CloseServiceHandle(scHandleService);
		}

		else {
			MessageBox(NULL, L"error getting the service handle", L"", 0);
			driverServer.Stop();
			EnableWindow(hwndStartButton, true);
		}

		CloseServiceHandle(scHandleManager);
	}

	else {
		MessageBox(NULL, L"error opening the service manager", L"", 0);
		driverServer.Stop();
		EnableWindow(hwndStartButton, true);
	}

	UpdateStatus(L"Ready.");
	TimerDriverStatusStopped = FALSE;
	//SetTimer(hwndDlg, TIMER_DRIVER_STATUS, 1000 * 2, NULL);

	return 0;
}

BOOL IsDriverRunning() {
	SC_HANDLE scHandleManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	BOOL isRunning = false;

	if (scHandleManager) {
		SC_HANDLE scHandleService = OpenService(scHandleManager, SERVICE_NAME, SC_MANAGER_ALL_ACCESS);
		if (scHandleService) {
			SERVICE_STATUS_PROCESS serviceStatusProcess;
			DWORD bytesNeeded = 0;
			if (QueryServiceStatusEx(scHandleService,
				SC_STATUS_PROCESS_INFO, (LPBYTE)&serviceStatusProcess,
				sizeof(serviceStatusProcess), &bytesNeeded)) {
				if (serviceStatusProcess.dwCurrentState == SERVICE_RUNNING) {
					isRunning = true;
				}
			}

			else {
				MessageBox(NULL, L"error getting the service status", L"", 0);
			}

			CloseServiceHandle(scHandleService);
		}

		else {
			//MessageBox(NULL, L"error getting the service handle", L"", 0);
		}
	}

	else {
		MessageBox(NULL, L"error opening the service manager", L"", 0);
	}

	return isRunning;
}

void ButtonClickStop() {
	HANDLE thread = CreateThread(NULL, 0, ThreadStopService, NULL, 0, NULL);
	if (!thread) {
		MessageBox(NULL, L"Error creating thread to start the service", L"", 0);
	}
}

DWORD WINAPI ThreadStopService(LPVOID lpParameter) {
	UpdateStatus(L"stopping the window's service...");
	SC_HANDLE scHandleManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (scHandleManager) {
		SC_HANDLE scHandleService = OpenService(scHandleManager, SERVICE_NAME, SC_MANAGER_ALL_ACCESS);
		if (scHandleService) {
			SERVICE_STATUS serviceStatus;
			if (!ControlService(scHandleService, SERVICE_CONTROL_STOP, &serviceStatus)) {
				MessageBox(NULL, L"error stopping the service...", L"", 0);
			}

			CloseServiceHandle(scHandleService);
		}

		else {
			MessageBox(NULL, L"error getting the service handle", L"", 0);
		}

		CloseServiceHandle(scHandleManager);
	}

	else {
		MessageBox(NULL, L"error opening the service manager", L"", 0);
	}

	UpdateStatus(L"stopping the local server...");
	driverServer.Stop();

	UpdateStatus(L"Ready.");
	TimerDriverStatusStopped = FALSE;
	//SetTimer(hwndDlg, TIMER_DRIVER_STATUS, 1000 * 2, NULL);

	EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_STOP), false);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_START), true);

	return 0;
}

void GetVersionFromWebService() {

}

void LoadWebServiceUrl() {
	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_PATH, 0, KEY_READ, &hKey);

	if (lRes == ERROR_SUCCESS) {
		bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
		std::wstring strValueOfBinDir;
		MacUtils::GetStringRegKey(hKey, L"web_service_url", strValueOfBinDir, L"http://");
		SetWindowText(hwndWebServiceUrl, strValueOfBinDir.c_str());

		powebservice::Url = MacUtils::WStrToStr(strValueOfBinDir);

		RegCloseKey(hKey);
	}
}

void LoadLocalServerPort() {
	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_PATH, 0, KEY_READ, &hKey);

	if (lRes == ERROR_SUCCESS) {
		bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
		std::wstring strValueOfBinDir;
		MacUtils::GetStringRegKey(hKey, L"local_server_port", strValueOfBinDir, L"1001");
		SetWindowText(hwndLocalServerPort, strValueOfBinDir.c_str());

		RegCloseKey(hKey);
	}
}

void GetWhiteListLastUpdate(std::string & whiteListUpdate) {
	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_PATH, 0, KEY_READ, &hKey);

	if (lRes == ERROR_SUCCESS) {
		bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
		std::wstring strValueOfBinDir;
		MacUtils::GetStringRegKey(hKey, L"whitelist_last_update", strValueOfBinDir, L"");
		RegCloseKey(hKey);

		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		whiteListUpdate.clear();
		whiteListUpdate.append(converter.to_bytes(strValueOfBinDir));
	}
}

void GetBlackListLastUpdate(std::string & blackListUpdate) {
	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_PATH, 0, KEY_READ, &hKey);

	if (lRes == ERROR_SUCCESS) {
		bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
		std::wstring strValueOfBinDir;
		MacUtils::GetStringRegKey(hKey, L"blacklist_last_update", strValueOfBinDir, L"");
		RegCloseKey(hKey);

		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		blackListUpdate.clear();
		blackListUpdate.append(converter.to_bytes(strValueOfBinDir));
	}
}

void UpdateBlackList() {
	std::string lastUpdate;

	Mlogger.Write("checking black list last update...");
	int err = powebservice::LastBlackListUpdate(lastUpdate);
	if (lastUpdate.length()) {
		std::string localLastUpdate;
		GetBlackListLastUpdate(localLastUpdate);
		Mlogger.Write("black list last update[" + lastUpdate + "] local["+localLastUpdate+"]...");
		if (localLastUpdate.compare(lastUpdate)) {
			std::vector<UrlPattern> matches;

			Mlogger.Write("downloading new black list...");
			powebservice::GetBlackList(matches);
			driverServer.UpdateBlackList(matches);
			MacUtils::SaveRegStrValue(REG_PATH, L"blacklist_last_update", std::wstring(lastUpdate.begin(), lastUpdate.end()));

			std::string t;
			for (int i = 0; i < matches.size(); i++) {
				t.append(matches[i].pattern + " " + matches[i].redirectUrl + " \r\n");
			}

			Mlogger.Write("writing blacklist to disk...");
			MacUtils::WriteToFile(ProgramOptions.write_folder_path + L"blacklist.txt", t);
		}
	}
	else {

	}
}

void UpdateWhiteList() {
	std::string lastUpdate;

	Mlogger.Write("checking whitelist last update...");
	int err = powebservice::LastWhiteListUpdate(lastUpdate);
	if (lastUpdate.length()) {
		std::string localLastUpdate;
		GetWhiteListLastUpdate(localLastUpdate);
		if (localLastUpdate.compare(lastUpdate)) {
			std::vector<UrlPattern> matches;

			Mlogger.Write("downloading whitelist to disk...");
			powebservice::GetWhiteList(matches);
			driverServer.UpdateWhiteList(matches);
			MacUtils::SaveRegStrValue(REG_PATH, L"whitelist_last_update", std::wstring(lastUpdate.begin(), lastUpdate.end()));

			std::string t;
			for (int i = 0; i < matches.size(); i++) {
				t.append(matches[i].host + " "+ matches[i].pattern + " " + matches[i].redirectUrl + " \r\n");
			}

			Mlogger.Write("writing whitelist[" + t + "] to disk...");
			if (!MacUtils::WriteToFile(ProgramOptions.write_folder_path + L"whitelist.txt", t)) {
				Mlogger.Write("writing the whitelist filed.["+MacUtils::NumberToString<DWORD>(GetLastError())+"]");
			}
		}
	}
	else {

	}
}

void LoadLocalWhiteList() {
	std::vector<std::string> lines;
	MacUtils::GetLinesFromFile(ProgramOptions.write_folder_path + L"whitelist.txt", lines);
	std::vector<UrlPattern> vectorP;
	for (int i = 0; i < lines.size(); i++) {
		std::vector<std::string> s;
		std::string line = MacUtils::Trim<std::string>(lines[i]);
		if (line.empty()) continue;
		MacUtils::StrSplit<std::string>(line, ' ', s);
		if (s.size() == 3) {
			vectorP.push_back({ s[0], s[1], s[2] });
			Mlogger.Write("white list added host[" + s[0] + "] pattern["+s[1]+"] redirect url["+s[2]+"]");
		}
		else {
			Mlogger.Write("corrupted white list line[" + line + "]");
		}
	}

	driverServer.UpdateWhiteList(vectorP);
}

void LoadLocalBlackList() {
	std::vector<std::string> lines;
	MacUtils::GetLinesFromFile(ProgramOptions.write_folder_path + L"blacklist.txt", lines);
	std::vector<UrlPattern> vectorP;
	for (int i = 0; i < lines.size(); i++) {
		std::vector<std::string> s;
		MacUtils::StrSplit<std::string>(lines[i], ' ', s);
		if (s.size() == 2) {
			vectorP.push_back({ s[0], s[1] });
		}
	}

	driverServer.UpdateBlackList(vectorP);
}

INT_PTR CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message){
		case WM_INITDIALOG:
		{
			hwndDlg = hDlg;
			hwndStartButton = GetDlgItem(hDlg, IDC_BUTTON_START);
			hwndWebServiceUrl = GetDlgItem(hDlg, IDC_EDIT_WEB_SERVICE_URL);
			hwndStatus = GetDlgItem(hDlg, IDC_STATIC_STATUS);
			hwndLocalServerPort = GetDlgItem(hDlg, IDC_EDIT_LOCAL_SERVER_PORT);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), false);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), false);
			isTimerDriverStatusGoing = false;
			isTimerCheckVersionGoing = false;
			TimerDriverStatusStopped = false;
			isTimerPatternUpdateDoing = false;

			UpdateStatus(L"Loading...");
			LoadWebServiceUrl();
			LoadLocalServerPort();
			LoadLocalWhiteList();
			LoadLocalBlackList();
			//UpdateBlackList();
			//UpdateWhiteList();
			/*
			std::string version;
			powebservice::GetVersion(version);
			if (version.compare(VERSION)) {
				MessageBox(hDlg, L"Update software?", L"", MB_OKCANCEL);
			}*/

			
			//SetTimer(hDlg, TIMER_PATTERN_UPDATE, 1000 * 60 * 5, NULL);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), true);
			UpdateStatus(L"Ready");
			//SetTimer(hDlg, TIMER_DRIVER_STATUS, 1000 * 2, NULL);
		}
		return (INT_PTR)TRUE;

		case WM_TIMER:
			switch (wParam) {
				case TIMER_DRIVER_STATUS:
					if (!isTimerDriverStatusGoing && !TimerDriverStatusStopped) {
						isTimerDriverStatusGoing = true;
						if (IsDriverRunning()) {
							EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), false);
							EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), true);
						}

						else {
							EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_START), true);
							EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_STOP), false);
						}
						isTimerDriverStatusGoing = false;
					}
				break;

				case TIMER_PATTERN_UPDATE:
					if (!isTimerPatternUpdateDoing) {
						isTimerPatternUpdateDoing = true;
						UpdateBlackList();
						UpdateWhiteList();
						isTimerPatternUpdateDoing = false;
					}
				break;
			}
		return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}

			if (LOWORD(wParam) == IDC_BUTTON_START) {
				if (hwndStartButton) {
					TimerDriverStatusStopped = TRUE;
					KillTimer(hDlg, TIMER_DRIVER_STATUS);
					EnableWindow(hwndStartButton, false);
					ButtonClickStart(hDlg);
					//EnableWindow(hwndStartButton, true);
				}
				return (INT_PTR)TRUE;
			}

			if (LOWORD(wParam) == IDC_BUTTON_STOP) {
				HWND hwndStopButton = GetDlgItem(hDlg, IDC_BUTTON_STOP);
				if (hwndStopButton) {
					TimerDriverStatusStopped = TRUE;
					KillTimer(hDlg, TIMER_DRIVER_STATUS);
					EnableWindow(hwndStopButton, false);
					ButtonClickStop();
					//EnableWindow(hwndStopButton, true);
				}
				return (INT_PTR)TRUE;
			}

			if (LOWORD(wParam) == IDC_BUTTON_REFRESH_PATTERNS) {
				HWND hwndButton = GetDlgItem(hDlg, IDC_BUTTON_REFRESH_PATTERNS);
				if (hwndButton) {
					EnableWindow(hwndButton, false);
					HANDLE thread = CreateThread(NULL, 0, ThreadUpdatePatterns, NULL, 0, NULL);
					if (!thread) {
						MessageBox(hDlg, L"Error creating thread to update the patterns", L"", 0);
					}
				}
				return (INT_PTR)TRUE;
			}
			break;

		case WM_CLOSE:
			if (!driverServer.serverGoing) {
				DestroyWindow(hDlg);
			}
			else {
				MessageBox(hDlg, L"Stop the server before exiting...", L"", 0);
			}
		return TRUE;

		case WM_DESTROY:
			if (driverServer.serverGoing) {
				MessageBox(hDlg, L"Stop the server before exiting...", L"", 0);
			}
			else {
				PostQuitMessage(0);
			}
		return TRUE;
	}


	return (INT_PTR)FALSE;
}
