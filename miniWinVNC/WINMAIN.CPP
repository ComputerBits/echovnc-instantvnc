#include <windows.h>
#include <stdio.h>

#include <time.h>

#define SERVICE_NAME "MR_SRVC"
#define SERVICE_DISPLAY_NAME "Marian-SRVC"

enum {
	MR_SRVC_ERR_NONE        =       0,
	MR_SRVC_ERR_UNKNOWN,
	MR_SRVC_ERR_CANTCONNECT,
	MR_SRVC_ERR_TIMEOUT,
};

#define _DEBUGMSG
#ifdef _DEBUGMSG
	#define DMESG dprintf
#else
	#ifdef _V_CPP6_
		#define  __noop 0		
	#endif
	#define DMESG __noop
#endif

HANDLE g_h_stop;
static SERVICE_STATUS g_svcStatus;
static SERVICE_STATUS_HANDLE g_svcStatusHandle;

//volatile LONG g_bShutdownInProgess=0;
volatile LONG g_bServiceStopping=0;

#ifdef _DEBUG
	#define GetLastErrorMsg GetLastErrorMsgA
	static char *GetLastErrorMsgA(DWORD err)
	{
		static char szErrorString[MAX_PATH];

		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) szErrorString,
			MAX_PATH,
			NULL
		);
		return szErrorString;
	}
#else
	#define GetLastErrorMsg __noop
#endif

static void dprintf(const char *format...)
{
	char buffer[1024];
	va_list vl;
	va_start(vl, format);
	vsprintf(buffer, format, vl);
	OutputDebugString(buffer);
}

int srvc_init(int nargin, const char* args)
{
	DMESG("%s: %d args %s", __FUNCTION__, nargin, args ? args : "");
	return MR_SRVC_ERR_NONE;
}

int srvc_main()
{
	g_hStopEvent = CreateEvent(NULL, true, false, NULL);
	while(true)
	{
		if (WaitForSingleObject(g_hStopEvent, 1) == WAIT_OBJECT_0)
		{
			DMESG("%s: got STOP signal", __FUNCTION__);
			break;
		}
		DMESG("in %s", __FUNCTION__);
		Sleep(999);
	}
	return MR_SRVC_ERR_NONE;
}

int srvc_cleanup()
{
	DMESG("%s", __FUNCTION__);
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
		DMESG("%s: cannot open service control manager! (error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
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

static DWORD InstallService()
{
    SC_HANDLE schService, schSCManager;
	DWORD errcode;
	TCHAR szBinaryPathName[MAX_PATH];
	
	errcode = 0;
	if (GetModuleFileName(NULL, szBinaryPathName, MAX_PATH) == 0)
	{
		errcode = GetLastError();
		DMESG("%s: service install failed ! (error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
		return errcode;
	}

	strcat(szBinaryPathName, " -service");
	
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		DMESG("%s: service install failed ! (error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
		return errcode;
	}

	schService = CreateService(
        schSCManager,                /* SCManager database  */
        SERVICE_NAME,                /* name of service */
        SERVICE_DISPLAY_NAME,        /* service name to display */
        SERVICE_START | SERVICE_STOP | SERVICE_INTERROGATE,    /* desired access */
        SERVICE_WIN32_OWN_PROCESS, /* service type */
        SERVICE_AUTO_START,        /* start type */
        SERVICE_ERROR_NORMAL,      /* error control type */
        szBinaryPathName,          /* service's binary */
        NULL,                      /* no load ordering group */
        NULL,                      /* no tag identifier */
        NULL,					    /* no dependencies */
        NULL,                      /* LocalSystem account */
        NULL);                     /* no password */
 
    if (schService == NULL)
	{
		errcode = GetLastError();
		DMESG("%s: service install failed ! (error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
	}
    else 
		DMESG("%s: the service was successfully installed.\n", SERVICE_NAME);
 
    if (schService)
		CloseServiceHandle(schService);
	
	CloseServiceHandle(schSCManager);
	return errcode;
}

static DWORD UninstallService()
{
    SC_HANDLE schService, schSCManager;
	DWORD errcode;

	errcode = 0;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		DMESG("%s: service uninstall failed ! (error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
		return errcode;
	}

	schService = OpenService(
        schSCManager,              /* SCManager database  */
        SERVICE_NAME,              /* name of service */
        DELETE);
 
    if (schService == NULL)
	{
		errcode = GetLastError();
		DMESG("%s: service uninstall failed ! (error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
	}
	else
	{
		if (DeleteService(schService) == FALSE)
		{
			errcode = GetLastError();
			DMESG("%s: service uninstall failed ! (error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
			if (errcode == ERROR_SERVICE_DOES_NOT_EXIST)
			{
				DMESG("%s: Bypassing the error.\n", SERVICE_NAME);
				errcode = 0;
			}
		}
		else
			DMESG("%s: the service was successfully uninstalled.\n", SERVICE_NAME);
	}

    if (schService)
		CloseServiceHandle(schService);

	CloseServiceHandle(schSCManager);
	return errcode;
}

/*static: MR_ADAM*/ DWORD StartService()
{
    SC_HANDLE schService, schSCManager;
	DWORD errcode = 0;
	
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		DMESG("%s: service start failed !\n (Error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
		return errcode;
	}
	schService = OpenService(
        schSCManager,              /* SCManager database  */
        SERVICE_NAME,              /* name of service */
        SERVICE_START);
 
    if (schService == NULL)
	{
		errcode = GetLastError();
		DMESG("%s: service start failed !\n (Error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
	}
	else
	{
		if (StartService(schService, 0, NULL) == FALSE)
		{
			errcode = GetLastError();
			DMESG("%s: service start failed !\n (Error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
		}
		else
			DMESG("%s: service started.\n", SERVICE_NAME);
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
	DMESG("StopService in progress...");
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		DMESG("%s: service stop failed ! (Error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
		return errcode;
	}

	schService = OpenService(
		schSCManager,              /* SCManager database  */
		SERVICE_NAME,              /* name of service */
		SERVICE_STOP | SERVICE_QUERY_STATUS);
	
	if (schService == NULL)
	{
		errcode = GetLastError();
		DMESG("%s: service stop failed ! (Error code: 0x%x, which means:%s).\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
	}
	else
	{
		if (ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus) == FALSE)
		{
			errcode = GetLastError();
			DMESG("%s: service stop failed ! (error code: %d, which means:%s)\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
			if ((errcode == ERROR_SERVICE_NOT_ACTIVE) || (errcode == ERROR_SERVICE_DOES_NOT_EXIST))
			{
				DMESG("%s: Bypassing the error.\n", SERVICE_NAME);
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
					DMESG("%s: service stop failed ! (error code: 0x%x, which means:%s)\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
					break;
				}

				if (svcStatus.dwCurrentState != SERVICE_STOPPED)
				{
					dwTimer++;
					Sleep(500);
					DMESG("StopService is counting down waiting for state to become SERVICE_STOPPED...");
					if (dwTimer == 40) /* = 20 seconds */
						bTimeout = TRUE;
				}			
			}while ((bTimeout == FALSE) && (svcStatus.dwCurrentState != SERVICE_STOPPED));
			
			if (bTimeout)
			{
				DMESG("%s: timeout occured while stopping !\n", SERVICE_NAME);
				errcode = 0x20000000 | MR_SRVC_ERR_TIMEOUT;
			} 
			else
			{
				if (errcode)
				{
					DMESG("%s: trying to bypass the error, but it may not work if the service doesn't stop quickly\n", SERVICE_NAME);
					errcode = 0;
				}
				else
				{
					DMESG("%s: service stopped.\n", SERVICE_NAME);
				}		
			}
		}			
	}
	
	if (schService)
		CloseServiceHandle(schService);

	CloseServiceHandle(schSCManager);
	return errcode;
}

/*MR_ADAM*///static VOID ServiceCtrlHandler(DWORD opcode)
/*MR_ADAM*/void WINAPI ServiceCtrlHandler(DWORD opcode)
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
			DMESG("[MR-Srvc stop]ServiceCtrlHandler: Setting Service status=SERVICE_STOP_PENDING");
			g_svcStatus.dwWin32ExitCode = 0;
			g_svcStatus.dwCheckPoint = 1;
			g_svcStatus.dwWaitHint = 20000;
			g_svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
			result=SetServiceStatus (g_svcStatusHandle,  &g_svcStatus);
			if (!result)
			{
				DMESG("[MR-Srvc stop]ServiceCtrlHandler - FAILED to set Service status=SERVICE_STOP_PENDING");
			}
			if (g_hStopEvent == INVALID_HANDLE_VALUE)
			{
				DMESG("[MR-Srvc stop]ServiceCtrlHandler: panic: global syncronization Event is invalid!\nStopping suddenly...");
				ExitProcess(0xEEEEEEEE);
			}
			else
			{
				DMESG("[MR-Srvc stop]ServiceCtrlHandler: Signalling stop event");
				result=SetEvent(g_hStopEvent);
				if (!result)
				{
					DMESG("[MR-Srvc stop]ServiceCtrlHandler: Signalling stop event FAILED");
				}
			}
			break;
		case SERVICE_CONTROL_INTERROGATE:
			g_svcStatus.dwCheckPoint = 50;
			g_svcStatus.dwWaitHint = 5000;
			if (g_bServiceStopping)
			{
				DMESG("[MR-Srvc stop]ServiceCtrlHandler: SERVICE_CONTROL_INTERROGATE");
			}
			SetServiceStatus (g_svcStatusHandle,  &g_svcStatus);
			break;		
		default:
//			g_svcStatus.dwCheckPoint = 50;
//			g_svcStatus.dwWaitHint = 5000;
//			SetServiceStatus (g_svcStatusHandle,  &g_svcStatus);
			break;
	}
}

static INT DebugService()
{
	INT status = srvc_init(0, NULL);
	if (status != 0)
	{
		DMESG("%s: initialization failed! (error code: %d)\n", SERVICE_NAME, status);
		return status;
	}
	srvc_main();

	return srvc_cleanup();
	return 0;
}

void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD status;
	DWORD errcode;

	g_svcStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
	
	if (!g_svcStatusHandle)
	{
		errcode = GetLastError();
		DMESG("%s: failed to register control handler! (error code: 0x%08X, meaning %s)\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
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
		errcode = GetLastError();
		DMESG("%s: failed to set service status! (error code: 0x%x, which means:%s)\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
		return;
	}

	if(!SetProcessShutdownParameters(0x4FF, 0))
	{
		DMESG("SetProcessShutdownParameters() failed! Reason:\r\n", GetLastErrorMsg(GetLastError()));
		g_svcStatus.dwCurrentState = SERVICE_STOPPED;
		g_svcStatus.dwCheckPoint = 0;
		g_svcStatus.dwWaitHint = 0;
		g_svcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		g_svcStatus.dwServiceSpecificExitCode = 1;
		SetServiceStatus(g_svcStatusHandle, &g_svcStatus);
		return;
	}
	
	/* Initialize */
	status = srvc_init(argc, (char*)argv);
	if (status != MR_SRVC_ERR_NONE)
	{
		DMESG("%s: initialization failed! (error code: %d)\n", SERVICE_NAME, status);
		g_svcStatus.dwCurrentState = SERVICE_STOPPED;
		g_svcStatus.dwCheckPoint = 0;
		g_svcStatus.dwWaitHint = 0;
		g_svcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		g_svcStatus.dwServiceSpecificExitCode = status;
		SetServiceStatus(g_svcStatusHandle, &g_svcStatus);
		srvc_cleanup();
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
		errcode = GetLastError();
		DMESG("%s: failed to set service status! (error code: 0x%x, which means:%s)\n", SERVICE_NAME, errcode, GetLastErrorMsg(errcode));
		return;
	}

	/* main processing function */
	if (srvc_main() != MR_SRVC_ERR_NONE)
	{
		DMESG("[MR-Srvc stop]ServiceMain: srv_main ended abnormally");
	}

	/* uninitialize */
	if (srvc_cleanup() != MR_SRVC_ERR_NONE)
	{
		DMESG("[MR-Srvc stop]ServiceMain: srv_uninit ended abnormally!");
//		ExitProcess(0xE2);
	}

	g_svcStatus.dwWin32ExitCode = 0;
	g_svcStatus.dwCheckPoint = 50;
	g_svcStatus.dwWaitHint = 5000;
	g_svcStatus.dwCurrentState = SERVICE_STOPPED;
	if (!SetServiceStatus(g_svcStatusHandle, &g_svcStatus))
	{
		errcode = GetLastError();
		DMESG("[MR-Srvc stop]ServiceMain: FAILED to set service status to SERVICE_STOPPED!\nReported error: %s", GetLastErrorMsg(errcode));
//		ExitProcess(0xE1);
		return;
	}

	DMESG("[MR-Srvc stop]ServiceMain - returning control to parent...");
}

/*
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DWORD result = 0;
	SERVICE_TABLE_ENTRY DispatchTable[] = {{SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
											{NULL, NULL}};
	if (strstr(lpCmdLine, "DEBUG"))
		return DebugService();
    if (strstr(lpCmdLine, "stop"))
		result = StopService();
	else 
	{
		if (strstr(lpCmdLine, "uninstall"))
			result = UninstallService();
		else 
		{
			if (strstr(lpCmdLine, "install"))
				result = InstallService();
			else
			{
				if (!IsServiceInstalled(SERVICE_NAME))
					InstallService();
		
				if (strstr(lpCmdLine, "service"))
				{
					if (StartServiceCtrlDispatcher(DispatchTable) == FALSE)
						result=GetLastError();
				} 
				else
					result = StartService();
			}
		}
	}
	return result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DMESG("%s: called with: %s", SERVICE_NAME, lpCmdLine);
	nResult=WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	DMESG("%s: exits with return value %d", SERVICE_NAME, nResult);
	return nResult;
}
*/
BOOL CanStartService()
{
    SC_HANDLE schService, schSCManager;
	DWORD errcode = 0;
	
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		errcode = GetLastError();
		DMESG("Service query failed !\n (Reported error: %s).\n", GetLastErrorMsg(errcode));
		return FALSE;
	}
	schService = OpenService(
        schSCManager,              /* SCManager database  */
        SERVICE_NAME,              /* name of service */
        SERVICE_QUERY_STATUS);
 
    if (schService == NULL)
	{
		errcode = GetLastError();
		DMESG("Service query failed !\n (Reported error: %s).\n", GetLastErrorMsg(errcode));
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
			{SERVICE_NAME, ServiceMain},
			{NULL, NULL}
		};

int main(int nargin, char* args[])
{
	int result=0;
	char *lpCmdLine="";
	if ((nargin>1) && (args[1]))
		lpCmdLine=args[1];
	DMESG("%s: called with: %s", SERVICE_NAME, lpCmdLine);
	if (strstr(lpCmdLine, "stop"))
		result = StopService();
	else 
	{
		if (strstr(lpCmdLine, "uninstall"))
			result = UninstallService();
		else 
		{
			if (strstr(lpCmdLine, "install"))
				result = InstallService();
			else
			{
				if (!IsServiceInstalled(SERVICE_NAME))
					InstallService();

				if (strstr(lpCmdLine, "service"))
				{
					DMESG("Passing by here2");
					if (!CanStartService())
					{
						DMESG("[MR-Srvc stop] Service is already running!");
						return -1;
					}
					if (StartServiceCtrlDispatcher(DispatchTable) == FALSE)
						result = GetLastError();
				} 
				else
					result = StartService();
			}
		}
	}
	DMESG("[MR-Srvc stop]main returns %d", result);
	return result;
}