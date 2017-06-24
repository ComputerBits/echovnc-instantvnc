//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

// WinVNC.cpp

// 24/11/97		WEZ

#define USE_LOG_FILE
// WinMain and main WndProc for the new version of WinVNC
#include "string.h"
const char *szModuleName = "[app] ";
volatile bool g_bStopped = false;
static char *szExecutable = NULL;
static char *szPath = NULL;

#define SZ_ECHOWARE_DLL "Echoware.dll"
#define SAFE_TRACE
////////////////////////////
// System headers
#include "stdhdrs.h"

////////////////////////////
// Custom headers
#include "VSocket.h"
#include "WinVNC.h"

#include "vncServer.h"
#include "vncMenu.h"
#include "vncOSVersion.h"
#include "vncusernamedlg.h"

#include "vncLog.h"

VNCLog vnclog;

#define LOCALIZATION_MESSAGES   // ACT: full declaration instead on extern ones
#include "localization.h" // Act : add localization on messages


#include "ElevatePrivilege.h"


//#include "..\Common\UserData.h"	//#IR:100315: - : moved to the "winvnc.h"
#include "..\Common\EchoWareDll.h"
#include "..\Common\ConfigDlg.h"
#include "..\Common\ParseCmdLine.h"

#include "CirDlg_Help.h"	//#IR:100303 +
#include "Tlhelp32.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Service Code ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>

#include <time.h>
#include "Wtsapi32.h"
#include "Userenv.h"

#include <tchar.h>
#include <psapi.h>

DWORD CAD_SubService(LPVOID lpParam);
DWORD LaunchProcessWin(DWORD dwSessionId);
void checkSpawnInstantVNC();

extern void CloseConnection();
extern int ConnectAgain();
void CloseStartupDlg();
static DWORD StopService();

#define SERVICE_NAME "InstantVNCsrvc"
#define SERVICE_DISPLAY_NAME "InstantWinVNC Service"
#define SZ_AUTORUN "autorun"

#define MAX_SESSIONS 10

struct {
	int sessionId;
	_WTS_CONNECTSTATE_CLASS sessionStatus;
	int pingRetries;
	bool initialHandshake;
	DWORD pid;
} sessionInfo;

enum {
	INSTANT_VNC_SRVCERR_NONE,
	INSTANT_VNC_SRVCERR_UNKNOWN,
	INSTANT_VNC_SRVCERR_CANTCONNECT,
	INSTANT_VNC_SRVCERR_TIMEOUT,
};

HANDLE g_hStopEvent = INVALID_HANDLE_VALUE;
static SERVICE_STATUS g_svcStatus;
static SERVICE_STATUS_HANDLE g_svcStatusHandle;

#pragma data_seg(".SHARED")
long g_nClients = 0;
#pragma data_seg()

volatile LONG g_bServiceStopping = false;
volatile LONG g_bServiceStopped = true;
volatile LONG g_bPipeSystemClosed = true;
volatile LONG g_bPipeHelperClosed = true;
static int g_bRunAsService = false;
static HANDLE hServiceStarted = INVALID_HANDLE_VALUE;
HANDLE g_hEventCAD = INVALID_HANDLE_VALUE;

volatile bool g_bMainWindowStarted = false;
HWND g_hwndTray = NULL;
vncMenu *menu;
static vncServer* g_pServer = NULL;
static CParseCmdLine *cmd = NULL;
static CEchoWareChannel echoDll;

static DWORD dwThreadPipeClientId = 0;

volatile static int firstSubService = 1;

static void WINAPI VNC_ServiceCtrlHandler(__in DWORD opcode);
bool GetEchoWareData(std::string& sAddr, std::string& sUserName, std::string& sPass);
static int srvc_init(int nargin, const char* args);
static int srvc_main();
static int srvc_cleanup();

bool isPidProcessName(DWORD processID, const char *szTargetProcessName)
{
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	static DWORD(WINAPI *dllGetProcessImageFileName)(HANDLE, LPSTR, DWORD) = NULL;
	*(FARPROC*)&dllGetProcessImageFileName = GetProcAddress(LoadLibrary("psapi.dll"), "GetProcessImageFileNameA");

	// Get a handle to the process.
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_QUERY_INFORMATION,
									FALSE, processID);

	if (NULL == hProcess)
	{
		return false;
	}

	if (dllGetProcessImageFileName)
	{
		dllGetProcessImageFileName(hProcess, szProcessName, sizeof(szProcessName)/sizeof(TCHAR));
	}
	else
	{
		ExitProcess(1);
	}
	CloseHandle(hProcess);

	return strstr(szProcessName, szTargetProcessName) != NULL;
}

int isLogonRunning()
{
	static BOOL(WINAPI *dllEnumProcesses)(DWORD*, DWORD, DWORD*);
	*(FARPROC*)&dllEnumProcesses = GetProcAddress(LoadLibrary("psapi.dll"), "EnumProcesses");

	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if (!dllEnumProcesses)
	{
		ExitProcess(0);
		return 0;
	}
	if (!dllEnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		return 0;
	}

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);

	for (i = 0; i < cProcesses; i++)
	{
		if (aProcesses[i] != 0)
		{
			if (isPidProcessName(aProcesses[i], "winlogon.exe"))
				return true;
		}
	}

	return false;
}

int getTransId()
{
	static int transId = 0;
	transId = (transId + 1) % 1024;
	if (!transId)
		transId = 1;
	return transId;
}

void CPipe::buildIpcMessage(CPipe::EnumIPCMessages msg_type, int length, const void *data, CPipe::CIpcMessage &msg)
{
	switch (msg_type)
	{
	case IPC_MESSAGE_HELLO:
		msg.length = sizeof(CPipe::CIpcMessage);
		strncpy_s(msg.data, (const char*)data, length);
		break;
	case IPC_MESSAGE_SET_TRAY_HWND:
		msg.length = 4;
		*(int*)msg.data = (int)data;
		break;
	case IPC_MESSAGE_GET_TRAY_INFO:
		msg.length = sizeof(CPipe::CIpcMessage);
		break;
	case IPC_MESSAGE_CLIENT_ASK_CONNECT:
	case IPC_MESSAGE_CLIENT_ASK_DISCONNECT:
		msg.length = sizeof(CPipe::CIpcMessage) - IPC_TRAY_MAX_DATA;
		break;
	case IPC_MESSAGE_SUB_SERVICE_ALIVE:
		msg.length = sizeof(CPipe::CIpcMessage) - IPC_TRAY_MAX_DATA;
		break;
	case IPC_MESSAGE_SUB_SERVICE_ASK_CAD:
		msg.length = sizeof(CPipe::CIpcMessage) - IPC_TRAY_MAX_DATA;
		break;
	default:
		break;
	}
	msg.msg_type = msg_type;
	msg.trans_id = getTransId();
	msg.pid = GetCurrentProcessId();
	ProcessIdToSessionId(msg.pid, &msg.sessionId);
}

static DWORD WINAPI WaitForClientPipeSubService(void *param)
{
	HANDLE hPipeService = (HANDLE)param;

	char buffer[sizeof(CPipe::CIpcMessage)];
	CPipe::CIpcMessage *ipcMessage = (CPipe::CIpcMessage*)buffer;

	g_bPipeSystemClosed = false;

	while (true)
	{
		if (WaitForSingleObject(g_hStopEvent, 50) == WAIT_OBJECT_0)
		{
			break;
		}
		if (ConnectNamedPipe(hPipeService, NULL) == 0)
		{
			Sleep(50);
		}
		else
		{
			if (WaitForSingleObject(g_hStopEvent, 0) == WAIT_OBJECT_0)
			{
				return 0;
				break;
			}

			DWORD dwRead = 0;
			DWORD dwWritten = 0;
			ReadFile(hPipeService, buffer, sizeof(CPipe::CIpcMessage), &dwRead, NULL);
			if (dwRead > 0)
			{
				buffer[dwRead - 1] = 0;

				if ((sessionInfo.pid == ipcMessage->pid) && (sessionInfo.sessionId == ipcMessage->sessionId))
				{
					sessionInfo.pingRetries = 0;
					sessionInfo.initialHandshake = false;
					switch (ipcMessage->msg_type)
					{
						case CPipe::IPC_MESSAGE_SUB_SERVICE_ALIVE:
							strncpy_s((char*)ipcMessage->data, IPC_TRAY_MAX_DATA, "Server ACK", IPC_TRAY_MAX_DATA - 1);
							WriteFile(hPipeService, buffer, sizeof(CPipe::CIpcMessage), &dwWritten, NULL);
							break;
						case CPipe::IPC_MESSAGE_HELLO:
							strncpy_s((char*)ipcMessage->data, IPC_TRAY_MAX_DATA, "Service: Sub-service HELLO", IPC_TRAY_MAX_DATA - 1);
							WriteFile(hPipeService, buffer, sizeof(CPipe::CIpcMessage), &dwWritten, NULL);
							break;
						case CPipe::IPC_MESSAGE_SUB_SERVICE_ASK_CAD:
							CAD_SubService(NULL);
							strncpy_s((char*)ipcMessage->data, IPC_TRAY_MAX_DATA, "Service: Sub-service CAD", IPC_TRAY_MAX_DATA - 1);
							WriteFile(hPipeService, buffer, sizeof(CPipe::CIpcMessage), &dwWritten, NULL);
							break;
						default:
							strncpy_s((char*)ipcMessage->data, IPC_TRAY_MAX_DATA, "AUTOMATIC RESPONSE", IPC_TRAY_MAX_DATA - 1);
							WriteFile(hPipeService, buffer, sizeof(CPipe::CIpcMessage), &dwWritten, NULL);
							break;
					}
				}
			}
			FlushFileBuffers(hPipeService); 
			DisconnectNamedPipe(hPipeService); 
		}
	}
	FlushFileBuffers(hPipeService); 
	DisconnectNamedPipe(hPipeService); 

	CloseHandle(hPipeService); 
	g_bPipeSystemClosed = true;

	return 0;
}

static DWORD WINAPI WaitForClientPipeHelper(void *param)
{
	HANDLE hPipeTray = (HANDLE)param;

	char buffer[sizeof(CPipe::CIpcMessage) + 1];
	CPipe::CIpcMessage *ipcMessage = (CPipe::CIpcMessage*)buffer;

	g_bPipeHelperClosed = false;
	while (true)
	{
		if (WaitForSingleObject(g_hStopEvent, 1) == WAIT_OBJECT_0)
		{
			return 0;
			break;
		}
		if (ConnectNamedPipe(hPipeTray, NULL) == 0)
		{
			Sleep(100);
		}
		else
		{
			if (WaitForSingleObject(g_hStopEvent, 0) == WAIT_OBJECT_0)
			{
				return 0;
				break;
			}
			//			TRACE("new connected helperTray");
			DWORD dwRead = 0;
			DWORD dwWritten = 0;
			ReadFile(hPipeTray, buffer, sizeof(CPipe::CIpcMessage), &dwRead, NULL);
			if (dwRead > 0)
			{
				buffer[dwRead] = 0;
				//				TRACE("read %d bytes from remote client", dwRead);
				switch (ipcMessage->msg_type)
				{
				case CPipe::IPC_MESSAGE_HELLO:
					strncpy_s((char*)ipcMessage->data, IPC_TRAY_MAX_DATA, "Service: Helper tray HELLO", IPC_TRAY_MAX_DATA - 1);
					WriteFile(hPipeTray, buffer, sizeof(CPipe::CIpcMessage), &dwWritten, NULL);
					break;
				case CPipe::IPC_MESSAGE_GET_TRAY_INFO:
					{
						ConnInfo *connInfo = (ConnInfo *)ipcMessage->data;

						connInfo->szAddress[0] = connInfo->szUser[0] = connInfo->szPasswd[0] = '\0';
						if (g_pServer->SockConnected())
						{
							std::string sAddr, sUserName, sPass;
							if (GetEchoWareData(sAddr, sUserName, sPass))
							{
								strncat_s(connInfo->szAddress, sizeof(connInfo->szAddress), sAddr.c_str(), sizeof(connInfo->szAddress) - 1);
								strncat_s(connInfo->szUser, sizeof(connInfo->szUser), sUserName.c_str(), sizeof(connInfo->szUser) - 1);
								strncat_s(connInfo->szPasswd, sizeof(connInfo->szPasswd), sPass.c_str(), sizeof(connInfo->szPasswd) - 1);
							}
						}
						WriteFile(hPipeTray, buffer, sizeof(CPipe::CIpcMessage), &dwWritten, NULL);
						break;
					}
					case CPipe::IPC_MESSAGE_CLIENT_ASK_CONNECT:
					case CPipe::IPC_MESSAGE_CLIENT_ASK_DISCONNECT:
						WriteFile(hPipeTray, buffer, sizeof(CPipe::CIpcMessage), &dwWritten, NULL);
						if (g_pServer->SockConnected())
						{
							g_pServer->KillAuthClients(TRUE);
							CloseConnection();
							CloseStartupDlg();
						}
						else
						{
							g_pServer->SockConnect(TRUE);
						}
						break;
					default:
						strncpy_s((char*)ipcMessage->data, IPC_TRAY_MAX_DATA, "AUTOMATIC RESPONSE", IPC_TRAY_MAX_DATA - 1);
						WriteFile(hPipeTray, buffer, sizeof(CPipe::CIpcMessage), &dwWritten, NULL);
						break;
				}
			}
			FlushFileBuffers(hPipeTray); 
			DisconnectNamedPipe(hPipeTray); 
		}
	}
	FlushFileBuffers(hPipeTray); 
	DisconnectNamedPipe(hPipeTray); 

	CloseHandle(hPipeTray); 
	g_bPipeHelperClosed = true;

	return 0;
}

const char *getStringError(LONG err)
{
	static char szErrorString[MAX_PATH];

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		(err == -1) ? GetLastError() : err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) szErrorString,
		MAX_PATH,
		NULL
		);
	return szErrorString;
}

static void WINAPI VNC_ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD status;
	DWORD errorCode;

	g_svcStatusHandle = RegisterServiceCtrlHandler((LPCTSTR)SERVICE_NAME, (LPHANDLER_FUNCTION)VNC_ServiceCtrlHandler);

	if (!g_svcStatusHandle)
	{
		errorCode = GetLastError();
		SetEvent(hServiceStarted);
		return;
	}

	g_svcStatus.dwServiceType = SERVICE_WIN32;
	g_svcStatus.dwCurrentState = SERVICE_START_PENDING;
	g_svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	g_svcStatus.dwWin32ExitCode = 0;
	g_svcStatus.dwServiceSpecificExitCode = 0;
	g_svcStatus.dwCheckPoint = 1;
	g_svcStatus.dwWaitHint = 500;

	if (!SetServiceStatus(g_svcStatusHandle, &g_svcStatus))
	{
		errorCode = GetLastError();
		SetEvent(hServiceStarted);
		return;
	}

	if(!SetProcessShutdownParameters(0x4FF, 0))
	{
		g_svcStatus.dwCurrentState = SERVICE_STOPPED;
		g_svcStatus.dwCheckPoint = 0;
		g_svcStatus.dwWaitHint = 0;
		g_svcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		g_svcStatus.dwServiceSpecificExitCode = 1;
		SetServiceStatus(g_svcStatusHandle, &g_svcStatus);
		SetEvent(hServiceStarted);
		return;
	}

	/* Initialize */
	status = srvc_init(argc, (char*)argv);
	if (status != INSTANT_VNC_SRVCERR_NONE)
	{
		g_svcStatus.dwCurrentState = SERVICE_STOPPED;
		g_svcStatus.dwCheckPoint = 0;
		g_svcStatus.dwWaitHint = 0;
		g_svcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		g_svcStatus.dwServiceSpecificExitCode = status;
		SetServiceStatus(g_svcStatusHandle, &g_svcStatus);
		srvc_cleanup();
		SetEvent(hServiceStarted);
		return;
	}

	g_svcStatus.dwServiceType = SERVICE_WIN32;
	g_svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	g_svcStatus.dwWin32ExitCode = 0;
	g_svcStatus.dwServiceSpecificExitCode = 0;
	g_svcStatus.dwCurrentState = SERVICE_RUNNING;
	g_svcStatus.dwCheckPoint = 1;
	g_svcStatus.dwWaitHint = 0;

	if (!SetServiceStatus(g_svcStatusHandle, &g_svcStatus))
	{
		errorCode = GetLastError();
		SetEvent(hServiceStarted);
		return;
	}

	g_bRunAsService = true;
	g_bServiceStopped = false;
	SetEvent(hServiceStarted);

	/* main processing function */
	if (srvc_main() != INSTANT_VNC_SRVCERR_NONE)
	{
		//srv_main ended abnormally
	}

	/* uninitialize */
	if (srvc_cleanup() != INSTANT_VNC_SRVCERR_NONE)
	{
		//srv_uninit ended abnormally
	}

	g_svcStatus.dwWin32ExitCode = 0;
	g_svcStatus.dwCheckPoint = 50;
	g_svcStatus.dwWaitHint = 5000;
	g_svcStatus.dwCurrentState = SERVICE_STOPPED;
	if (!SetServiceStatus(g_svcStatusHandle, &g_svcStatus))
	{
		errorCode = GetLastError();
		return;
	}
}

static int srvc_init(int nargin, const char* args)
{
	return INSTANT_VNC_SRVCERR_NONE;
}

int UninstallHelperTray()
{
	int ret = 0;
	HKEY hKey;
	const char *szRegSubKeyRun = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegSubKeyRun, 0, KEY_WRITE, &hKey))
	{
		if (ERROR_SUCCESS != RegDeleteValue(hKey, "InstantVNC"))
		{
			ret = -1;					
		}
		RegCloseKey(hKey);
	}
	else
	{
		ret = -1;
	}
	return ret;
}

int InstallHelperTray(const char *szFullPath)
{
	HKEY hKey;
	const char *szRegSubKeyRun = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	int result = ERROR_REGISTRY_IO_FAILED;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegSubKeyRun, 0, KEY_WRITE, &hKey))
	{
		std::string strKeyValue(szFullPath);

		strKeyValue.append(" -helperTray");
		result = RegSetValueEx(hKey, "InstantVNC", 0, REG_SZ, (const BYTE*)(strKeyValue.c_str()), 1 + strKeyValue.length());
		RegCloseKey(hKey);
	}

	return result;
}

static int srvc_main()
{
	g_hStopEvent = CreateEvent(NULL, true, false, NULL);

	while(true)
	{
		if (WaitForSingleObject(g_hStopEvent, 100) == WAIT_OBJECT_0)
		{
			break;
		}
		Sleep(999);
	}
	return INSTANT_VNC_SRVCERR_NONE;
}

static int srvc_cleanup()
{
	return 0;
}

static BOOL IsServiceInstalled(LPSTR szServiceName)
{
#define STATUS_BUF_SIZE 1024
	SC_HANDLE schSCManager;
	DWORD errcode, bytes_needed, num_services, resume_handle;
	BYTE StatusBuffer[STATUS_BUF_SIZE];
	unsigned i;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		return FALSE;
	}

	resume_handle = 0;

	do 
	{
		errcode = 0;
		if (!EnumServicesStatus(schSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, (LPENUM_SERVICE_STATUS)StatusBuffer, STATUS_BUF_SIZE, &bytes_needed, &num_services, &resume_handle))
		{
			errcode = GetLastError();
			if (errcode != ERROR_MORE_DATA)
				break;
		} 

		for (i = 0; i < num_services; i++)
		{
			if (!strcmp(((LPENUM_SERVICE_STATUS)StatusBuffer + i)->lpServiceName, szServiceName))
			{
				CloseServiceHandle(schSCManager);
				return TRUE;
			}
		}
	} while (errcode == ERROR_MORE_DATA);

	CloseServiceHandle(schSCManager);
	return FALSE;
}

BOOL IsServiceInstalled()
{
	return IsServiceInstalled(SERVICE_NAME);
}

DWORD InstallService(const char *szCmdLine, std::string &strAbsoluteFileName)
{
	SC_HANDLE schService, schSCManager = NULL;
	DWORD errcode = 0;
	std::string strQuotedServicePath("");
	char szCurrentAppPath[MAX_PATH];
	std::string strBinaryPathName("");
	char szProgramFiles[MAX_PATH];
	int pos;

	if (GetModuleFileName(NULL, szCurrentAppPath, MAX_PATH) == 0)
	{
		return GetLastError();
	}

	if (GetEnvironmentVariable("ProgramFiles", szProgramFiles, MAX_PATH) == 0)
	{
		return GetLastError();
	}

	pos = strlen(szCurrentAppPath) - 1;
	while (pos && szCurrentAppPath[pos] != '\\') pos--;

	strBinaryPathName.append(szProgramFiles);
	strBinaryPathName.append("\\Echogent");
	CreateDirectory(strBinaryPathName.c_str(), NULL);
	strBinaryPathName.append("\\InstantVNC");
	CreateDirectory(strBinaryPathName.c_str(), NULL);
	strBinaryPathName.append("\\");
	strBinaryPathName.append(szCurrentAppPath + pos + 1);

	strQuotedServicePath.append("\"");
	strQuotedServicePath.append(strBinaryPathName);
	strQuotedServicePath.append("\"");
	strAbsoluteFileName.assign(strQuotedServicePath);

	strQuotedServicePath.append(" ");
	strQuotedServicePath.append(szCmdLine);

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		if (errcode != ERROR_ACCESS_DENIED)
		{
			MessageBox(NULL, "Installation of service Failed", szCurrentAppPath + pos + 1, MB_ICONASTERISK);
		}
		return errcode;
	}

	schService = CreateService(
		schSCManager,                /* SCManager database  */
		SERVICE_NAME,                /* name of service */
		SERVICE_DISPLAY_NAME,        /* service name to display */
		SERVICE_START | SERVICE_STOP | SERVICE_INTERROGATE,    /* desired access */
		SERVICE_WIN32_OWN_PROCESS,	/* service type */
		SERVICE_AUTO_START,			/* start type */
		SERVICE_ERROR_NORMAL,		/* error control type */
		strQuotedServicePath.c_str(),	/* service's binary */
		NULL,                      /* no load ordering group */
		NULL,                      /* no tag identifier */
		NULL,					    /* no dependencies */
		NULL,                      /* LocalSystem account */
		NULL);                     /* no password */

	if (schService == NULL)
	{
		errcode = GetLastError();
		if (errcode != ERROR_ACCESS_DENIED)
		{
			MessageBox(NULL, "Installation of service Failed", szCurrentAppPath + pos + 1, MB_ICONASTERISK);
		}
	}
	else 
	{
		if (CopyFile(szCurrentAppPath, strBinaryPathName.c_str(), true))
		{
			std::string strDependencyDllSrc(szCurrentAppPath);
			std::string strDependencyDllDst(strBinaryPathName);
			strDependencyDllSrc.replace(strDependencyDllSrc.find(szCurrentAppPath + pos + 1), strlen(szCurrentAppPath + pos + 1), SZ_ECHOWARE_DLL);
			strDependencyDllDst.replace(strDependencyDllDst.find(szCurrentAppPath + pos + 1), strlen(szCurrentAppPath + pos + 1), SZ_ECHOWARE_DLL);
			if (CopyFile(strDependencyDllSrc.c_str(), strDependencyDllDst.c_str(), true))
			{				
				MessageBox(NULL, "Service installed successfully. InstantVNC will auto-start the next time you reboot.", szCurrentAppPath + pos + 1, MB_OK);
			}
			else
			{
				errcode = GetLastError();
			}
		}
		else
		{
			errcode = GetLastError();
		}
	}

	if (schService != NULL)
	{
		CloseServiceHandle(schService);
	}

	CloseServiceHandle(schSCManager);
	return errcode;
}

DWORD UninstallService()
{
	SC_HANDLE schService, schSCManager;
	DWORD errcode;

	errcode = 0;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		if (errcode != ERROR_ACCESS_DENIED)
		{
			MessageBox(NULL, "Removal of service Failed", SERVICE_DISPLAY_NAME, MB_ICONASTERISK);
		}
		return errcode;
	}

	schService = OpenService(
		schSCManager,              /* SCManager database  */
		SERVICE_NAME,              /* name of service */
		DELETE | SERVICE_QUERY_CONFIG);

	if (schService == NULL)
	{
		errcode = GetLastError();
	}
	else
	{
		char buffer[8192];
		LPQUERY_SERVICE_CONFIG lpQsc = (LPQUERY_SERVICE_CONFIG)(buffer);
		DWORD dwNeeded;
		bool bStopService = g_nClients == 0;
		std::string strBinaryPathName("");
		int result = QueryServiceConfig(schService, lpQsc, sizeof(buffer), &dwNeeded);
		char tempFile[MAX_PATH];
		memset(tempFile, '\0', MAX_PATH);

		if (result)
		{
			int pos;

			strBinaryPathName.append(lpQsc->lpBinaryPathName);
			lpQsc = NULL;

			if (strBinaryPathName.at(0) == '"')
			{
				strBinaryPathName.erase(0, 1);
				pos = strBinaryPathName.find('"', 1);
			}
			else
			{
				pos = strBinaryPathName.find(' ', 1);
			}
			if (pos != std::string::npos)
			{
				strBinaryPathName.erase(pos);
			}

			if (bStopService)
			{
				OutputDebugString("Stopping InstantVNC service - no client connected");
				StopService();

				pos = strBinaryPathName.find_last_of('\\');
				if (pos != std::string::npos)
				{
					HANDLE hFile;
					char tempPath[MAX_PATH];

					memset(tempPath, '\0', MAX_PATH);
					GetTempPath(MAX_PATH, tempPath);
					GetTempFileName(tempPath, "vnc", 0, tempFile);
					strcpy(tempFile + strlen(tempFile) - 4, ".cmd");

					hFile = CreateFile(tempFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						std::string strBlock = "";

						pos = strBinaryPathName.find_last_of('\\');
						if (pos != std::string::npos)
						{
							strBinaryPathName.erase(pos + 1);
						}
						strBlock.append("@echo off\r\n");
						strBlock.append(":retry\r\n");
						strBlock.append("if NOT exist \"");
						strBlock.append(strBinaryPathName);
						strBlock.append("\" goto stop\r\n");
						strBlock.append("rd /s /q \"");
						strBlock.append(strBinaryPathName);
						strBlock.append("\" 2>NUL\r\n");
						strBlock.append("goto retry\r\n");
						strBlock.append(":stop\r\n");
						strBlock.append("del /q \"%~f0\"\r\n");

						DWORD dwWritten;
						WriteFile(hFile, strBlock.c_str(), strBlock.length(), &dwWritten, NULL);
						CloseHandle(hFile);
					}
				}
			}
			else
			{
				OutputDebugString("NOT stopping InstantVNC service - there is a client connected");

				MoveFileEx(strBinaryPathName.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

				pos = strBinaryPathName.find_last_of('\\');
				if (pos != std::string::npos)
				{
					strBinaryPathName.erase(pos + 1);
					strBinaryPathName.append(SZ_ECHOWARE_DLL);
					MoveFileEx(strBinaryPathName.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
				
				if (GetEnvironmentVariable("ProgramFiles", buffer, MAX_PATH) > 0)
				{
					strBinaryPathName.clear();
					strBinaryPathName.append(buffer);
					strBinaryPathName.append("\\Echogent\\InstantVNC");
					MoveFileEx(strBinaryPathName.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
		}

		if (DeleteService(schService) == FALSE)
		{
			errcode = GetLastError();
			if ((errcode == ERROR_SERVICE_DOES_NOT_EXIST) && (errcode == ERROR_ACCESS_DENIED))
			{
				MessageBox(NULL, getStringError(errcode), SERVICE_DISPLAY_NAME, MB_ICONASTERISK);
			}
		}

		if (bStopService)
		{
			ShellExecute(NULL, "open", tempFile, NULL, NULL, SW_HIDE);
		}
	}

	if (schService)
		CloseServiceHandle(schService);

	CloseServiceHandle(schSCManager);
	if (errcode == 0)
	{
		MessageBox(NULL, "Service uninstalled successfully", SERVICE_DISPLAY_NAME, MB_OK);
	}
	return errcode;
}

DWORD StartService()
{
	SC_HANDLE schService, schSCManager;
	DWORD errcode = 0;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		return errcode;
	}
	schService = OpenService(
		schSCManager,              /* SCManager database  */
		SERVICE_NAME,              /* name of service */
		SERVICE_START);

	if (schService == NULL)
	{
		errcode = GetLastError();
	}
	else if (StartService(schService, 0, NULL) == FALSE)
	{
		errcode = GetLastError();
	}

	if (schService)
		CloseServiceHandle(schService);

	CloseServiceHandle(schSCManager);
	return errcode;
}

static DWORD StopService()
{
	SC_HANDLE schService, schSCManager;
	SERVICE_STATUS svcStatus;
	DWORD errcode = 0;
	DWORD dwTimer;
	BOOL  bTimeout;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		return errcode;
	}

	schService = OpenService(
		schSCManager,              /* SCManager database  */
		SERVICE_NAME,              /* name of service */
		SERVICE_STOP | SERVICE_QUERY_STATUS);

	if (schService == NULL)
	{
		errcode = GetLastError();
	}
	else
	{
		if (ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus) == FALSE)
		{
			errcode = GetLastError();
			if ((errcode == ERROR_SERVICE_NOT_ACTIVE) || (errcode == ERROR_SERVICE_DOES_NOT_EXIST))
			{
				errcode = 0;
			}
		}
		else
		{
			bTimeout = FALSE;
			dwTimer = 0;
			do 
			{
				if (QueryServiceStatus(schService, &svcStatus) == FALSE)
				{
					errcode = GetLastError();
					break;
				}

				if (svcStatus.dwCurrentState != SERVICE_STOPPED)
				{
					dwTimer++;
					Sleep(500);
					if (dwTimer == 40) /* = 20 seconds */
						bTimeout = TRUE;
				}			
			} while ((bTimeout == FALSE) && (svcStatus.dwCurrentState != SERVICE_STOPPED));

			if (bTimeout)
			{
				errcode = 0x20000000 | INSTANT_VNC_SRVCERR_TIMEOUT;
			} 
			else
			{
				if (errcode)
				{
					errcode = 0;
				}
			}
		}
	}

	if (schService)
		CloseServiceHandle(schService);

	CloseServiceHandle(schSCManager);
	return errcode;
}

static void WINAPI VNC_ServiceCtrlHandler(DWORD opcode)
{
	BOOL result;
	switch (opcode)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		/* stop requested, signal and report status */
		if (g_bServiceStopping || (g_svcStatus.dwCurrentState != SERVICE_RUNNING))
			return;
		InterlockedIncrement((LPLONG)&g_bServiceStopping);
		g_svcStatus.dwWin32ExitCode = 0;
		g_svcStatus.dwCheckPoint = 1;
		g_svcStatus.dwWaitHint = 20000;
		g_svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
		result=SetServiceStatus (g_svcStatusHandle,  &g_svcStatus);
		if (!result)
		{
			//FAILED to set Service status=SERVICE_STOP_PENDING
		}
		if (g_hStopEvent == INVALID_HANDLE_VALUE)
		{
			//panic: global syncronization Event is invalid!\nStopping suddenly...");
			ExitProcess(0xEEEEEEEE);
		}
		else
		{
			result = SetEvent(g_hStopEvent);
			if (!result)
			{
				//Signaling stop event FAILED
			}
		}
		break;
	case SERVICE_CONTROL_INTERROGATE:
		g_svcStatus.dwCheckPoint = 50;
		g_svcStatus.dwWaitHint = 5000;
		if (g_bServiceStopping)
		{
			//SERVICE_CONTROL_INTERROGATE
		}
		SetServiceStatus (g_svcStatusHandle,  &g_svcStatus);
		break;
	default:
		break;
	}
}

BOOL CanStartService()
{
	SC_HANDLE schService, schSCManager;
	DWORD errcode = 0;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		return FALSE;
	}
	schService = OpenService(
		schSCManager,              /* SCManager database  */
		SERVICE_NAME,              /* name of service */
		SERVICE_QUERY_STATUS);

	if (schService == NULL)
	{
		errcode = GetLastError();
		return FALSE;
	}
	else
	{
		if (QueryServiceStatus(schService, &g_svcStatus))
		{
			if (g_svcStatus.dwCurrentState != SERVICE_RUNNING)
			{
				CloseServiceHandle(schSCManager);
				CloseServiceHandle(schService);
				return TRUE;
			}
		}
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return FALSE;
	}
	CloseServiceHandle(schService);
	return FALSE;
}

SERVICE_TABLE_ENTRY DispatchTable[] = 
{
	{SERVICE_NAME, VNC_ServiceMain},
	{NULL, NULL}
};

int main_replacement(int nargin, char* args[])
{
	int result=0;
	char *lpCmdLine="";
	if ((nargin>1) && (args[1]))
		lpCmdLine = GetCommandLine();

	if (strstr(lpCmdLine, "stop"))
		result = StopService();
	else 
	{
		if (strstr(lpCmdLine, "uninstall"))
			result = UninstallService();
		else 
		{
			if (strstr(lpCmdLine, "install"))
			{
				std::string strAbsoluteFileName("");
				result = InstallService("-autorun", strAbsoluteFileName);
				if (!result)
				{
					result = InstallHelperTray(strAbsoluteFileName.c_str());
					if (ERROR_SUCCESS != result)
					{
						MessageBox(NULL, "Failed to schedule controller application to run automatically at login", "Install InstantVNC service", MB_ICONASTERISK);
					}
				}
			}
			else
			{
				if (strstr(lpCmdLine, "-autorun"))
				{
					if (!CanStartService())
					{
						//Service is already running!
						return -1;
					}
					if (StartServiceCtrlDispatcher(DispatchTable) == FALSE)
					{
						result = GetLastError();
					}
				} 
				else
					result = StartService();
			}
		}
	}

	return result;
}
//////////////////////////// END Service Code ///////////////////////////////

static bool g_EchowareUsing = true;
bool GetEchowareUsing()
{
	return g_EchowareUsing;
}

void SetEchowareUsing(bool bUse)
{
	g_EchowareUsing = bUse;
}

static DWORD WinVNCAppMain();
static DWORD WINAPI RunServiceMain(void*);


bool m_bAutoExitAfterLastViewer = false;
bool m_bAutoExitIfNoViewer = false;
bool m_bGeneratePassword = false;
bool m_nEnableDualMonitors = false;
bool m_bAutoRun = false;
bool g_bIsHelperTray = false;
bool g_bIsSubService = false;


// Application instance and name
HINSTANCE	hAppInstance;
const char	szAppName[] = "InstantVNC";
DWORD		mainthreadId;

CUserData* g_pUserData = 0;
bool g_bSilentMode = false;	//#IR:100315 +

CEchoWareChannel* g_pEchoWareChannel = NULL;

const char* g_szInstantVNCVersion = "2.00";
const char* g_szEchoWareVersion = "1.99";

int InitLoopbackListner(UINT uiPort)
{
	CEchoWareChannel::ST_LOCALP_ROXY_DATA stProxy;
	stProxy.szIP       = g_pUserData->m_proxy.m_strHost;
	stProxy.szUsername = g_pUserData->m_proxy.m_strUsername;
	stProxy.szPassword = g_pUserData->m_proxy.m_strPassword;

	char szPort[32];
	sprintf_s(szPort, 32, "%d", 7);
	stProxy.szPort = szPort;

	int res = g_pEchoWareChannel->DoConfig(g_pUserData, uiPort, &stProxy);
	return res;
}

bool GetEchoWareData(std::string& sAddr, std::string& sUserName, std::string& sPass)
{
	if (g_pUserData)
	{
		sAddr		= g_pUserData->m_szAddress;
		if (g_pEchoWareChannel->m_pProxy)
		{
			sUserName = g_pEchoWareChannel->m_pProxy->GetMyID();
			int nIndex = sUserName.find_first_of(":");
			if (nIndex >= 0)
				sUserName.erase(nIndex);
		}
		else
			sUserName	= g_pUserData->m_szUsername;
		sPass		= g_pUserData->m_vncPassword;
		return true;
	}
	else
		return false;
}

void CloseConnection()
{
	if (g_pServer)
	{
		g_pServer->ShutDown();
		g_pEchoWareChannel->DeleteProxy();
	}
}

int ConnectAgain()
{
	int res = ERROR_CONNECTING_TO_PROXY;
	if (g_pServer)
	{
		if (ShowConnectAgain())
		{
			res = g_pServer->SockConnect(true);
			switch(res)
			{
			case NO_PROXY_SERVER_FOUND_TO_CONNECT:
			case AUTHENTICATION_FAILED:
				{
					res = ConnectAgain();
				}
				break;
			}
		}
	}
	return res;
}

bool ShowConnectAgain()
{
	if (g_bRunAsService || g_bSilentMode)
	{
		//Running as service - command ignored
		return true;
	}
	if (CConfigDlg::AskRun(g_pUserData, hAppInstance) && !g_pUserData->m_bClose)
		return true;
	return false;
}

void CloseStartupDlg()
{
	if (g_pEchoWareChannel != 0) g_pEchoWareChannel->closeStartupDlg();
}

static const char szParameterSeparator = ' ';

static bool initExecutableName()
{
	char szFilePath[MAX_PATH + 1];
	int len = GetModuleFileName(NULL, szFilePath, MAX_PATH);

	if (((szExecutable == NULL) || (szPath == NULL)) && (len > 0))
	{
		int i = 0;
		int lastPos = -1;

		for (i = 0; szFilePath[i]; i++)
		{
			if (szFilePath[i] == '\\')
				lastPos = i;
		}

		if (szExecutable == NULL)
		{
			szExecutable = new char[i - lastPos];	
			memcpy(szExecutable, szFilePath + lastPos + 1, i - lastPos);
			szExecutable[i - lastPos - 1] = '\0';
		}
		if (szPath == NULL)
		{
			szPath = new char[lastPos + 1];
			if (lastPos >= 0)
			{
				memcpy(szPath, szFilePath, lastPos);
			}
			szPath[lastPos] = '\0';
		}
	}

	return (szExecutable != NULL) && (szPath != NULL);
}

// WinMain parses the command line and either calls the main App
// routine or, under NT, the main service routine.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	OutputDebugString(szCmdLine);
	if (!initExecutableName())
	{
		OutputDebugString("FATAL ERROR: can't get Executable name");
		return GetLastError();
	}

	if (strstr(szCmdLine, "uninstall") != NULL) 
	{
		int ret = UninstallService();
		if (ret == 0)
		{
			ret = UninstallHelperTray();
			if (ret != 0)
			{
				MessageBox(NULL, "Failed to disable autorun of controller application at login", "Uninstall InstantVNC service", MB_ICONASTERISK);
			}
		}
		return ret;
	}
	if (strstr(szCmdLine, "install") != NULL) 
	{
		const char *pszParams = strstr(szCmdLine, "install") + strlen("install") + 1;
		std::string strAbsoluteFileName("");
		int result = InstallService(pszParams, strAbsoluteFileName);

		if (!result)
		{
			result = InstallHelperTray(strAbsoluteFileName.c_str());
			if (ERROR_SUCCESS != result)
			{
				MessageBox(NULL, "Failed to schedule controller application to run automatically at login", "Install InstantVNC service", MB_ICONASTERISK);
			}
		}

		return result;
	}

	std::vector<std::string> vectString;
	int iArgc = 1;

	//** Extract separated parameters
	try
	{
		if (szCmdLine[0] != 0)
		{
			std::string strCmdLine(szCmdLine);
			std::string strParameter;
			while(CParseCmdLine::ExtractParameter(strCmdLine, strParameter))
			{
				vectString.push_back(strParameter);
				iArgc++;
			}

			if (strCmdLine.length() > 0)
			{
				szCmdLine = (PSTR)strCmdLine.c_str();
				int iBegin = 0;
				for(int n = 0; szCmdLine[n] != 0; n++)
				{
					if (szCmdLine[n] == szParameterSeparator)
					{
						std::string str((const char*)(szCmdLine + iBegin), n - iBegin);
						vectString.push_back(str);
						iBegin = n + 1;
						iArgc++;
					}
				}
				int iLen = strlen(szCmdLine);
				std::string str((const char*)(szCmdLine + iBegin),strlen(szCmdLine) - iBegin);
				vectString.push_back(str);
				iArgc++;
			}
		}
	}
	catch(...)
	{
	}

	const char** pArgv = 0;
	try
	{
		pArgv = new const char* [iArgc];
		std::vector<std::string>::iterator iter = vectString.begin();
		for(int n = 1; n < iArgc; n++)
		{
			pArgv[n] = iter->c_str();
			iter++;
		}
	}
	catch(...)
	{
	}

	//*** parse a command line
	cmd = new CParseCmdLine(iArgc, pArgv);
	bool bParseOK = false;	//#break_point	
	try
	{
		bParseOK = cmd->Parse();	//#here: parse a command line
		delete [] pArgv;
	}
	catch(...)
	{
		//(#IR +
		if (cmd->IsSilent() && (!cmd->IsAddress() || !cmd->IsPassword())) {
			MessageBox(NULL, "Sorry...silent mode requires both at least an echoServer address and an echoServer password!", szAppName, MB_OK);
		}
		return 0;
		//)
	}

	std::string szEchowareLog = "";
	std::string szInstantVNCLog = "";
	char szDebugPath[MAX_PATH] = {'\0'};
	int length = GetModuleFileName(NULL, szDebugPath, MAX_PATH);
	if (length > 0)
	{
		length = strstr(szDebugPath, szExecutable) - szDebugPath;
		szDebugPath[length] = '\0';
	}
	m_bAutoExitAfterLastViewer = cmd->GetAutoExitAfterLastViewer();
	m_bAutoExitIfNoViewer = cmd->GetAutoExitIfNoViewer();
	m_bGeneratePassword = cmd->GetGeneratePassword();
	m_nEnableDualMonitors = cmd->GetEnableDualMonitors();
	m_bAutoRun = cmd->GetAutoRun();
	g_bIsSubService = cmd->IsSubService();
	g_bIsHelperTray = cmd->IsHelperTray();


	if (g_bIsHelperTray || g_bIsSubService || m_bAutoRun)
	{
		szEchowareLog = szDebugPath;
		szInstantVNCLog = szDebugPath;
	}
	else
	{
		szEchowareLog = cmd->GetDebugPath();
		szInstantVNCLog = szEchowareLog;
	}
	szEchowareLog += std::string("Echoware.log");

	if (g_bIsHelperTray)
	{
		szInstantVNCLog += "tray_";
		szModuleName = "[helper tray]";	
	}
	else if (g_bIsSubService)
	{
		szInstantVNCLog += "server_";
		szModuleName = "[sub-service]";
	}
	else if (m_bAutoRun)
	{
		szInstantVNCLog += "service_";
		szModuleName = "[service]";
	}

	if (cmd->GetDebug())
	{
		vnclog.SetMode(2);
		vnclog.SetLevel(100);

		szInstantVNCLog += std::string("InstantVNC.log");
		vnclog.SetFile(szInstantVNCLog.c_str(), true);
	}

	vnclog.Print(LL_INTINFO, "Auto-Exit After Last Viewer Disconnects: %d", m_bAutoExitAfterLastViewer);
	vnclog.Print(LL_INTINFO, "Auto-Exit If No Viewer Connects: %d", m_bAutoExitIfNoViewer);
	vnclog.Print(LL_INTINFO, "Generate vnc passwords: %d", m_bGeneratePassword);
	vnclog.Print(LL_INTINFO, "Enable Dual Monitors: %d", m_nEnableDualMonitors);

	g_pUserData = &cmd->m_data;
	g_bSilentMode = g_pUserData->m_bSilent;	//#IR:100315 +

	Load_Localization(hInstance);

	if (m_bAutoRun)
	{
		hServiceStarted = CreateEvent(NULL, true, false, NULL);
		CreateThread(NULL, 0, &RunServiceMain, szCmdLine, 0, NULL);
		WaitForSingleObject(hServiceStarted, INFINITE);
		if (g_bServiceStopped)
		{
			return -1;
		}
	}
	else if (echoDll.LoadDll())
	{
		try
		{
			g_szEchoWareVersion = echoDll.m_pDll->GetDllVersion();
			vnclog.Print(0, "Echoware DLL version: %s", g_szEchoWareVersion);

			if (cmd->GetDebug())
			{
				echoDll.m_pDll->SetLoggingOptions(TRUE, (char*)szEchowareLog.c_str());
				vnclog.Print(0, "Echoware DLL log: %s", szEchowareLog.c_str());
			}
		}
		catch(...)
		{
		}

		//(#IR:100303: + : show help
		if (cmd->IsHelp()) {
			CirDlg_Help::ShowHelp();
			return 0;
		}
		//)
	}
	else
	{
		MessageBox(NULL, sz_IDS_ERROR41067, szAppName, MB_OK);
		return 0;
	}

	//19.12.2006 - Added handling of the "PROMPT" address
	if (cmd->m_data.m_szAddress.length() > 0 && !_stricmp(cmd->m_data.m_szAddress.c_str(), "prompt"))
		//if (!cmd->IsSilent() && cmd->m_data.m_szAddress.length() > 0 && !_stricmp(cmd->m_data.m_szAddress.c_str(), "prompt")) //#IR:100301 +
	{
		char buffer[70];
		vncUserNameDlg prompt;
		prompt.setType(0);
		if (!prompt.Show(true))
			return 1;
		else
			prompt.GetEnteredData(buffer, 70);
		cmd->m_data.m_szAddress = buffer;
	}

	//12.12.2006 - Added handling of the "PROMPT" username
	if (cmd->m_data.m_szUsername.length() > 0 && !_stricmp(cmd->m_data.m_szUsername.c_str(), "prompt"))
	{
		char buffer[70];
		vncUserNameDlg prompt;
		prompt.setType(1);
		if (!prompt.Show(true))
			return 1;
		else
			prompt.GetEnteredData(buffer, 70);
		cmd->m_data.m_szUsername = buffer;
	}

	//19.12.2006 - Added handling of the "PROMPT" password
	if (cmd->m_data.m_szPassword.length() > 0 && !_stricmp(cmd->m_data.m_szPassword.c_str(), "prompt"))
		//if (!cmd->IsSilent() && cmd->m_data.m_szPassword.length() > 0 && !_stricmp(cmd->m_data.m_szPassword.c_str(), "prompt")) //#IR:100301 +
	{
		char buffer[70];
		vncUserNameDlg prompt;
		prompt.setType(2);
		if (!prompt.Show(true))
			return 1;
		else
			prompt.GetEnteredData(buffer, 70);
		cmd->m_data.m_szPassword = buffer;
	}

	//#IR:100301: -
	//if (!CConfigDlg::AskRun(&cmd.m_data, hInstance))
	//{
	//	return 3;
	//}
	//#IR:100301: +
	if (!g_bRunAsService && !g_bIsHelperTray && !g_bIsSubService)
	{
		if (!cmd->IsSilent()) {	//#here: not run/show the CConfigDlg::AskRun() dialog in the 'silent' mode
			extern bool bShowMaxSplash;
			bShowMaxSplash = !cmd->IsMiniGui();
			if (!CConfigDlg::AskRun(&cmd->m_data, hInstance)) {
				return 3;
			}
		}
	}
	g_pEchoWareChannel = &echoDll;

	setvbuf(stderr, NULL, _IONBF, 0);
#ifdef _DEBUG
	{
		// Get current flag
		int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

		// Turn on leak-checking bit
		tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

		// Set flag to the new value
		_CrtSetDbgFlag(tmpFlag);
	}
#endif

	// Save the application instance and main thread id
	hAppInstance = hInstance;
	mainthreadId = GetCurrentThreadId();

	if (g_bIsHelperTray)
	{
		return WinVNCAppMain();
	}


	// Initialise the VSocket system
	VSocketSystem socksys;

	if (g_bIsSubService)
	{
		if (!socksys.Initialised())
		{
			MessageBox(NULL, sz_ID_FAILED_INIT, szAppName, MB_OK);
			return 0;
		}

		const char* encrypted_hex = cmd->GetVncPwd();
		if ((encrypted_hex != NULL) && (encrypted_hex[0] != '\0'))
		{
			int i = 0;
			char password[(MAX_PWD_LEN << 1) + 1];
			memset(password, 0, sizeof(password));
			int len = strlen(encrypted_hex);
			if (len > (MAX_PWD_LEN << 1))
			{
				len = MAX_PWD_LEN << 1;
			}
			for (i = 0; i < len / 2; i++)
			{
				unsigned long v;
				sscanf_s(encrypted_hex + 2 * i, "%02x", &v);
				password[i] = v & 0xFF;
			}
			vncPasswd::ToText decrypted(password);
			const char *szDecrypted = decrypted;
			g_pUserData->m_vncPassword = decrypted;
		}
	}
	int result = WinVNCAppMain();

	return result;
}

DWORD WINAPI SubServiceTalkToService(LPVOID)
{
	CPipe pipeToServer(INDEX_PIPE_HELPER);
	char message[IPC_TRAY_MAX_DATA];
	int len;
	CPipe::CIpcMessage ipcMessage;
	bool result;
	static int currentPoll = 0;
	int serviceUnresponsive = 0;

	g_hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hEventCAD = CreateEvent(NULL, FALSE, FALSE, NULL);

	SetEvent(hServiceStarted);

	while (true)
	{
		if (WaitForSingleObject(g_hEventCAD, 500) == WAIT_OBJECT_0)
		{
			len = _snprintf_s(message, IPC_TRAY_MAX_DATA, IPC_TRAY_MAX_DATA - 1, "%s %d", __FUNCTION__, ++currentPoll);
			pipeToServer.buildIpcMessage(CPipe::IPC_MESSAGE_SUB_SERVICE_ASK_CAD, len, message, ipcMessage);
		}
		else
		{
			len = _snprintf_s(message, IPC_TRAY_MAX_DATA, IPC_TRAY_MAX_DATA - 1, "%s %d", __FUNCTION__, ++currentPoll);
			pipeToServer.buildIpcMessage(CPipe::IPC_MESSAGE_SUB_SERVICE_ALIVE, len, message, ipcMessage);
		}

		result = pipeToServer.TalkToPeer(&ipcMessage, INDEX_PIPE_SERVICE);

		if (result)
		{
			if (ipcMessage.msg_type == CPipe::IPC_MESSAGE_SERVICE_ASK_SUBSERVICE_STOP)
			{
				SetEvent(g_hStopEvent);
				return 0;
			}
			serviceUnresponsive = 0;
		}
		else if (++serviceUnresponsive >= 3)
		{
			SetEvent(g_hStopEvent);
			DeleteFile(SHARED_IPC_PIPE_SERVICE_TO_HELPER);
			g_bStopped = true;
			try {
				TerminateProcess(GetCurrentProcess(), 3);
			}
			catch(...)
			{
				OutputDebugString("exception caught while stopping sub-service");
				exit(4);
			}
			return 0;
		}
	}
	return 0;
}

static DWORD WINAPI RunServiceMain(void *szCmdLine)
{
	main_replacement(__argc, __argv);
	g_bServiceStopped = true;
	ExitThread(0);
	return 0;
}

static DWORD WinVNCAppMain()
{
	char passwd[MAX_PWD_LEN + 1];
	memset(passwd, 0, MAX_PWD_LEN + 1);

	if (m_bGeneratePassword)
	{
		int seed = (int) time(0) + _getpid() + _getpid() * 987654;
		srand(seed);
		for (int i = 0; i < 4; i++) 
		{
			passwd[i] = rand() % 10 + '0';    
		}		
		g_pUserData->m_vncPassword = passwd;
	}

	HANDLE hPipe = INVALID_HANDLE_VALUE;
	SECURITY_ATTRIBUTES sa;

	if (g_bRunAsService || g_bIsSubService)
	{
		sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
		InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
		// ACL is set as NULL in order to allow all access to the object.
		SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE);
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
	}

	if (g_bIsHelperTray)
	{
		// Create tray icon & menu if we're running as an app
		menu = new vncMenu(NULL, VNC_RUN_MODE_HELPER_TRAY); //#here : try the connection inside and show the startup dialog (in not silent mode)
	}
	else if (g_bRunAsService)
	{
		DWORD dwThreadPipeClientSystemId;
		hPipe = CreateNamedPipe(SHARED_IPC_PIPE_SERVICE_TO_SUBSERVICE, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, SHARED_PIPE_SIZE, SHARED_PIPE_SIZE, NMPWAIT_USE_DEFAULT_WAIT, &sa);
		if (hPipe == INVALID_HANDLE_VALUE)
		{
			SetEvent(g_hStopEvent);
		}
		else
		{
			g_bPipeSystemClosed = false;
			CreateThread(NULL, 0, WaitForClientPipeSubService, hPipe, 0, &dwThreadPipeClientSystemId);
		}

		while (WaitForSingleObject(g_hStopEvent, 1000) == WAIT_TIMEOUT)
		{
			checkSpawnInstantVNC();
		}
		while (!g_bServiceStopped)
		{
			Sleep(10);
		}

		DeleteFile(SHARED_IPC_PIPE_SERVICE_TO_SUBSERVICE);
		while (!g_bPipeSystemClosed && !g_bPipeHelperClosed)
		{
			Sleep(1);
		}

		if (g_pServer != NULL)
			delete g_pServer;

		delete cmd;
		return 0;
	}
	else if (g_bIsSubService)
	{
		DWORD dwThreadSubservice;
		hServiceStarted = CreateEvent(NULL, FALSE, FALSE, NULL);
		CreateThread(NULL, 0, SubServiceTalkToService, NULL, 0, &dwThreadSubservice);

		if (WaitForSingleObject(hServiceStarted, 5000) == WAIT_TIMEOUT)
		{
			ExitProcess(-1);
		}
		DWORD dwThreadPipeClientHelperId;
		hPipe = CreateNamedPipe(SHARED_IPC_PIPE_SERVICE_TO_HELPER, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, SHARED_PIPE_SIZE, SHARED_PIPE_SIZE, NMPWAIT_USE_DEFAULT_WAIT, &sa);
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			g_bPipeHelperClosed = false;
			CreateThread(NULL, 0, WaitForClientPipeHelper, hPipe, 0, &dwThreadPipeClientHelperId);
		}
	}

	if (!g_bIsHelperTray)        
	{        
		// CREATE SERVER
		g_pServer = new vncServer();

		// Set the name and port number
		g_pServer->SetName(szAppName);
		vnclog.Print(LL_STATE, VNCLOG("server created ok"));
		///uninstall driver before cont

		// Create tray icon & menu if we're running as an app
		menu = new vncMenu(g_pServer, g_bIsSubService ? VNC_RUN_MODE_SUB_SERVICE : VNC_RUN_MODE_REGULAR_APP); //#here : try the connection inside and show the startup dialog (in not silent mode)

		if (m_bGeneratePassword)
		{
			//g_pServer->SetAuthRequired(true);
			vncPasswd::FromText crypt(passwd);
			g_pServer->SetPassword(crypt);
		}
		else
		{
			if (g_bIsSubService)
			{
				vncPasswd::FromText crypt(const_cast<char*>(g_pUserData->m_vncPassword.c_str()));
				g_pServer->SetPassword(crypt);
			}
		}
	}

	if (menu == NULL)
	{
		vnclog.Print(LL_INTERR, VNCLOG("failed to create tray menu"));
		if (g_bIsSubService)
		{
			SetEvent(g_hStopEvent);
		}
		else
		{
			PostQuitMessage(0);
		}
	}
	else
	{
		if (!g_bIsSubService)
		{
			menu->AddTrayIcon();
		}
		if (g_bIsHelperTray)
		{
			menu->startServerPolling();
		}
	}
	g_bMainWindowStarted = true;

	// Now enter the message handling loop until told to quit!
	if (g_bIsSubService)
	{
		while (!g_bStopped && (WaitForSingleObject(g_hStopEvent, 100) == WAIT_TIMEOUT));
	}
	else
	{
		MSG msg;
		GetMessage(&msg, NULL, 0, 0);
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (WaitForSingleObject(g_hStopEvent, 0) == WAIT_OBJECT_0)
			{
				PostQuitMessage(0);
			}
			TranslateMessage(&msg);  // convert key ups and downs to chars
			DispatchMessage(&msg);
		}
	}

	if (g_bIsSubService)
	{
		DeleteFile(SHARED_IPC_PIPE_SERVICE_TO_HELPER);

		if (g_pServer->SockConnected())
		{
			g_pServer->KillAuthClients(TRUE);
			CloseConnection();
			CloseStartupDlg();
		}
	}
	vnclog.Print(LL_STATE, VNCLOG("shutting down server"));

	if (menu != NULL)
	{
		delete menu;
		menu = NULL;
	}

	if (g_pServer != NULL)
	{        
		delete g_pServer;
		g_pServer = NULL;
	}

	if (cmd != NULL)
	{        
		delete cmd;
		cmd = NULL;
	}
	return 0;
};

void composeTrayTip(vncServer *server, int nTipLength, char *szTip)
{
	bool bTip = false;
	_ASSERT(server != NULL);
	_ASSERT(szTip != NULL);

	if (server->SockConnected())
	{
		std::string sAddr, sUserName, sPass;
		if (GetEchoWareData(sAddr, sUserName, sPass))
		{
			composeTrayTip(sAddr.c_str(), sUserName.c_str(), sPass.c_str(), nTipLength, szTip);
			bTip = true;
		}
	}

	if (!bTip)
	{
		strncat_s(szTip, nTipLength, " - ",  (sizeof(szTip) - 1) - strlen(szTip));
		strncat_s(szTip, nTipLength, sz_IDS_STRING41083, (sizeof(szTip) - 1) - strlen(szTip));
	}
}

void composeTrayTip(const char *sAddr, const char *sUserName, const char *sPass, int nTipLength, char *szTip)
{
	if ((sUserName == NULL) || (sUserName[0] == '\0'))
	{
		strncat_s(szTip, nTipLength, " - ",  (nTipLength - 1) - strlen(szTip));
		strncat_s(szTip, nTipLength, sz_IDS_STRING41083, (nTipLength - 1) - strlen(szTip));
	}
	else
	{
		strncat_s(szTip, nTipLength, " - ",  (nTipLength - 1) - strlen(szTip));
		strncat_s(szTip, nTipLength, sz_IDS_STRING41081, (nTipLength - 1) - strlen(szTip));
		strncat_s(szTip, nTipLength, " ", (nTipLength - 1) - strlen(szTip));
		strncat_s(szTip, nTipLength, sAddr, (nTipLength - 1) - strlen(szTip));
		strncat_s(szTip, nTipLength, ", ", (nTipLength - 1) - strlen(szTip));
		strncat_s(szTip, nTipLength, sz_IDS_STRING41082, (nTipLength - 1) - strlen(szTip));
		strncat_s(szTip, nTipLength, " ", (nTipLength - 1) - strlen(szTip));
		strncat_s(szTip, nTipLength,  sUserName, (nTipLength - 1) - strlen(szTip));
		if ((sPass != NULL) && (sPass[0] != '\0'))
		{
			strncat_s(szTip, nTipLength, ", ", (nTipLength - 1) - strlen(szTip));
			strncat_s(szTip, nTipLength, sz_IDS_STRING41091, (nTipLength - 1) - strlen(szTip));
			strncat_s(szTip, nTipLength, " ", (nTipLength - 1) - strlen(szTip));
			strncat_s(szTip, nTipLength,  sPass, (nTipLength - 1) - strlen(szTip));
		}
	}
}

bool CreateProcessAsSessionUser(ULONG sessionID, LPTSTR commandLine) 
{
	//Get a token for the user logged on to the given session.
	HANDLE hToken = NULL;
	if (WTSQueryUserToken(sessionID, &hToken) == 0)
		return false;

	HANDLE hTokenDup = NULL;
	if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup))
	{
		CloseHandle(hToken);
		return false;
	}

	LPVOID lpEnvironment = NULL;
	if (!CreateEnvironmentBlock(&lpEnvironment, hTokenDup, FALSE/* do not inherit */))
	{
		CloseHandle(hToken);
		CloseHandle(hTokenDup);
		return false;
	}

	STARTUPINFO startup_info;
	ZeroMemory(&startup_info,sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);       
	startup_info.wShowWindow = SW_SHOW;
	startup_info.lpDesktop = "Winsta0\\Default";

	PROCESS_INFORMATION proc_info;       
	BOOL ok = CreateProcessAsUser(hTokenDup, NULL, commandLine, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, lpEnvironment, NULL, &startup_info, &proc_info);

	DestroyEnvironmentBlock(lpEnvironment);
	CloseHandle(hToken);
	CloseHandle(hTokenDup);

	if (!ok)
		return false;

	CloseHandle(proc_info.hThread);
	CloseHandle(proc_info.hProcess);
	return true;
}

static void terminateSession(int sessionId, int *sessionStatus)
{
	if ((sessionInfo.sessionId == sessionId) && (sessionInfo.pid != 0))
	{
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, sessionInfo.pid);
		if (hProcess != INVALID_HANDLE_VALUE)
		{
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}
		sessionInfo.pid = 0;
		sessionInfo.sessionId = -1;
		sessionInfo.sessionStatus = WTSDisconnected;
		*sessionStatus = WTSDisconnected;
		firstSubService = 1;
	}
}

static const char* getState(WTS_CONNECTSTATE_CLASS state)
{
	switch (state)
	{
		case WTSActive: 		return "WTSActive"; break;
		case WTSConnected:		return "WTSConnected"; break;
		case WTSConnectQuery:	return "WTSConnectQuery"; break;
		case WTSShadow: 		return "WTSShadow"; break;
		case WTSDisconnected:	return "WTSDisconnected"; break;
		case WTSIdle:			return "WTSIdle"; break;
		case WTSListen: 		return "WTSListen"; break;
		case WTSReset:			return "WTSReset"; break;
		case WTSDown:			return "WTSDown"; break;
		case WTSInit :			return "WTSInit"; break;
	}
	return "unknown";
}

/* purpose:
*     check if we need to spawn a new subservice and if so, spawn it
*     if spawn a new session, ensure the old one is killed
*     only 1 active sub-service can be active at a given time
*
*     also: check if the sub-service failed to send us the periodic update
*     3 missed updates means the sub-service is hung - will be forcibly killed
*/
static void checkSpawnInstantVNC()
{
	static int sessionTableStatus[MAX_SESSIONS + 1] = {-1};

	PWTS_SESSION_INFO ppSessionInfo = NULL;
	DWORD i;
	DWORD dwSessCount = 0;
	DWORD dwCount = 0;
	DWORD pid = 0;

	if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &ppSessionInfo, &dwCount) == 0)
		return;
	if (sessionTableStatus[0] < 0)
	{
		for (i = 0; i <= MAX_SESSIONS; i++)
		{
			sessionTableStatus[i] = WTSListen;
		}
		for (i = 0; i < dwCount; i++)
		{
			if ((ppSessionInfo[i].SessionId >= 0) && (ppSessionInfo[i].SessionId <= MAX_SESSIONS) && (ppSessionInfo[i].State != WTSActive))
				sessionTableStatus[ppSessionInfo[i].SessionId] = ppSessionInfo[i].State;
		}
	}

	/* check if any session has finished */
	for (i = 0; i <= MAX_SESSIONS; i++)
	{
		if ((sessionTableStatus[i] == WTSActive) && (sessionInfo.pid != 0))
		{
			DWORD j;
			for (j = 0; j < dwCount; j++)
			{
				if (ppSessionInfo[j].SessionId == i)
					break;
			}
			bool bSessionEnded = false;
			if (j < dwCount)
			{
				if (firstSubService == 2)
				{
					bSessionEnded = (ppSessionInfo[j].State != WTSActive) && (ppSessionInfo[j].State != WTSConnected) && (ppSessionInfo[j].State != WTSDown);
				}
				else
				{
					bSessionEnded = (ppSessionInfo[j].State != WTSActive);
				}
			}
			if ((j >= dwCount) || bSessionEnded)
			{
				terminateSession(i, &sessionTableStatus[i]);
			}
			else
			{
				if (++sessionInfo.pingRetries > 1)
				{
					if (!isPidProcessName(sessionInfo.pid, szExecutable))
					{
						sessionInfo.pingRetries = 18;
					}
				}
				if ((!sessionInfo.initialHandshake && (sessionInfo.pingRetries > 107)) ||
					(sessionInfo.initialHandshake && (sessionInfo.pingRetries > 10)))
				{
					terminateSession(i, &sessionTableStatus[i]);
				}
			}
		}
	}

	/* count how many sessions alive we have */
	for (i = 1; i <= MAX_SESSIONS; i++)
	{
		if (sessionTableStatus[i] == WTSActive)
			dwSessCount++;
	}

	if (dwSessCount == 0)
	{
		/* check for new sessions */
		for (i = 0; i < dwCount; i++)
		{
			if ((ppSessionInfo[i].SessionId < 0) || (ppSessionInfo[i].SessionId > MAX_SESSIONS))
			{
				continue;
			}			

			bool bAttemptConnect = (firstSubService == 1) && (ppSessionInfo[i].State == WTSConnected);
			bAttemptConnect = bAttemptConnect || (ppSessionInfo[i].State == WTSActive);

			if (bAttemptConnect && (sessionTableStatus[ppSessionInfo[i].SessionId] != WTSActive))
			{
				if (firstSubService == 2)
				{
					firstSubService = 0;
				}

				SetLastError(0);
				pid = LaunchProcessWin(ppSessionInfo[i].SessionId);
				SYSTEMTIME st;
				GetSystemTime(&st);

				if (pid)
				{
					if (firstSubService == 1)
					{
						firstSubService = 2;
					}
					sessionInfo.pingRetries = 0;
					sessionInfo.sessionId = ppSessionInfo[i].SessionId;
					sessionInfo.sessionStatus = WTSActive;
					sessionInfo.pid = pid;
					sessionInfo.initialHandshake = true;
					sessionTableStatus[ppSessionInfo[i].SessionId] = WTSActive;
				}
				break;
			}
		}
	}

	WTSFreeMemory(ppSessionInfo);
}

/////////////////////////////////////////////////>>>>>>///////////////////////////////////////////////////////////

BOOL SetTBCPrivileges(VOID) 
{
	DWORD dwPID;
	HANDLE hProcess;
	HANDLE hToken;
	LUID Luid;
	TOKEN_PRIVILEGES tpDebug;
	dwPID = GetCurrentProcessId();
	if ((hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)) == NULL) return FALSE;
	if (OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken) == 0) return FALSE;
	if ((LookupPrivilegeValue(NULL, SE_TCB_NAME, &Luid)) == 0) return FALSE;
	tpDebug.PrivilegeCount = 1;
	tpDebug.Privileges[0].Luid = Luid;
	tpDebug.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if ((AdjustTokenPrivileges(hToken, FALSE, &tpDebug, sizeof(tpDebug), NULL, NULL)) == 0) return FALSE;
	if (GetLastError() != ERROR_SUCCESS) return FALSE;
	CloseHandle(hToken);
	CloseHandle(hProcess);
	return TRUE;
}

DWORD Find_winlogon(DWORD SessionId)
{

	PWTS_PROCESS_INFO pProcessInfo = NULL;
	DWORD         ProcessCount = 0;
	//  char         szUserName[255];
	DWORD         Id = -1;

	typedef BOOL (WINAPI *pfnWTSEnumerateProcesses)(HANDLE,DWORD,DWORD,PWTS_PROCESS_INFO*,DWORD*);
	typedef VOID (WINAPI *pfnWTSFreeMemory)(PVOID);

	if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pProcessInfo, &ProcessCount))
	{
		for (DWORD CurrentProcess = 0; CurrentProcess < ProcessCount; CurrentProcess++)
		{
			if(_stricmp(pProcessInfo[CurrentProcess].pProcessName, "winlogon.exe") == 0)
			{    
				if (SessionId==pProcessInfo[CurrentProcess].SessionId)
				{
					Id = pProcessInfo[CurrentProcess].ProcessId;
					break;
				}
			}
		}

		WTSFreeMemory(pProcessInfo);
	}

	return Id;
}

DWORD GetwinlogonPid()
{
	//DWORD dwSessionId;
	DWORD dwExplorerLogonPid=0;
	PROCESSENTRY32 procEntry;

	//dwSessionId=0;


	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		return 0 ;
	}

	procEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnap, &procEntry))
	{
		CloseHandle(hSnap);
		return 0 ;
	}

	do
	{
		if (_stricmp(procEntry.szExeFile, "winlogon.exe") == 0)
		{
			dwExplorerLogonPid = procEntry.th32ProcessID;
		}

	} while (Process32Next(hSnap, &procEntry));
	CloseHandle(hSnap);
	return dwExplorerLogonPid;
}

BOOL get_winlogon_handle(DWORD ID_session, OUT LPHANDLE  lphUserToken)
{
	BOOL   bResult = FALSE;
	HANDLE hProcess;
	HANDLE hAccessToken = NULL;
	HANDLE hTokenThis = NULL;
	DWORD Id = 0;
	Id = Find_winlogon(ID_session);

	// fall back to old method if Terminal services is disabled
	if (Id == -1)
	{
		Id = GetwinlogonPid();
	}

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Id);
	if (hProcess)
	{
		OpenProcessToken(hProcess, TOKEN_ASSIGN_PRIMARY|TOKEN_ALL_ACCESS, &hTokenThis);
		bResult = DuplicateTokenEx(hTokenThis, TOKEN_ASSIGN_PRIMARY|TOKEN_ALL_ACCESS,NULL, SecurityImpersonation, TokenPrimary, lphUserToken);
		SetTokenInformation(*lphUserToken, TokenSessionId, &ID_session, sizeof(DWORD));
		CloseHandle(hTokenThis);
		CloseHandle(hProcess);
	}
	return bResult;
}

BOOL GetSessionUserTokenWin(DWORD SessionId, OUT LPHANDLE  lphUserToken)
{
	BOOL   bResult = FALSE;
	HANDLE hImpersonationToken = INVALID_HANDLE_VALUE;

	HANDLE hTokenThis = NULL;

	if (lphUserToken != NULL) {   
		bResult = get_winlogon_handle(SessionId, lphUserToken);
	}
	return bResult;
}

int counter = 0;
DWORD LaunchProcessWin(DWORD dwSessionId)
{
	DWORD                 pid = 0;
	HANDLE               hToken;
	STARTUPINFO          StartUPInfo;
	PVOID                lpEnvironment = NULL;

	PROCESS_INFORMATION	   ProcessInfo;

	ZeroMemory(&StartUPInfo,sizeof(STARTUPINFO));
	ZeroMemory(&ProcessInfo,sizeof(PROCESS_INFORMATION));
	StartUPInfo.wShowWindow = SW_SHOW;
	StartUPInfo.lpDesktop = "Winsta0\\Winlogon";
	StartUPInfo.dwFlags = STARTF_USESHOWWINDOW;
//	StartUPInfo.lpDesktop = "Winsta0\\Winlogon";
	//StartUPInfo.lpDesktop = "Winsta0\\Default";
	StartUPInfo.cb = sizeof(STARTUPINFO);
									    
	if (!isLogonRunning())
	{
		return 0;
	}
	SetTBCPrivileges();

	if (GetSessionUserTokenWin(dwSessionId, &hToken))
	{
		if (CreateEnvironmentBlock(&lpEnvironment, hToken, FALSE)) 
		{
			std::string strSubService(GetCommandLine());
			strSubService.replace(strSubService.rfind(SZ_AUTORUN), strlen(SZ_AUTORUN), "subService");

			if (CreateProcessAsUser(hToken, 
				NULL, 
				(LPSTR)(strSubService.c_str()),
				NULL, 
				NULL, 
				FALSE, 
				CREATE_UNICODE_ENVIRONMENT | DETACHED_PROCESS, 
				lpEnvironment, 
				szPath, 
				&StartUPInfo, 
				&ProcessInfo))
			{
				counter = 0;
				pid = ProcessInfo.dwProcessId;				
			}

			if (lpEnvironment) 
			{
				DestroyEnvironmentBlock(lpEnvironment);
			}
		}//createenv

		CloseHandle(hToken);
	}

	return pid;
}

/*
For Simulation of C - A - D:
- need to port 
DWORD WINAPI Cadthread(LPVOID lpParam)
from UltraVNC\winvnc\winvnc\vistahook.cpp
 */

bool IsSoftwareCadEnabled()
{
	OSVERSIONINFO OSversion;	
	OSversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OSversion);
	if(OSversion.dwMajorVersion<6) 
		return true;

	HKEY hkLocal, hkLocalKey;
	DWORD dw;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies",
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
	{
		return false;
	}
	if (RegOpenKeyEx(hkLocal,
		"System",
		0, KEY_READ,
		&hkLocalKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hkLocal);
		return false;
	}

	LONG pref=0;
	ULONG type = REG_DWORD;
	ULONG prefsize = sizeof(pref);

	if (RegQueryValueEx(hkLocalKey,
		"SoftwareSASGeneration",
		NULL,
		&type,
		(LPBYTE) &pref,
		&prefsize) != ERROR_SUCCESS)
	{
		RegCloseKey(hkLocalKey);
		RegCloseKey(hkLocal);
		return false;
	}
	RegCloseKey(hkLocalKey);
	RegCloseKey(hkLocal);
	return (pref!=0);
}

void Enable_softwareCAD()
{							
	HKEY hkLocal, hkLocalKey;
	DWORD dw;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies",
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
	{
		return;
	}
	if (RegOpenKeyEx(hkLocal,
		"System",
		0, KEY_WRITE | KEY_READ,
		&hkLocalKey) != ERROR_SUCCESS)
	{
		RegCloseKey(hkLocal);
		return;
	}
	LONG pref;
	pref = 1;
	RegSetValueEx(hkLocalKey, "SoftwareSASGeneration", 0, REG_DWORD, (LPBYTE) &pref, sizeof(pref));
	RegCloseKey(hkLocalKey);
	RegCloseKey(hkLocal);
}

DWORD CAD_SubService(LPVOID lpParam)
{
	OSVERSIONINFO OSversion;	
	OSversion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&OSversion);


	HDESK desktop = NULL;
	desktop = OpenInputDesktop(0, FALSE,
		DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
		DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
		DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
		DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);



	if (desktop == NULL)
		vnclog.Print(LL_INTERR, VNCLOG("OpenInputdesktop Error \n"));
	else 
		vnclog.Print(LL_INTERR, VNCLOG("OpenInputdesktop OK\n"));

	HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());
	DWORD dummy;

	char new_name[256];
	if (desktop)
	{
		if (!GetUserObjectInformation(desktop, UOI_NAME, &new_name, 256, &dummy))
		{
			vnclog.Print(LL_INTERR, VNCLOG("!GetUserObjectInformation \n"));
		}

		vnclog.Print(LL_INTERR, VNCLOG("SelectHDESK to %s (%x) from %x\n"), new_name, desktop, old_desktop);

		if (!SetThreadDesktop(desktop))
		{
			vnclog.Print(LL_INTERR, VNCLOG("SelectHDESK:!SetThreadDesktop \n"));
		}
	}

	if (!IsSoftwareCadEnabled())
	{
		Enable_softwareCAD();
	}

	typedef VOID (WINAPI *SendSas)(BOOL asUser);
	HINSTANCE Inst = LoadLibrary("sas.dll");
	SendSas sendSas = (SendSas) GetProcAddress(Inst, "SendSAS");
	if (sendSas) 
	{
		sendSas(FALSE);
	}
	if (Inst) FreeLibrary(Inst);

	if (old_desktop) 
	{
		SetThreadDesktop(old_desktop);
	}
	if (desktop)
	{
		CloseDesktop(desktop);
	}
	return 0;
}

bool IsVista()
{
	OSVERSIONINFO osver;

	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	
	if (::GetVersionEx(&osver) && 
		(osver.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
		(osver.dwMajorVersion >= 6))
		return true;

	return false;
}
