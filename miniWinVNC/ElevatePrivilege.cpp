#include "stdhdrs.h"
#include "WinVNC.h"
#include "vncService.h"
#include "ElevatePrivilege.h"

#include <lmcons.h>
#include <wininet.h>
#include <shlobj.h>
#include <VersionHelpers.h>

#include "../Common/InterfaceDllProxyInfo.h"
#include "Localization.h"

#ifndef USING_VS2008_EXPRESS
#include "HideDesktop.h"
#endif

#include "../Common/irUtility.h"	//#IR:100303 +

// Header

#include "vncMenu.h"

#include "windows.h"
#include "ElevatePrivilege.h"

char CServicePassword::m_passwd[MAX_PWD_LEN + 1];
bool CServicePassword::m_dlgResult;

static bool IsVista()
{
	OSVERSIONINFO osver;

	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	
	if (IsWindowsVistaOrGreater() && !IsWindows7OrGreater())
		return true;

	return false;
}

static bool RunElevated(char* szModulePath, char *szCmdLine, bool fWaitFor)
{
#define _BUFF_SIZE 1024
	char szExecutable[_BUFF_SIZE];
	bool bResult = false;
	char szSeparator[2] = {0, 0};
	SHELLEXECUTEINFO shex;
	char *p;
	TCHAR szFile[_MAX_PATH];

	memset(szExecutable, 0, _BUFF_SIZE);
	strcpy_s(szExecutable, _BUFF_SIZE, szModulePath);
	
	memset( &shex, 0, sizeof(shex));
	memset(szFile, 0, _MAX_PATH);

	p = strrchr(szModulePath, '\\');
	if (!p)
	{
		p = strrchr(szModulePath, '/');
	}
	if (p)
	{
		strcpy_s(szFile, _MAX_PATH, p + 1);
		*(p) = '\0';
	}

	shex.cbSize			= sizeof(SHELLEXECUTEINFO); 
	shex.fMask			= SEE_MASK_NOCLOSEPROCESS; 
	shex.hwnd			= NULL;
	shex.lpVerb			= "runas";
	shex.lpFile			= szFile; 
	shex.lpParameters	= szCmdLine; 
	shex.lpDirectory	= szModulePath; 
	shex.nShow			= SW_NORMAL; 

	if (::ShellExecuteEx(&shex))
	{
		bResult = true;
		if (fWaitFor)
		{
			DWORD dwExitCode = 0;
			WaitForSingleObject(shex.hProcess, INFINITE);
			if (GetExitCodeProcess(shex.hProcess, &dwExitCode))
			{
				bResult = !dwExitCode;
			}
		}
		CloseHandle(shex.hProcess);
	}
	return bResult;
}

bool CServicePassword::askVncPassword(char *password, int nMaxLen)
{
	memset(m_passwd, 0, sizeof(m_passwd));
	int ret = DialogBoxParam(GetModuleHandle(0),
		MAKEINTRESOURCE(IDD_SERICE_PASSWORD), 
		NULL, (DLGPROC)CServicePasswordDialogProc,
		NULL);

	if (m_dlgResult)
	{
		password[0] = 0;
		vncPasswd::FromText encrypted(m_passwd);
		const char *szEncrypted = encrypted;
		int i;
		int len = 0;
		memset(password, 0, nMaxLen);
		for (i = 0; szEncrypted[i] != '\0'; i++)
		{
			len += _snprintf_s(password + len, nMaxLen - len, nMaxLen - len - 1, 
							   "%02x", (BYTE)szEncrypted[i]);
		}
	}
	return m_dlgResult;
}

BOOL CALLBACK CServicePassword::CServicePasswordDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SendMessage(GetDlgItem(hwnd, IDC_SERVICE_ASK_PWD1),  EM_LIMITTEXT, MAX_PWD_LEN, 0);
			SendMessage(GetDlgItem(hwnd, IDC_SERVICE_ASK_PWD2),  EM_LIMITTEXT, MAX_PWD_LEN, 0);
			SetForegroundWindow(hwnd);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					EndDialog(hwnd, FALSE);
					return TRUE;
				case IDOK:
				{
					char pwd1[MAX_PWD_LEN + 1];
					char pwd2[MAX_PWD_LEN + 1];
					GetWindowText(GetDlgItem(hwnd, IDC_SERVICE_ASK_PWD1), pwd1, MAX_PWD_LEN + 1);
					GetWindowText(GetDlgItem(hwnd, IDC_SERVICE_ASK_PWD2), pwd2, MAX_PWD_LEN + 1);
					m_dlgResult = strcmp(pwd1, pwd2) == 0;
					if (m_dlgResult)
					{
						EndDialog(hwnd, TRUE);
						strncpy_s(m_passwd, pwd1, MAX_PWD_LEN + 1);
					}
					else
					{
						MessageBox(hwnd, "Sorry...passwords don't match.", "Configure VNC Password", MB_ICONINFORMATION);
					}
					return TRUE;
				}
			}
			break;
		case WM_DESTROY:
			EndDialog(hwnd, FALSE);
			return TRUE;
	}
	return 0;
}


bool CElevatePrivilege::m_dlgResult = false;
char CElevatePrivilege::m_user[32] = {0};
char CElevatePrivilege::m_passwd[32] = {0};

bool CElevatePrivilege::runAsUser(const char *szParam, const char *szCmdLine)
{
	m_dlgResult = false;
	m_passwd[0] = 0;
	m_user[0] = 0;

	if (IsVista())
	{
#define MAX_COMMAND_LINE 512
		char szBigCommandLine[MAX_COMMAND_LINE];
		char szFileName[MAX_PATH];
		GetModuleFileName(NULL, szFileName, MAX_PATH);
		_snprintf_s(szBigCommandLine, MAX_COMMAND_LINE, MAX_COMMAND_LINE - 1, "%s %s", szParam, szCmdLine);
		return RunElevated(szFileName, szCmdLine ? (char*)szBigCommandLine : (char*)szParam, true);
	}

	int ret =  DialogBoxParam(hAppInstance,
		MAKEINTRESOURCE(IDD_ELEVATE_PRIVILEGES), 
		NULL, (DLGPROC) ElevatePrivilegeDialogProc,
		NULL);

	if (m_dlgResult)
	{
		WCHAR wszFileName[MAX_PATH];
		WCHAR wszUser[32] = {0};
		WCHAR wszPassword[32] = {0};
		WCHAR wszParam[1024] = {0};
		PROCESS_INFORMATION pi = {0};
		STARTUPINFOW         si = {0};
		WCHAR delim = 32;
		int len;

		GetModuleFileNameW(NULL, wszFileName, MAX_PATH);

		MultiByteToWideChar(CP_ACP, 0, m_user, -1, wszUser, 32);
		MultiByteToWideChar(CP_ACP, 0, m_passwd, -1, wszPassword, 32);
		len = MultiByteToWideChar(CP_ACP, 0, szParam, -1, wszParam, 1024);

		if (szCmdLine)
		{
			wszParam[len++] = delim;
			len += MultiByteToWideChar(CP_ACP, 0, szCmdLine, -1, wszParam, 1024);
			wszParam[len++] = delim;
		}
		len += MultiByteToWideChar(CP_ACP, 0, szParam, -1, wszParam + len, 1024 - len);

		char pszParam[1024];
		char szFileName[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, wszParam, len, pszParam, 1024, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, wszFileName, wcslen(wszFileName), szFileName, MAX_PATH, NULL, NULL);
		si.cb = sizeof(STARTUPINFO);
		ret = CreateProcessWithLogonW(wszUser, NULL, wszPassword, 0, wszFileName, wszParam, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi);
		if (ret)
		{
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}
	return ret != 0;
}

BOOL CALLBACK CElevatePrivilege::ElevatePrivilegeDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SendMessage(GetDlgItem(hwnd, IDC_ELEVATED_ACCT_USER),  EM_LIMITTEXT, 31, 0);
			SendMessage(GetDlgItem(hwnd, IDC_ELEVATED_ACCT_PWD),  EM_LIMITTEXT, 31, 0);
			SetForegroundWindow(hwnd);
			return TRUE;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					EndDialog(hwnd, FALSE);
					return TRUE;
				case IDOK:
					GetWindowText(GetDlgItem(hwnd, IDC_ELEVATED_ACCT_USER), m_user, 32);
					GetWindowText(GetDlgItem(hwnd, IDC_ELEVATED_ACCT_PWD), m_passwd, 32);
					m_dlgResult = true;
					EndDialog(hwnd, TRUE);
					return TRUE;
			}
			break;

		case WM_DESTROY:
			EndDialog(hwnd, FALSE);
			return TRUE;
	}
	return 0;
}
