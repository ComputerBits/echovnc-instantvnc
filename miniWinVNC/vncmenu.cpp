//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
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


// vncMenu

// Implementation of a system tray icon & menu for WinVNC

extern volatile bool g_bStopped;
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
DWORD InstallService(const char *szCmdLine, std::string &strAbsoluteFileName);
int InstallHelperTray(const char *szFullPath);
int UninstallHelperTray();

DWORD UninstallService();
BOOL IsServiceInstalled();

extern bool m_bAutoExitAfterLastViewer;
extern bool m_bAutoExitIfNoViewer;
extern HANDLE g_hStopEvent;

BOOL m_bWasAero = FALSE;
INT m_iFlashState = 0;

#include "../COMMON/UserData.h"
extern CUserData* g_pUserData;

// Constants
const UINT MENU_PROPERTIES_SHOW = RegisterWindowMessage("EchoVNC.Properties.User.Show");
const UINT MENU_DEFAULT_PROPERTIES_SHOW = RegisterWindowMessage("EchoVNC.Properties.Default.Show");
const UINT MENU_ABOUTBOX_SHOW = RegisterWindowMessage("EchoVNC.AboutBox.Show");
const UINT MENU_SERVICEHELPER_MSG = RegisterWindowMessage("EchoVNC.ServiceHelper.Message");
const UINT MENU_ADD_CLIENT_MSG = RegisterWindowMessage("EchoVNC.AddClient.Message");
const UINT MENU_REMOVE_CLIENTS_MSG = RegisterWindowMessage("EchoVNC.RemoveClients.Message"); // REalVNc 336

const UINT FileTransferSendPacketMessage = RegisterWindowMessage("EchoVNC.Viewer.FileTransferSendPacketMessage");

const char *MENU_CLASS_NAME = "EchoVNC Tray Icon";

BOOL g_restore_ActiveDesktop = FALSE;
bool RunningAsAdministrator ();
void WriteToFile(char *szText);

bool CPipe::TalkToPeer(CPipe::CIpcMessage *ipcMessage, int peerIndex)
{
	HANDLE hPipe;
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	DWORD dwWritten;
	DWORD dwRead;
	char buffer[sizeof(CPipe::CIpcMessage)];
	CPipe::CIpcMessage *incomingIpcMessage = (CPipe::CIpcMessage *)buffer;
	
	_ASSERT(g_bIsHelperTray || g_bIsSubService);
	
	while (WaitForSingleObject(g_hStopEvent, 1) == WAIT_TIMEOUT)
	{
		hPipe = CreateFile((peerIndex == INDEX_PIPE_SERVICE) ? SHARED_IPC_PIPE_SERVICE_TO_SUBSERVICE : SHARED_IPC_PIPE_SERVICE_TO_HELPER, 
                            GENERIC_WRITE | GENERIC_READ, 
                            0, 
                            NULL, 
                            OPEN_EXISTING, 
                            0, 
                            NULL);
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			break;
		}
		if (!WaitNamedPipe((peerIndex == INDEX_PIPE_SERVICE) ? SHARED_IPC_PIPE_SERVICE_TO_SUBSERVICE : SHARED_IPC_PIPE_SERVICE_TO_HELPER, 
                            TIMER_INTERVAL_TRAY_POLL_SERVICE / 2))
		{
			CloseHandle(hPipe);
			return false;
		}
	}

	SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
	WriteFile(hPipe, ipcMessage, sizeof(CPipe::CIpcMessage), &dwWritten, NULL);
	if (ReadFile(hPipe, buffer, sizeof(CPipe::CIpcMessage), &dwRead, NULL))
	{
		memcpy(ipcMessage->data, incomingIpcMessage->data, dwRead);
	}
	CloseHandle(hPipe);

	return ipcMessage->trans_id == incomingIpcMessage->trans_id;
}

static void KillWallpaper()
{
#ifndef USING_VS2008_EXPRESS
	HideDesktop();
#endif
}

static void RestoreWallpaper()
{
#ifndef USING_VS2008_EXPRESS
	RestoreDesktop();
#endif
}

// Implementation

vncMenu::vncMenu(vncServer *server, int runMode) : m_runMode(runMode), m_server(server), m_bServiceInstalled(false)
{
	CoInitialize(0);

	if (m_runMode == VNC_RUN_MODE_SERVICE)
	{
		if (PropsInit() != CONNECTION_SUCCESSFUL) //#here
		{
			PostQuitMessage(0);
		}
		return;
	}
	if (m_runMode == VNC_RUN_MODE_HELPER_TRAY)
	{										  
		m_bServiceInstalled = true;
		memset(&m_connInfo, 0, sizeof(ConnInfo));
	}

	// Create a dummy window to handle tray icon messages
	WNDCLASSEX wndclass;

	wndclass.cbSize			= sizeof(wndclass);
	wndclass.style			= 0;
	wndclass.lpfnWndProc	= vncMenu::WndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= hAppInstance;
	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName	= (const char *) NULL;
	wndclass.lpszClassName	= MENU_CLASS_NAME;
	wndclass.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);

	m_hwnd = CreateWindow(MENU_CLASS_NAME, MENU_CLASS_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
							200, 200, NULL, NULL, hAppInstance, NULL);
	if (m_hwnd == NULL)
	{
		PostQuitMessage(0);
		return;
	}

	// record which client created this window
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG) this);

	UINT nImageColor = LR_VGACOLOR;
	if (IsWindowsVersionOrGreater(5, 0, 0) && !IsWindowsVistaOrGreater())
		nImageColor = LR_DEFAULTCOLOR;

	m_winvnc_icon = (HICON)LoadImage(hAppInstance, MAKEINTRESOURCE(IDI_WINVNC), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), nImageColor);
	m_flash_icon = (HICON)LoadImage(hAppInstance, MAKEINTRESOURCE(IDI_FLASH), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), nImageColor);

	// Load the popup menu
	m_hmenu = LoadMenu(hAppInstance, MAKEINTRESOURCE(IDR_TRAYMENU));

	if (m_runMode != VNC_RUN_MODE_SUB_SERVICE)
	{
		StartTimerAutoExitIfNoViewer(m_hwnd);
		AddTrayIcon();
	}
	if ((m_runMode != VNC_RUN_MODE_HELPER_TRAY))
	{
		// Set the initial user name to something sensible...
		vncService::CurrentUser((char *)&m_username, sizeof(m_username));

		// Ask the server object to notify us of stuff
		server->AddNotify(m_hwnd);

		// Initialise the properties dialog object
		if (PropsInit() != CONNECTION_SUCCESSFUL) //#here
		{
			PostQuitMessage(0);
			return;
		}

	        if (m_runMode != VNC_RUN_MODE_SUB_SERVICE)
        	{
    			if (!server->GetDisableTrayIcon())
    				StartTimerFlashTrayIcon(m_hwnd);
	        }
	}
}

vncMenu::~vncMenu()
{
	// Tell the server to stop notifying us!
	if (m_server != NULL)
	{
		m_server->RemNotify(m_hwnd);

		if (m_server->RemoveWallpaperEnabled())
			RestoreWallpaper();
	}

	if (m_runMode != VNC_RUN_MODE_SUB_SERVICE)
	{
    		// Remove the tray icon
	    	DelTrayIcon();
	}
    
	// Destroy the loaded menu
	if (m_hmenu != NULL)
		DestroyMenu(m_hmenu);
}

int vncMenu::PropsInit()
{
	if (m_runMode == VNC_RUN_MODE_HELPER_TRAY)
	{
		return 0;
	}
	if (g_bStopped)
	{
		return -1;
	}
	return m_properties.Init(m_server);
}

void vncMenu::AddTrayIcon()
{
	if (m_runMode == VNC_RUN_MODE_SUB_SERVICE)
		return;

	HMENU hmenu = GetSubMenu(m_hmenu, 0);

	if (m_runMode == VNC_RUN_MODE_HELPER_TRAY)
	{
		SendTrayMsg(NIM_ADD, FALSE);
	
		if (!m_bServiceInstalled)
		{
			RemoveMenu(hmenu, ID_STOP_AUTORUN, MF_BYCOMMAND);
		}
	}
	else 
	{
		HMENU hmenu = GetSubMenu(m_hmenu, 0);

		// If the user name is non-null then we have a user!
		if (strlen(m_username) > 0)
			if (!m_server->GetDisableTrayIcon())
				SendTrayMsg(NIM_ADD, FALSE);

		if ((m_runMode != VNC_RUN_MODE_REGULAR_APP) && m_bServiceInstalled)
		{
			RemoveMenu(hmenu, ID_AUTORUN, MF_BYCOMMAND);
		}
	}
	RemoveMenu(hmenu, (m_runMode != VNC_RUN_MODE_REGULAR_APP) ? ID_AUTORUN : ID_STOP_AUTORUN, MF_BYCOMMAND);
}

void
vncMenu::DelTrayIcon()
{
	SendTrayMsg(NIM_DELETE, FALSE);
}

#define  DWM_EC_DISABLECOMPOSITION  0
#define  DWM_EC_ENABLECOMPOSITION   1

void
vncMenu::FlashTrayIcon(BOOL flash)
{
	SendTrayMsg(NIM_MODIFY, flash);
	if (IsWindowsVistaOrGreater())
	{
		typedef HRESULT(WINAPI *ISAERO)(BOOL*);
		typedef HRESULT(WINAPI *CHANGEAERO)(UINT);
		ISAERO pDwmIsCompositionEnabled = NULL;
		CHANGEAERO pDwmEnableComposition = NULL;
		HMODULE hDLL = LoadLibrary("DWMAPI.dll");
		if (hDLL)
		{
			pDwmIsCompositionEnabled = (ISAERO)GetProcAddress(hDLL, "DwmIsCompositionEnabled");
			pDwmEnableComposition = (CHANGEAERO)GetProcAddress(hDLL, "DwmEnableComposition");
		}
		else
			return;

		if (m_iFlashState == flash)
			return;

		m_iFlashState = flash;
		//Disable/Enable Aero Under Vista
		if (flash)
		{
			pDwmIsCompositionEnabled(&m_bWasAero);
			if (m_bWasAero == TRUE)
				pDwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
		}
		else
		{
			if (m_bWasAero == TRUE)
				pDwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
		}
	}
}

// Connected to: <echoserver address>
// Connected as: <username>
// Password: <password>  #if there is a password
extern bool GetEchoWareData( std::string& sAddr, std::string& sUserName, std::string& sPass);
#include <Windows.h>

void vncMenu::SendTrayMsg(DWORD msg, BOOL flash)
{
	// Create the tray icon message
	m_nid.hWnd = m_hwnd;
	m_nid.cbSize = sizeof(m_nid);
	m_nid.uID = IDI_WINVNC;			// never changes after construction
	m_nid.hIcon = flash ? m_flash_icon : m_winvnc_icon;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE;
	m_nid.uCallbackMessage = WM_TRAYNOTIFY;

	// Use resource string as tip if there is one
	if (LoadString(hAppInstance, IDI_WINVNC, m_nid.szTip, sizeof(m_nid.szTip)))
	    m_nid.uFlags |= NIF_TIP;
	
	// Try to add the server's IP addresses to the tip string, if possible
	if (m_nid.uFlags & NIF_TIP)
	{
		if (m_runMode == VNC_RUN_MODE_HELPER_TRAY)
		{
			composeTrayTip(m_connInfo.szAddress, m_connInfo.szUser, m_connInfo.szPasswd, 128, m_nid.szTip);
		}
		else
		{
			composeTrayTip(m_server, 128, m_nid.szTip);
		}
	}

	const char* szText;

	if (m_runMode == VNC_RUN_MODE_HELPER_TRAY)
	{
		szText = (m_connInfo.szUser[0] == '\0') ? sz_IDS_STRING41077 : sz_IDS_STRING41076;
	}
	else
	{
		szText = m_server->SockConnected() ? sz_IDS_STRING41076 : sz_IDS_STRING41077;
	}

	HMENU hmenu = GetSubMenu(m_hmenu, 0);
	ModifyMenu(hmenu, ID_CONNECT, MF_BYCOMMAND | MF_STRING, ID_CONNECT, (LPTSTR)szText);
	ModifyMenu(hmenu, ID_ABOUT, MF_BYCOMMAND | MF_STRING, ID_ABOUT, (LPTSTR)sz_IDS_STRING41078);
	ModifyMenu(hmenu, ID_CLOSE, MF_BYCOMMAND | MF_STRING, ID_CLOSE, (LPTSTR)sz_IDS_STRING41079);
	ModifyMenu(hmenu, ID_AUTORUN, MF_BYCOMMAND | MF_STRING, ID_AUTORUN, (LPTSTR)sz_IDS_STRING41092);
	ModifyMenu(hmenu, ID_STOP_AUTORUN, MF_BYCOMMAND | MF_STRING, ID_STOP_AUTORUN, (LPTSTR)sz_IDS_STRING41093);

	// Send the message
	if (Shell_NotifyIcon(msg, &m_nid))
	{
		if (m_runMode != VNC_RUN_MODE_HELPER_TRAY)
		{
			// Set the enabled/disabled state of the menu items
			EnableMenuItem(m_hmenu, ID_ADMIN_PROPERTIES, m_properties.AllowProperties() && RunningAsAdministrator() ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(m_hmenu, ID_CLOSE, m_properties.AllowShutdown() ? MF_ENABLED : MF_GRAYED);
		}
	}
}

BOOL vncMenu::StartTimerAutoExitAfterLastViewer(HWND hwnd)
{
	vnclog.Print(LL_INTINFO, "Auto-exit after last viewer disconnects timer starts...");
	return SetTimer(hwnd, TIMER_ID_AUTO_EXIT_AFTER_LAST_VIEWER, 
		TIMER_INTERVAL_AUTO_EXIT_AFTER_LAST_VIEWER, NULL) != 0;
}

BOOL vncMenu::StopTimerAutoExitAfterLastViewer(HWND hwnd)
{
	vnclog.Print(LL_INTINFO, "Auto-exit after last viewer disconnects timer ends...");
	return KillTimer(hwnd, TIMER_ID_AUTO_EXIT_AFTER_LAST_VIEWER);
}

BOOL vncMenu::StartTimerAutoExitIfNoViewer(HWND hwnd)
{
	vnclog.Print(LL_INTINFO, "Auto-exit if no viewer disconnects timer starts...");
	return SetTimer(hwnd, TIMER_ID_AUTO_EXIT_IF_NO_VIEWER, 
		TIMER_INTERVAL_AUTO_EXIT_IF_NO_VIEWER, NULL) != 0;
}

BOOL vncMenu::StopTimerAutoExitIfNoViewer(HWND hwnd)
{
	vnclog.Print(LL_INTINFO, "Auto-exit if no viewer disconnects timer ends...");
	return KillTimer(hwnd, TIMER_ID_AUTO_EXIT_IF_NO_VIEWER);
}

BOOL vncMenu::StartTimerFlashTrayIcon(HWND hwnd)
{
	return SetTimer(hwnd, TIMER_ID_FLASH_TRAY_ICON, TIMER_INTERVAL_FLASH_TRAY_ICON, NULL) != 0;
}

BOOL vncMenu::StopTimerFlashTrayIcon(HWND hwnd)
{
	return KillTimer(hwnd, TIMER_ID_FLASH_TRAY_ICON);
}

extern void CloseConnection();
extern int ConnectAgain();
extern void CloseStartupDlg();


static bool installServiceAsUser(const char *szCommandLine)
{
	CElevatePrivilege runas;
	return runas.runAsUser("install", szCommandLine);
}

static bool uninstallServiceAsUser()
{
	CElevatePrivilege runas;
	return runas.runAsUser("uninstall");
}

// Process window messages
LRESULT CALLBACK vncMenu::WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	vncMenu *_this = (vncMenu*)GetWindowLong(hwnd, GWL_USERDATA);

	switch (iMsg)
	{
		// Every five seconds, a timer message causes the icon to update
		case WM_TIMER:
			if (wParam == TIMER_ID_FLASH_TRAY_ICON)
				_this->FlashTrayIcon(_this->m_server->AuthClientCount() != 0);
			else if (wParam == TIMER_ID_AUTO_EXIT_AFTER_LAST_VIEWER)
			{
				_this->StopTimerAutoExitAfterLastViewer(hwnd);
				if (m_bAutoExitAfterLastViewer)
				{
					vnclog.Print(LL_INTINFO, "Auto-exit after last viewer disconnects...");
					PostMessage(hwnd, WM_COMMAND, ID_CLOSE, 0);
				}
			}
			else if (wParam == TIMER_ID_AUTO_EXIT_IF_NO_VIEWER)
			{
				_this->StopTimerAutoExitIfNoViewer(hwnd);
				if (m_bAutoExitIfNoViewer)
				{
					vnclog.Print(LL_INTINFO, "Auto-exit if no viewer connects...");
					PostMessage(hwnd, WM_COMMAND, ID_CLOSE, 0);
				}
			}
			else if (TIMER_ID_TRAY_POLL_SERVICE)
			{
				_ASSERT(_this->m_runMode == VNC_RUN_MODE_HELPER_TRAY);
				_this->performServerPolling();
			}
			break;

		// DEAL WITH NOTIFICATIONS FROM THE SERVER:
		case WM_SRV_CLIENT_AUTHENTICATED:
			_this->StopTimerAutoExitIfNoViewer(hwnd);
			_this->StopTimerAutoExitAfterLastViewer(hwnd);
		case WM_SRV_CLIENT_DISCONNECT:
			// Adjust the icon accordingly
			_this->FlashTrayIcon(_this->m_server->AuthClientCount() != 0);

			if (_this->m_server->AuthClientCount() != 0)
			{
				if (_this->m_server->RemoveWallpaperEnabled())
					KillWallpaper();
			}
			else
			{
				_this->StopTimerAutoExitAfterLastViewer(hwnd);
				_this->StartTimerAutoExitAfterLastViewer(hwnd);
				if (_this->m_server->RemoveWallpaperEnabled())
					RestoreWallpaper();
			}
			return 0;

		case WM_COMMAND:
			// User has clicked an item on the tray menu
			switch (LOWORD(wParam))
			{
				case ID_AUTORUN:
				{
					int result = MessageBox(hwnd, "Are you sure you want InstantVNC to be active on this computer all of the time?", "Automatically Start InstantVNC", MB_YESNO | MB_ICONQUESTION);
					if (result == IDYES)
					{
#define MAX_KEY_LEN 1023
#define MAX_COPY (MAX_KEY_LEN - len)
						CServicePassword srvcPasswd;
						char szValue[MAX_KEY_LEN + 1];
						char password[(MAX_PWD_LEN << 1) + 1];
						int len = 0;

						memset(szValue, 0, sizeof(szValue));
						if (!srvcPasswd.askVncPassword(password, (MAX_PWD_LEN << 1) + 1))
						{
							break;
						}
						len += _snprintf_s(szValue + len, MAX_KEY_LEN, MAX_COPY, "vncPwd=%s", password);
						len += _snprintf_s(szValue + len, MAX_COPY, MAX_COPY, " %s=%s", 
										"Password", g_pUserData->m_szPassword.c_str());
						len += _snprintf_s(szValue + len, MAX_COPY, MAX_COPY, " %s=%s %s=%s", 
										"Username", g_pUserData->m_szUsername.c_str(), 
										"Address", g_pUserData->m_szAddress.c_str());
						if (g_pUserData->m_szPort.length() > 0)
						{
							len += _snprintf_s(szValue + len, MAX_COPY, MAX_COPY, ":%s", g_pUserData->m_szPort.c_str());
						}
						if ((g_pUserData->m_proxy.m_strHost.length() > 0) && (g_pUserData->m_proxy.m_strUsername.length() > 0))
						{
							len += _snprintf_s(szValue + len, MAX_COPY, MAX_COPY, " %s=%s %s=%s",
											"Huser", g_pUserData->m_proxy.m_strUsername.c_str(),
											"Haddr", g_pUserData->m_proxy.m_strHost.c_str());
							if (g_pUserData->m_proxy.m_strPort.length() > 0)
							{
								len += _snprintf_s(szValue + len, MAX_COPY, MAX_COPY, ":%s", g_pUserData->m_proxy.m_strPort.c_str());
							}
						}
						if (g_pUserData->m_szImagePath.length() > 0)
						{
							len += _snprintf_s(szValue + len, MAX_COPY, MAX_COPY, " %s=\"%s\"", "Image", g_pUserData->m_szImagePath.c_str());
						}
						if (g_pUserData->m_szIniPath.length() > 0)
						{
							len += _snprintf_s(szValue + len, MAX_COPY, MAX_COPY, " %s=\"%s\"", "UseIni", g_pUserData->m_szIniPath.c_str());
						}

						LPSTR szCmdLine = GetCommandLine();
						if (szCmdLine && (strstr(szCmdLine, "-debug=") != NULL))
						{
							len += _snprintf_s(szValue + len, MAX_KEY_LEN, MAX_COPY, " -debug=.");
						}
						len += _snprintf_s(szValue + len, MAX_COPY, MAX_COPY, " -autorun -silent");

						std::string strAbsoluteFileName("");
						if (0 == ::InstallService(szValue, strAbsoluteFileName))
						{
							int ret = ::InstallHelperTray(strAbsoluteFileName.c_str());
							if (ret == ERROR_SUCCESS)
							{
								_this->m_bServiceInstalled = true;
								_this->AddTrayIcon();
							}
							else
							{
								MessageBox(NULL, "Failed to schedule controller application to run automatically at login", "Install InstantVNC service", MB_ICONASTERISK);
							}
						}
						else
						{
							DWORD errcode = GetLastError();

							if (ERROR_ACCESS_DENIED == errcode)
							{
								if (installServiceAsUser(szValue))
								{
									_this->m_bServiceInstalled = true;
									_this->AddTrayIcon();
								}
								else
								{
									errcode = GetLastError();
									if (ERROR_ACCESS_DENIED == errcode)
										MessageBox(hwnd, "Service cannot be installed", "InstantVNC service install", MB_ICONASTERISK);
								}
							}
							else
							{
								if (::IsServiceInstalled())
								{
									MessageBox(hwnd, "Service cannot be installed", "A service with the same name is already installed", MB_ICONASTERISK);
									_this->m_bServiceInstalled = true;
								}
								else
									MessageBox(hwnd, "Service cannot be installed", "InstantVNC service install", MB_ICONASTERISK);
							}
						}
#undef MAX_COPY 
#undef MAX_KEY_LEN
					}
				}
				break;

				case ID_STOP_AUTORUN:
				{
					int result = MessageBox(hwnd, "Are you sure you want to disable InstantVNC from auto-starting after a reboot?", "InstantVNC service", MB_YESNO | MB_ICONQUESTION);
					if (result == IDYES)
					{
						if (0 == (result = ::UninstallService()))
						{
							if (UninstallHelperTray() == 0)
							{
								_this->m_bServiceInstalled = false;
								_this->AddTrayIcon();
							}
							else
							{
								MessageBox(NULL, "Failed to disable autorun of controller application at login", "Uninstall InstantVNC service", MB_ICONASTERISK);
							}
						}
						else
						{
							if (ERROR_ACCESS_DENIED == result)
							{
								if (uninstallServiceAsUser())
								{
									_this->m_bServiceInstalled = false;
									_this->AddTrayIcon();
								}
								else
								{
									MessageBox(hwnd, "Service cannot be uninstalled", "InstantVNC service install", MB_ICONASTERISK);
								}
							}
							else
							{
								MessageBox(hwnd, "Service cannot be uninstalled", "InstantVNC service uninstall", MB_ICONASTERISK);
							}
						}
					}
				}
				break;

				case ID_CONNECT:
					if (_this->m_runMode == VNC_RUN_MODE_HELPER_TRAY)
					{
						_this->performServerDisConnect(_this->m_connInfo.szUser[0] == '\0');
						return 0;
					}
					if (_this->m_server->SockConnected())
					{
						_this->m_server->KillAuthClients(TRUE);
						CloseConnection();
						CloseStartupDlg();
					}
					else
					{
						ConnectAgain();
					}  
					break;

				case ID_ABOUT:
					_this->m_about.Show(TRUE);
					break;

				case ID_CLOSE:
					if (_this->m_runMode != VNC_RUN_MODE_HELPER_TRAY)
					{
						g_pUserData->m_bClose = TRUE;
						if (_this->m_server->SockConnected())
						{
							_this->m_server->KillAuthClients(TRUE);
							CloseConnection();
							CloseStartupDlg();
						}
					}

				PostMessage(hwnd, WM_CLOSE, 0, 0);
				break;
			}
			return 0;

			case WM_TRAYNOTIFY:
			{
				// Get the submenu to use as a pop-up menu
				HMENU submenu = GetSubMenu(_this->m_hmenu, 0);

				// What event are we responding to, RMB click?
				if (lParam == WM_RBUTTONUP)
				{
					if (submenu == NULL)
					{ 
						vnclog.Print(LL_INTERR, VNCLOG("no submenu available"));
						return 0;
					}

					// Get the current cursor position, to display the menu at
					POINT mouse;
					GetCursorPos(&mouse);

					SetForegroundWindow(_this->m_nid.hWnd);

					// Display the menu at the desired position
					TrackPopupMenu(submenu, 0, mouse.x, mouse.y, 0, _this->m_nid.hWnd, NULL);

					return 0;
				}

				// Or was there a LMB double click?
				if (lParam == WM_LBUTTONDBLCLK)
				{
					// double click: execute first menu item
					SendMessage(_this->m_nid.hWnd, WM_COMMAND,  GetMenuItemID(submenu, 0), 0);
				}

				return 0;
			}

		case WM_CLOSE:
			// Only accept WM_CLOSE if the logged on user has AllowShutdown set
			if (!_this->m_properties.AllowShutdown())
				return 0;
			RestoreWallpaper();
			break;

		case WM_DESTROY:
			vnclog.Print(LL_INTINFO, VNCLOG("quitting from WM_DESTROY"));

			_this->StopTimerAutoExitAfterLastViewer(hwnd);
			_this->StopTimerAutoExitIfNoViewer(hwnd);
			_this->StopTimerFlashTrayIcon(hwnd);

			PostQuitMessage(0);
			return 0;

		default:
			if (iMsg == MENU_ABOUTBOX_SHOW)
			{
				// External request to show our About dialog
				PostMessage(hwnd, WM_COMMAND, MAKELONG(ID_ABOUT, 0), 0);
				return 0;
			}

			// Process FileTransfer asynchronous Send Packet Message
			if (iMsg == FileTransferSendPacketMessage) 
			{
			  vncClient* pClient = (vncClient*) wParam;
			  if (_this->m_server->IsClient(pClient)) pClient->SendFileChunk();
			}
			break;
	}

	// Message not recognised
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void vncMenu::startServerPolling()
{
	_ASSERT(m_runMode == VNC_RUN_MODE_HELPER_TRAY);

	SetTimer(m_nid.hWnd, TIMER_ID_TRAY_POLL_SERVICE, TIMER_INTERVAL_TRAY_POLL_SERVICE, NULL);
}

void vncMenu::performServerPolling()
{
	CPipe pipeToServer(INDEX_PIPE_HELPER);
	char message[IPC_TRAY_MAX_DATA];
	int len;
	CPipe::CIpcMessage ipcMessage;
	bool result;
	static int currentPoll = 0;
	bool bNeedUpdate = false;

	_ASSERT(m_runMode == VNC_RUN_MODE_HELPER_TRAY);

	len = _snprintf_s(message, IPC_TRAY_MAX_DATA, IPC_TRAY_MAX_DATA - 1, "%s %d", __FUNCTION__, ++currentPoll);
	pipeToServer.buildIpcMessage(CPipe::IPC_MESSAGE_GET_TRAY_INFO, len, message, ipcMessage);
	result = pipeToServer.TalkToPeer(&ipcMessage, INDEX_PIPE_HELPER);
	if (result)
	{
		ConnInfo *pNewConnInfo = (ConnInfo *)(&ipcMessage.data);
		if (strcmp(pNewConnInfo->szUser, m_connInfo.szUser) ||
			strcmp(pNewConnInfo->szPasswd, m_connInfo.szPasswd) ||
			strcmp(pNewConnInfo->szAddress, m_connInfo.szAddress))
		{
			memcpy(&m_connInfo, ipcMessage.data, sizeof(ConnInfo));
			SendTrayMsg(NIM_MODIFY, 0);
		}
	}
	else
	{
		static int skipDisconnected = 3;

		m_connInfo.szUser[0] = '\0';
		if (--skipDisconnected <= 0)
		{
			skipDisconnected = 3;
			SendTrayMsg(NIM_MODIFY, 0);
		}
	}
}

void vncMenu::performServerDisConnect(bool bConect)
{
	CPipe pipeToServer(INDEX_PIPE_HELPER);
	char message[IPC_TRAY_MAX_DATA];
	int len;
	CPipe::CIpcMessage ipcMessage;
	bool result;
	static int currentPoll = 0;
	bool bNeedUpdate = false;

	_ASSERT(m_runMode == VNC_RUN_MODE_HELPER_TRAY);

	len = _snprintf_s(message, IPC_TRAY_MAX_DATA, IPC_TRAY_MAX_DATA - 1, "%s %d", __FUNCTION__, ++currentPoll);
	pipeToServer.buildIpcMessage(bConect ? CPipe::IPC_MESSAGE_CLIENT_ASK_CONNECT : CPipe::IPC_MESSAGE_CLIENT_ASK_DISCONNECT, 0, "", ipcMessage);
	result = pipeToServer.TalkToPeer(&ipcMessage, INDEX_PIPE_HELPER);
	if (result)
	{
		performServerPolling();
	}
}

