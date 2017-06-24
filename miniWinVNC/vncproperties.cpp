//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//  Copyright (C) 2000-2002 Const Kaplinsky. All Rights Reserved.
//  Copyright (C) 2002 TightVNC. All Rights Reserved.
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


// vncProperties.cpp

// Implementation of the Properties dialog!

#include "stdhdrs.h"
#include "lmcons.h"
#include "vncService.h"

#include "WinVNC.h"
#include "vncProperties.h"
#include "vncServer.h"
#include "vncPasswd.h"
#include "vncOSVersion.h"
#include "../Common/InterfaceDllProxyInfo.h"
#include "vncmenu.h"

#include "localization.h" // ACT : Add localization on messages

//extern HINSTANCE g_hInst;

bool RunningAsAdministrator();

// Marscha@2004 - authSSP: Function pointer for dyn. linking
typedef void (*vncEditSecurityFn) (HWND hwnd, HINSTANCE hInstance);
vncEditSecurityFn vncEditSecurity = 0;

// Constructor & Destructor
vncProperties::vncProperties()
{
	m_alloweditclients = TRUE;
	m_allowproperties = TRUE;
	m_allowshutdown = TRUE;
//	m_dlgvisible = FALSE;
	m_usersettings = TRUE;
	Lock_service_helper = true;
}

vncProperties::~vncProperties()
{
}

// Initialisation
int vncProperties::Init(vncServer *server)
{
	// Save the server pointer
	m_server = server;
	
	// Load the settings from the registry
	int res = Load(TRUE);
/*
	// If the password is empty then always show a dialog
	char passwd[MAXPWLEN];
	m_server->GetPassword(passwd);
	{
	    vncPasswd::ToText plain(passwd);
	    if (strlen(plain) == 0)
			 if (!m_allowproperties || !RunningAsAdministrator ()) {
				if(m_server->AuthRequired()) {
					MessageBox(NULL, sz_ID_NO_PASSWD_NO_OVERRIDE_ERR,
								sz_ID_WINVNC_ERROR,
								MB_OK | MB_ICONSTOP);
					PostQuitMessage(0);
				} else {
					MessageBox(NULL, sz_ID_NO_PASSWD_NO_OVERRIDE_WARN,
								sz_ID_WINVNC_ERROR,
								MB_OK | MB_ICONEXCLAMATION);
				}
			} else {
				// If null passwords are not allowed, ensure that one is entered!
				if (m_server->AuthRequired()) {
					char username[UNLEN+1];
					if (!vncService::CurrentUser(username, sizeof(username)))
						return FALSE;
					if (strcmp(username, "") == 0) {
						Lock_service_helper=true;
						MessageBox(NULL, sz_ID_NO_PASSWD_NO_LOGON_WARN,
									sz_ID_WINVNC_ERROR,
									MB_OK | MB_ICONEXCLAMATION);
//						ShowAdmin(TRUE, FALSE);
						Lock_service_helper=false;
					} else {
//						ShowAdmin(TRUE, TRUE);
					}
				}
			}
	}
	Lock_service_helper=false;
*/
	return res;
}

/*

// Dialog box handling functions
void
vncProperties::ShowAdmin(BOOL show, BOOL usersettings)
{
//	if (Lock_service_helper) return;
	if (!m_allowproperties) 
		return;

	if (!RunningAsAdministrator ()) 
		return;

	if (vncService::Get_Fus()==1) 
		return;

	if (vncService::RunningAsService()) 
		usersettings=false;

	m_usersettings=usersettings;
	if (show)
	{
/*
		// Verify that we know who is logged on
		if (usersettings) 
		{
			char username[UNLEN+1];
			if (!vncService::CurrentUser(username, sizeof(username)))
				return;
			if (strcmp(username, "") == 0) 
			{
				MessageBox(NULL, sz_ID_NO_CURRENT_USER_ERR, sz_ID_WINVNC_ERROR, MB_OK | MB_ICONEXCLAMATION);
				return;
			}
		} 
		else 
		{
			// We're trying to edit the default local settings - verify that we can
			HKEY hkLocal, hkDefault;
			BOOL canEditDefaultPrefs = 1;
			DWORD dw;
			if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
				WINVNC_REGISTRY_KEY,
				0, REG_NONE, REG_OPTION_NON_VOLATILE,
				KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
				canEditDefaultPrefs = 0;
			else if (RegCreateKeyEx(hkLocal,
				"Default",
				0, REG_NONE, REG_OPTION_NON_VOLATILE,
				KEY_WRITE | KEY_READ, NULL, &hkDefault, &dw) != ERROR_SUCCESS)
				canEditDefaultPrefs = 0;
			if (hkLocal) RegCloseKey(hkLocal);
			if (hkDefault) RegCloseKey(hkDefault);

			if (!canEditDefaultPrefs) 
			{
				MessageBox(NULL, sz_ID_CANNOT_EDIT_DEFAULT_PREFS, sz_ID_WINVNC_ERROR, MB_OK | MB_ICONEXCLAMATION);
				return;
			}
		}
*/
		// Now, if the dialog is not already displayed, show it!
/*		if (!m_dlgvisible)
		{
			if (usersettings)
				vnclog.Print(LL_INTINFO, VNCLOG("show per-user Properties\n"));
			else
				vnclog.Print(LL_INTINFO, VNCLOG("show default system Properties\n"));

			// Load in the settings relevant to the user or system
			//Load(usersettings);
			m_usersettings=usersettings;

			for (;;)
			{
				m_returncode_valid = FALSE;

				// Do the dialog box
				int result = DialogBoxParam(hAppInstance,
				    MAKEINTRESOURCE(IDD_ADMIN_PROP), 
				    NULL,
				    (DLGPROC) DialogProc,
				    (LONG) this);

				if (!m_returncode_valid)
				    result = IDCANCEL;

				vnclog.Print(LL_INTINFO, VNCLOG("dialog result = %d\n"), result);

				if (result == -1)
				{
					// Dialog box failed, so quit
					PostQuitMessage(0);
					return;
				}

				// We're allowed to exit if the password is not empty
				char passwd[MAXPWLEN];
				m_server->GetPassword(passwd);
				{
				    vncPasswd::ToText plain(passwd);
				    //if ((strlen(plain) != 0) || !m_server->AuthRequired())
					break;
				}

				vnclog.Print(LL_INTERR, VNCLOG("warning - empty password\n"));

				// The password is empty, so if OK was used then redisplay the box,
				// otherwise, if CANCEL was used, close down WinVNC
				if (result == IDCANCEL)
				{
				    vnclog.Print(LL_INTERR, VNCLOG("no password - QUITTING\n"));
				    PostQuitMessage(0);
				    return;
				}

				// If we reached here then OK was used & there is no password!
				int result2 = MessageBox(NULL, sz_ID_NO_PASSWORD_WARN,
				    sz_ID_WINVNC_WARNIN, MB_OK | MB_ICONEXCLAMATION);

				omni_thread::sleep(4);
			}

			// Load in all the settings
			Load(TRUE);
		}
	}
}
*/

extern void SetEchowareUsing(bool bUse);
extern bool GetEchowareUsing();

/*
BOOL CALLBACK
vncProperties::DialogProc(HWND hwnd,
						  UINT uMsg,
						  WPARAM wParam,
						  LPARAM lParam )
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncProperties *_this = (vncProperties *) GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg)
	{

	case WM_INITDIALOG:
		{

			HWND hCheckEchoWare = ::GetDlgItem( hwnd, IDC_CHECK_ECHOWARE);
			SendMessage(hCheckEchoWare, BM_SETCHECK, GetEchowareUsing(), 0);

			vnclog.Print(LL_INTINFO, VNCLOG("sINITDOALOG properties\n"));
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			_this = (vncProperties *) lParam;
			_this->Load(_this->m_usersettings);
			_this->m_dlgvisible = TRUE;

			// Set the dialog box's title to indicate which Properties we're editting
			if (_this->m_usersettings) {
				SetWindowText(hwnd, sz_ID_CURRENT_USER_PROP);
			} else {
				SetWindowText(hwnd, sz_ID_DEFAULT_SYST_PROP);
			}


			// Initialise the properties controls
			HWND hConnectSock = GetDlgItem(hwnd, IDC_CONNECT_SOCK);

			// Tight 1.2.7 method
			BOOL bConnectSock = _this->m_server->SockConnected();
			SendMessage(hConnectSock, BM_SETCHECK, bConnectSock, 0);

			// Set the content of the password field to a predefined string.
//		    SetDlgItemText(hwnd, IDC_PASSWORD, "~~~~~~~~");
//			EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), bConnectSock);
			EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), false);

			// Set the initial keyboard focus
			if (bConnectSock)
			{
				//SetFocus(GetDlgItem(hwnd, IDC_PASSWORD));
				//SendDlgItemMessage(hwnd, IDC_PASSWORD, EM_SETSEL, 0, (LPARAM)-1);
			}
			else
				SetFocus(hConnectSock);
			// Set display/ports settings
			_this->InitPortSettings(hwnd);

			HWND hConnectHTTP = GetDlgItem(hwnd, IDC_CONNECT_HTTP);
			SendMessage(hConnectHTTP,
				BM_SETCHECK,
				_this->m_server->HTTPConnectEnabled(),
				0);
			HWND hConnectXDMCP = GetDlgItem(hwnd, IDC_CONNECT_XDMCP);
			SendMessage(hConnectXDMCP,
				BM_SETCHECK,
				_this->m_server->XDMCPConnectEnabled(),
				0);

			// Modif sf@2002
		   HWND hSingleWindow = GetDlgItem(hwnd, IDC_SINGLE_WINDOW);
           SendMessage(hSingleWindow, BM_SETCHECK, _this->m_server->SingleWindow(), 0);

		   // handler to get window name
           HWND hWindowName = GetDlgItem(hwnd, IDC_NAME_APPLI);
           if ( _this->m_server->GetWindowName() != NULL)
		   {
               SetDlgItemText(hwnd, IDC_NAME_APPLI,_this->m_server->GetWindowName());
           }
           EnableWindow(hWindowName, _this->m_server->SingleWindow());

		   // Modif sf@2002 - v1.1.0
		   HWND hFileTransfer = GetDlgItem(hwnd, IDC_FILETRANSFER);
           SendMessage(hFileTransfer, BM_SETCHECK, _this->m_server->FileTransferEnabled(), 0);

		   HWND hFileTransferUserImp = GetDlgItem(hwnd, IDC_FTUSERIMPERSONATION_CHECK);
           SendMessage(hFileTransferUserImp, BM_SETCHECK, _this->m_server->FTUserImpersonation(), 0);
		   
		   HWND hBlank = GetDlgItem(hwnd, IDC_BLANK);
           SendMessage(hBlank, BM_SETCHECK, _this->m_server->BlankMonitorEnabled(), 0);

		   HWND hAlpha = GetDlgItem(hwnd, IDC_ALPHA);
           SendMessage(hAlpha, BM_SETCHECK, _this->m_server->CaptureAlphaBlending(), 0);
		   HWND hAlphab = GetDlgItem(hwnd, IDC_ALPHABLACK);
           SendMessage(hAlphab, BM_SETCHECK, _this->m_server->BlackAlphaBlending(), 0);
		   
		   HWND hLoopback = GetDlgItem(hwnd, IDC_ALLOWLOOPBACK);
		   BOOL fLoopback = _this->m_server->LoopbackOk();
		   SendMessage(hLoopback, BM_SETCHECK, fLoopback, 0);

		   HWND hLoopbackonly = GetDlgItem(hwnd, IDC_LOOPBACKONLY);
		   BOOL fLoopbackonly = _this->m_server->LoopbackOnly();
		   SendMessage(hLoopbackonly, BM_SETCHECK, fLoopbackonly, 0);

		   HWND hTrayicon = GetDlgItem(hwnd, IDC_DISABLETRAY);
		   BOOL fTrayicon = _this->m_server->GetDisableTrayIcon();
		   SendMessage(hTrayicon, BM_SETCHECK, fTrayicon, 0);

		   HWND hAllowshutdown = GetDlgItem(hwnd, IDC_ALLOWSHUTDOWN);
		   SendMessage(hAllowshutdown, BM_SETCHECK, !_this->m_allowshutdown , 0);

		   HWND hm_alloweditclients = GetDlgItem(hwnd, IDC_ALLOWEDITCLIENTS);
		   SendMessage(hm_alloweditclients, BM_SETCHECK, !_this->m_alloweditclients , 0);
		   _this->m_server->SetAllowEditClients(_this->m_alloweditclients);
		   

		   if (vnclog.GetMode() >= 2)
			   CheckDlgButton(hwnd, IDC_LOG, BST_CHECKED);
		   else
			   CheckDlgButton(hwnd, IDC_LOG, BST_UNCHECKED);
		   
			// Marscha@2004 - authSSP: moved MS-Logon checkbox back to admin props page
			// added New MS-Logon checkbox
			// only enable New MS-Logon checkbox and Configure MS-Logon groups when MS-Logon
			// is checked.
			HWND hMSLogon = GetDlgItem(hwnd, IDC_MSLOGON_CHECKD);
			SendMessage(hMSLogon, BM_SETCHECK, _this->m_server->MSLogonRequired(), 0);

			HWND hNewMSLogon = GetDlgItem(hwnd, IDC_NEW_MSLOGON);
			SendMessage(hNewMSLogon, BM_SETCHECK, _this->m_server->GetNewMSLogon(), 0);

			EnableWindow(GetDlgItem(hwnd, IDC_NEW_MSLOGON), _this->m_server->MSLogonRequired());
			EnableWindow(GetDlgItem(hwnd, IDC_MSLOGON), _this->m_server->MSLogonRequired());
			// Marscha@2004 - authSSP: end of change

		   SetDlgItemInt(hwnd, IDC_SCALE, _this->m_server->GetDefaultScale(), false);

		   
		   // Remote input settings
			HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
			SendMessage(hEnableRemoteInputs,
				BM_SETCHECK,
				!(_this->m_server->RemoteInputsEnabled()),
				0);

			// Local input settings
			HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
			SendMessage(hDisableLocalInputs,
				BM_SETCHECK,
				_this->m_server->LocalInputsDisabled(),
				0);

			// Remove the wallpaper
			HWND hRemoveWallpaper = GetDlgItem(hwnd, IDC_REMOVE_WALLPAPER);
			SendMessage(hRemoveWallpaper,
				BM_SETCHECK,
				_this->m_server->RemoveWallpaperEnabled(),
				0);

			// Lock settings
			HWND hLockSetting;
			switch (_this->m_server->LockSettings()) {
			case 1:
				hLockSetting = GetDlgItem(hwnd, IDC_LOCKSETTING_LOCK);
				break;
			case 2:
				hLockSetting = GetDlgItem(hwnd, IDC_LOCKSETTING_LOGOFF);
				break;
			default:
				hLockSetting = GetDlgItem(hwnd, IDC_LOCKSETTING_NOTHING);
			};
			SendMessage(hLockSetting,
				BM_SETCHECK,
				TRUE,
				0);

			HWND hmvSetting;
			switch (_this->m_server->ConnectPriority()) {
			case 0:
				hmvSetting = GetDlgItem(hwnd, IDC_MV1);
				break;
			case 1:
				hmvSetting = GetDlgItem(hwnd, IDC_MV2);
				break;
			case 2:
				hmvSetting = GetDlgItem(hwnd, IDC_MV3);
				break;
			case 3:
				hmvSetting = GetDlgItem(hwnd, IDC_MV4);
				break;
			};
			SendMessage(hmvSetting,
				BM_SETCHECK,
				TRUE,
				0);


			HWND hQuerySetting;
			switch (_this->m_server->QueryAccept()) {
			case 0:
				hQuerySetting = GetDlgItem(hwnd, IDC_DREFUSE);
				break;
			case 1:
				hQuerySetting = GetDlgItem(hwnd, IDC_DACCEPT);
				break;
			default:
				hQuerySetting = GetDlgItem(hwnd, IDC_DREFUSE);
			};
			SendMessage(hQuerySetting,
				BM_SETCHECK,
				TRUE,
				0);

			// sf@2002 - List available DSM Plugins
			HWND hPlugins = GetDlgItem(hwnd, IDC_PLUGINS_COMBO);
			int nPlugins = _this->m_server->GetDSMPluginPointer()->ListPlugins(hPlugins);
			if (!nPlugins) 
			{
				SendMessage(hPlugins, CB_ADDSTRING, 0, (LPARAM) sz_ID_NO_PLUGIN_DETECT);
				SendMessage(hPlugins, CB_SETCURSEL, 0, 0);
			}
			else
				SendMessage(hPlugins, CB_SELECTSTRING, 0, (LPARAM)_this->m_server->GetDSMPluginName());

			// Modif sf@2002
			HWND hUsePlugin = GetDlgItem(hwnd, IDC_PLUGIN_CHECK);
			SendMessage(hUsePlugin,
				BM_SETCHECK,
				_this->m_server->IsDSMPluginEnabled(),
				0);
			HWND hButton = GetDlgItem(hwnd, IDC_PLUGIN_BUTTON);
			EnableWindow(hButton, _this->m_server->IsDSMPluginEnabled());

			// Query window option - Taken from TightVNC advanced properties 
			HWND hQuery = GetDlgItem(hwnd, IDQUERY);
			BOOL queryEnabled = (_this->m_server->QuerySetting() == 4);
			SendMessage(hQuery, BM_SETCHECK, queryEnabled, 0);
			HWND hQueryTimeout = GetDlgItem(hwnd, IDQUERYTIMEOUT);
			EnableWindow(hQueryTimeout, queryEnabled);
			char timeout[128];
			UINT t = _this->m_server->QueryTimeout();
			sprintf(timeout, "%d", (int)t);
		    SetDlgItemText(hwnd, IDQUERYTIMEOUT, (const char *) timeout);

			SetForegroundWindow(hwnd);

			return FALSE; // Because we've set the focus
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDOK:
		case IDC_APPLY:
			{

				// Save echoWare.dll using
				HWND hCheckEchoWare = ::GetDlgItem( hwnd, IDC_CHECK_ECHOWARE);
				if (hCheckEchoWare)
				{
					bool bUse =
						(::SendMessage( hCheckEchoWare, BM_GETCHECK, 0, 0) != 0) ? true:false;
					SetEchowareUsing(bUse);
				}


				// Save the password
/*
				char passwd[MAXPWLEN+1];
				// TightVNC method
				int len = GetDlgItemText(hwnd, IDC_PASSWORD, (LPSTR) &passwd, MAXPWLEN+1);
				if (strcmp(passwd, "~~~~~~~~") != 0) 
				{
					if (len == 0)
					{
						vncPasswd::FromClear crypt;
						_this->m_server->SetPassword(crypt);
					}
					else
					{
						vncPasswd::FromText crypt(passwd);
						_this->m_server->SetPassword(crypt);
					}
				}
*/
				// Save the new settings to the server
/*				int state = SendDlgItemMessage(hwnd, IDC_PORTNO_AUTO, BM_GETCHECK, 0, 0);
				_this->m_server->SetAutoPortSelect(state == BST_CHECKED);

				// Save port numbers if we're not auto selecting
				if (!_this->m_server->AutoPortSelect()) {
					if ( SendDlgItemMessage(hwnd, IDC_SPECDISPLAY,
											BM_GETCHECK, 0, 0) == BST_CHECKED ) {
						// Display number was specified
						BOOL ok;
						UINT display = GetDlgItemInt(hwnd, IDC_DISPLAYNO, &ok, TRUE);
						if (ok)
							_this->m_server->SetPorts(DISPLAY_TO_PORT(display),
													  DISPLAY_TO_HPORT(display));
					} else {
						// Assuming that port numbers were specified
						BOOL ok1, ok2;
						UINT port_rfb = GetDlgItemInt(hwnd, IDC_PORTRFB, &ok1, TRUE);
						UINT port_http = GetDlgItemInt(hwnd, IDC_PORTHTTP, &ok2, TRUE);
						if (ok1 && ok2)
							_this->m_server->SetPorts(port_rfb, port_http);
					}
				}
				HWND hConnectSock = GetDlgItem(hwnd, IDC_CONNECT_SOCK);
				_this->m_server->SockConnect(
					SendMessage(hConnectSock, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				// Update display/port controls on pressing the "Apply" button
				if (LOWORD(wParam) == IDC_APPLY)
					_this->InitPortSettings(hwnd);

				

				HWND hConnectHTTP = GetDlgItem(hwnd, IDC_CONNECT_HTTP);
				_this->m_server->EnableHTTPConnect(
					SendMessage(hConnectHTTP, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hConnectXDMCP = GetDlgItem(hwnd, IDC_CONNECT_XDMCP);
				_this->m_server->EnableXDMCPConnect(
					SendMessage(hConnectXDMCP, BM_GETCHECK, 0, 0) == BST_CHECKED
					);
				
				// Remote input stuff
				HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
				_this->m_server->EnableRemoteInputs(
					SendMessage(hEnableRemoteInputs, BM_GETCHECK, 0, 0) != BST_CHECKED
					);

				// Local input stuff
				HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
				_this->m_server->DisableLocalInputs(
					SendMessage(hDisableLocalInputs, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				// Wallpaper handling
				HWND hRemoveWallpaper = GetDlgItem(hwnd, IDC_REMOVE_WALLPAPER);
				_this->m_server->EnableRemoveWallpaper(
					SendMessage(hRemoveWallpaper, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				// Lock settings handling
				if (SendMessage(GetDlgItem(hwnd, IDC_LOCKSETTING_LOCK), BM_GETCHECK, 0, 0)
					== BST_CHECKED) {
					_this->m_server->SetLockSettings(1);
				} else if (SendMessage(GetDlgItem(hwnd, IDC_LOCKSETTING_LOGOFF), BM_GETCHECK, 0, 0)
					== BST_CHECKED) {
					_this->m_server->SetLockSettings(2);
				} else {
					_this->m_server->SetLockSettings(0);
				}

				if (SendMessage(GetDlgItem(hwnd, IDC_DREFUSE), BM_GETCHECK, 0, 0)
					== BST_CHECKED) {
					_this->m_server->SetQueryAccept(0);
				} else if (SendMessage(GetDlgItem(hwnd, IDC_DACCEPT), BM_GETCHECK, 0, 0)
					== BST_CHECKED) {
					_this->m_server->SetQueryAccept(1);
				} 

				if (SendMessage(GetDlgItem(hwnd, IDC_MV1), BM_GETCHECK, 0, 0)
					== BST_CHECKED) {
					_this->m_server->SetConnectPriority(0);
				} else if (SendMessage(GetDlgItem(hwnd, IDC_MV2), BM_GETCHECK, 0, 0)
					== BST_CHECKED) {
					_this->m_server->SetConnectPriority(1);
				} 
				 else if (SendMessage(GetDlgItem(hwnd, IDC_MV3), BM_GETCHECK, 0, 0)
					== BST_CHECKED) {
					_this->m_server->SetConnectPriority(2);
				} else if (SendMessage(GetDlgItem(hwnd, IDC_MV4), BM_GETCHECK, 0, 0)
					== BST_CHECKED) {
					_this->m_server->SetConnectPriority(3);
				} 

				

				// Modif sf@2002
				HWND hSingleWindow = GetDlgItem(hwnd, IDC_SINGLE_WINDOW);
				_this->m_server->SingleWindow(SendMessage(hSingleWindow, BM_GETCHECK, 0, 0) == BST_CHECKED);

				char szName[32];
				if (GetDlgItemText(hwnd, IDC_NAME_APPLI, (LPSTR) szName, 32) == 0)
				{
					vnclog.Print(LL_INTINFO,VNCLOG("Error while reading Window Name %d \n"), GetLastError());
				}
				else
				{
					_this->m_server->SetSingleWindowName(szName);
				}
				
				// Modif sf@2002 - v1.1.0
				HWND hFileTransfer = GetDlgItem(hwnd, IDC_FILETRANSFER);
				_this->m_server->EnableFileTransfer(SendMessage(hFileTransfer, BM_GETCHECK, 0, 0) == BST_CHECKED);

				HWND hFileTransferUserImp = GetDlgItem(hwnd, IDC_FTUSERIMPERSONATION_CHECK);
				_this->m_server->FTUserImpersonation(SendMessage(hFileTransferUserImp, BM_GETCHECK, 0, 0) == BST_CHECKED);

				HWND hBlank = GetDlgItem(hwnd, IDC_BLANK);
				_this->m_server->BlankMonitorEnabled(SendMessage(hBlank, BM_GETCHECK, 0, 0) == BST_CHECKED);
				HWND hAlpha = GetDlgItem(hwnd, IDC_ALPHA);
				_this->m_server->CaptureAlphaBlending(SendMessage(hAlpha, BM_GETCHECK, 0, 0) == BST_CHECKED);
				HWND hAlphab = GetDlgItem(hwnd, IDC_ALPHABLACK);
				_this->m_server->BlackAlphaBlending(SendMessage(hAlphab, BM_GETCHECK, 0, 0) == BST_CHECKED);

				_this->m_server->SetLoopbackOk(IsDlgButtonChecked(hwnd, IDC_ALLOWLOOPBACK));
				_this->m_server->SetLoopbackOnly(IsDlgButtonChecked(hwnd, IDC_LOOPBACKONLY));

				_this->m_server->SetDisableTrayIcon(IsDlgButtonChecked(hwnd, IDC_DISABLETRAY));
				_this->m_allowshutdown=!IsDlgButtonChecked(hwnd, IDC_ALLOWSHUTDOWN);
				_this->m_alloweditclients=!IsDlgButtonChecked(hwnd, IDC_ALLOWEDITCLIENTS);
				_this->m_server->SetAllowEditClients(_this->m_alloweditclients);
				if (IsDlgButtonChecked(hwnd, IDC_LOG))
				{
					vnclog.SetMode(2);
					vnclog.SetLevel(10);
				}
				else
				{
					vnclog.SetMode(0);
				}
				// Modif sf@2002 - v1.1.0
				// Marscha@2004 - authSSP: moved MS-Logon checkbox back to admin props page
				// added New MS-Logon checkbox
				HWND hMSLogon = GetDlgItem(hwnd, IDC_MSLOGON_CHECKD);
				_this->m_server->RequireMSLogon(SendMessage(hMSLogon, BM_GETCHECK, 0, 0) == BST_CHECKED);
				
				HWND hNewMSLogon = GetDlgItem(hwnd, IDC_NEW_MSLOGON);
				_this->m_server->SetNewMSLogon(SendMessage(hNewMSLogon, BM_GETCHECK, 0, 0) == BST_CHECKED);
				// Marscha@2004 - authSSP: end of change

				int nDScale = GetDlgItemInt(hwnd, IDC_SCALE, NULL, FALSE);
				if (nDScale < 1 || nDScale > 9) nDScale = 1;
				_this->m_server->SetDefaultScale(nDScale);
				
				// sf@2002 - DSM Plugin loading
				// If Use plugin is checked, load the plugin if necessary
				HWND hPlugin = GetDlgItem(hwnd, IDC_PLUGIN_CHECK);
				if (SendMessage(hPlugin, BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					TCHAR szPlugin[_MAX_PATH];
					GetDlgItemText(hwnd, IDC_PLUGINS_COMBO, szPlugin, _MAX_PATH);
					_this->m_server->SetDSMPluginName(szPlugin);
					_this->m_server->EnableDSMPlugin(true);
				}
				else // If Use plugin unchecked but the plugin is loaded, unload it
				{
					_this->m_server->EnableDSMPlugin(false);
					if (_this->m_server->GetDSMPluginPointer()->IsLoaded())
					{
						_this->m_server->GetDSMPluginPointer()->UnloadPlugin();
						_this->m_server->GetDSMPluginPointer()->SetEnabled(false);
					}	
				}

				// Query Window options - Taken from TightVNC advanced properties
				char timeout[256];
				if (GetDlgItemText(hwnd, IDQUERYTIMEOUT, (LPSTR) &timeout, 256) == 0)
				    _this->m_server->SetQueryTimeout(atoi(timeout));
				else
				    _this->m_server->SetQueryTimeout(atoi(timeout));
				HWND hQuery = GetDlgItem(hwnd, IDQUERY);
				_this->m_server->SetQuerySetting((SendMessage(hQuery, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 4 : 2);

				// And to the registry

				_this->Save();


				// Was ok pressed?
				if (LOWORD(wParam) == IDOK)
				{
					// Yes, so close the dialog
					vnclog.Print(LL_INTINFO, VNCLOG("enddialog (OK)\n"));

					_this->m_returncode_valid = TRUE;

					EndDialog(hwnd, IDOK);
					_this->m_dlgvisible = FALSE;
				}

				_this->m_server->SetHookings();

				return TRUE;
			}

		// Modif sf@2002
		 case IDC_SINGLE_WINDOW:
			 {
				 HWND hSingleWindow = GetDlgItem(hwnd, IDC_SINGLE_WINDOW);
				 BOOL fSingleWindow = (SendMessage(hSingleWindow, BM_GETCHECK,0, 0) == BST_CHECKED);
				 HWND hWindowName   = GetDlgItem(hwnd, IDC_NAME_APPLI);
				 EnableWindow(hWindowName, fSingleWindow);
			 }
			 return TRUE;

		case IDCANCEL:
			vnclog.Print(LL_INTINFO, VNCLOG("enddialog (CANCEL)\n"));

			_this->m_returncode_valid = TRUE;

			EndDialog(hwnd, IDCANCEL);
			_this->m_dlgvisible = FALSE;
			return TRUE;

		case IDC_CONNECT_SOCK:
			// TightVNC 1.2.7 method
			// The user has clicked on the socket connect tickbox
			{
				BOOL bConnectSock =
					(SendDlgItemMessage(hwnd, IDC_CONNECT_SOCK,
										BM_GETCHECK, 0, 0) == BST_CHECKED);

				EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), false );//bConnectSock);

				HWND hPortNoAuto = GetDlgItem(hwnd, IDC_PORTNO_AUTO);
				EnableWindow(hPortNoAuto, bConnectSock);
				HWND hSpecDisplay = GetDlgItem(hwnd, IDC_SPECDISPLAY);
				EnableWindow(hSpecDisplay, bConnectSock);
				HWND hSpecPort = GetDlgItem(hwnd, IDC_SPECPORT);
				EnableWindow(hSpecPort, bConnectSock);

				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), bConnectSock &&
					(SendMessage(hSpecDisplay, BM_GETCHECK, 0, 0) == BST_CHECKED));
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), bConnectSock &&
					(SendMessage(hSpecPort, BM_GETCHECK, 0, 0) == BST_CHECKED));
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), bConnectSock &&
					(SendMessage(hSpecPort, BM_GETCHECK, 0, 0) == BST_CHECKED));
			}
			// RealVNC method
			/*
			// The user has clicked on the socket connect tickbox
			{
				HWND hConnectSock = GetDlgItem(hwnd, IDC_CONNECT_SOCK);
				BOOL connectsockon =
					(SendMessage(hConnectSock, BM_GETCHECK, 0, 0) == BST_CHECKED);

				HWND hAutoDisplayNo = GetDlgItem(hwnd, IDC_AUTO_DISPLAY_NO);
				EnableWindow(hAutoDisplayNo, connectsockon);
			
				HWND hPortNo = GetDlgItem(hwnd, IDC_PORTNO);
				EnableWindow(hPortNo, connectsockon
					&& (SendMessage(hAutoDisplayNo, BM_GETCHECK, 0, 0) != BST_CHECKED));
			
				HWND hPassword = GetDlgItem(hwnd, IDC_PASSWORD);
				EnableWindow(hPassword, connectsockon);
			}
			*/
/*			return TRUE;

		// TightVNC 1.2.7 method
		case IDC_PORTNO_AUTO:
			{
				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), FALSE);

				SetDlgItemText(hwnd, IDC_DISPLAYNO, "");
				SetDlgItemText(hwnd, IDC_PORTRFB, "");
				SetDlgItemText(hwnd, IDC_PORTHTTP, "");
			}
			return TRUE;

		case IDC_SPECDISPLAY:
			{
				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), FALSE);

				int display = PORT_TO_DISPLAY(_this->m_server->GetPort());
				if (display < 0 || display > 99)
					display = 0;
				SetDlgItemInt(hwnd, IDC_DISPLAYNO, display, FALSE);
				SetDlgItemInt(hwnd, IDC_PORTRFB, _this->m_server->GetPort(), FALSE);
				SetDlgItemInt(hwnd, IDC_PORTHTTP, _this->m_server->GetHttpPort(), FALSE);

				SetFocus(GetDlgItem(hwnd, IDC_DISPLAYNO));
				SendDlgItemMessage(hwnd, IDC_DISPLAYNO, EM_SETSEL, 0, (LPARAM)-1);
			}
			return TRUE;

		case IDC_SPECPORT:
			{
				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), TRUE);

				int d1 = PORT_TO_DISPLAY(_this->m_server->GetPort());
				int d2 = HPORT_TO_DISPLAY(_this->m_server->GetHttpPort());
				if (d1 == d2 && d1 >= 0 && d1 <= 99) {
					SetDlgItemInt(hwnd, IDC_DISPLAYNO, d1, FALSE);
				} else {
					SetDlgItemText(hwnd, IDC_DISPLAYNO, "");
				}
				SetDlgItemInt(hwnd, IDC_PORTRFB, _this->m_server->GetPort(), FALSE);
				SetDlgItemInt(hwnd, IDC_PORTHTTP, _this->m_server->GetHttpPort(), FALSE);

				SetFocus(GetDlgItem(hwnd, IDC_PORTRFB));
				SendDlgItemMessage(hwnd, IDC_PORTRFB, EM_SETSEL, 0, (LPARAM)-1);
			}
			return TRUE;

		// RealVNC method
		/*
		case IDC_AUTO_DISPLAY_NO:
			// User has toggled the Auto Port Select feature.
			// If this is in use, then we don't allow the Display number field
			// to be modified!
			{
				// Get the auto select button
				HWND hPortNoAuto = GetDlgItem(hwnd, IDC_AUTO_DISPLAY_NO);

				// Should the portno field be modifiable?
				BOOL enable = SendMessage(hPortNoAuto, BM_GETCHECK, 0, 0) != BST_CHECKED;

				// Set the state
				HWND hPortNo = GetDlgItem(hwnd, IDC_PORTNO);
				EnableWindow(hPortNo, enable);
			}
			return TRUE;
		*/

		// Query window option - Taken from TightVNC advanced properties code
/*		case IDQUERY:
			{
				HWND hQuery = GetDlgItem(hwnd, IDQUERY);
				BOOL queryon = (SendMessage(hQuery, BM_GETCHECK, 0, 0) == BST_CHECKED);
				EnableWindow(GetDlgItem(hwnd, IDQUERYTIMEOUT), queryon);
			}
			return TRUE;

		// sf@2002 - DSM Plugin
		case IDC_PLUGIN_CHECK:
			{
				HWND hUse = GetDlgItem(hwnd, IDC_PLUGIN_CHECK);
				BOOL enable = SendMessage(hUse, BM_GETCHECK, 0, 0) == BST_CHECKED;
				HWND hButton = GetDlgItem(hwnd, IDC_PLUGIN_BUTTON);
				EnableWindow(hButton, enable);
			}
			return TRUE;
			// Marscha@2004 - authSSP: moved MSLogon checkbox back to admin props page
			// Reason: Different UI for old and new mslogon group config.
		case IDC_MSLOGON_CHECKD:
			{
				BOOL bMSLogonChecked =
				(SendDlgItemMessage(hwnd, IDC_MSLOGON_CHECKD,
										BM_GETCHECK, 0, 0) == BST_CHECKED);

				EnableWindow(GetDlgItem(hwnd, IDC_NEW_MSLOGON), bMSLogonChecked);
				EnableWindow(GetDlgItem(hwnd, IDC_MSLOGON), bMSLogonChecked);

			}
			return TRUE;
		case IDC_MSLOGON:
			{
				// Marscha@2004 - authSSP: if "New MS-Logon" is checked,
				// call vncEditSecurity from SecurityEditor.dll,
				// else call "old" dialog.
				BOOL bNewMSLogonChecked =
				(SendDlgItemMessage(hwnd, IDC_NEW_MSLOGON,
										BM_GETCHECK, 0, 0) == BST_CHECKED);
				if (bNewMSLogonChecked) {
					char szCurrentDir[_MAX_PATH];
					if (GetModuleFileName(NULL, szCurrentDir, _MAX_PATH)) {
						char* p = strrchr(szCurrentDir, '\\');
						*p = '\0';
						strcat (szCurrentDir,"\\authSSP.dll");
					}
					HMODULE hModule = LoadLibrary(szCurrentDir);
					if (hModule) {
						vncEditSecurity = (vncEditSecurityFn) GetProcAddress(hModule, "vncEditSecurity");
						HRESULT hr = CoInitialize(NULL);
						vncEditSecurity(hwnd, hAppInstance);
						CoUninitialize();
						FreeLibrary(hModule);
					}
				} else { 
					// Marscha@2004 - authSSP: end of change
					_this->m_vncauth.Init(_this->m_server);
					_this->m_vncauth.Show(TRUE);
				}
			}
			return TRUE;
		case IDC_CHECKDRIVER:
			{
				CheckVideoDriver(1);
			}
			return TRUE;
		case IDC_PLUGIN_BUTTON:
			{
				HWND hPlugin = GetDlgItem(hwnd, IDC_PLUGIN_CHECK);
				if (SendMessage(hPlugin, BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					TCHAR szPlugin[_MAX_PATH];
					GetDlgItemText(hwnd, IDC_PLUGINS_COMBO, szPlugin, _MAX_PATH);
					if (!_this->m_server->GetDSMPluginPointer()->IsLoaded())
						_this->m_server->GetDSMPluginPointer()->LoadPlugin(szPlugin, false);
					else
					{
						// sf@2003 - We check if the loaded plugin is the same than
						// the currently selected one or not
						_this->m_server->GetDSMPluginPointer()->DescribePlugin();
						if (stricmp(_this->m_server->GetDSMPluginPointer()->GetPluginFileName(), szPlugin))
						{
							_this->m_server->GetDSMPluginPointer()->UnloadPlugin();
							_this->m_server->GetDSMPluginPointer()->LoadPlugin(szPlugin, false);
						}
					}
				
					if (_this->m_server->GetDSMPluginPointer()->IsLoaded())
					{
						// We don't send the password yet... no matter the plugin requires
						// it or not, we will provide it later (at plugin "real" init)
						// Knowing the environnement ("server-svc" or "server-app") right 
						// now can be usefull or even mandatory for the plugin 
						// (specific params saving and so on...)
						char szParams[32];
						strcpy(szParams, "NoPassword,");
						strcat(szParams, vncService::RunningAsService() ? "server-svc" : "server-app");
						_this->m_server->GetDSMPluginPointer()->SetPluginParams(hwnd, szParams);
					}
					else
					{
						MessageBox(NULL, 
							sz_ID_PLUGIN_NOT_LOAD, 
							sz_ID_PLUGIN_LOADIN, MB_OK | MB_ICONEXCLAMATION );
					}
				}				
				return TRUE;
			}



		}
		break;
	}
	return 0;
}

*/
/*
// TightVNC 1.2.7
// Set display/port settings to the correct state
void
vncProperties::InitPortSettings(HWND hwnd)
{
	BOOL bConnectSock = m_server->SockConnected();
	BOOL bAutoPort = m_server->AutoPortSelect();
	UINT port_rfb = m_server->GetPort();
	UINT port_http = m_server->GetHttpPort();
	int d1 = PORT_TO_DISPLAY(port_rfb);
	int d2 = HPORT_TO_DISPLAY(port_http);
	BOOL bValidDisplay = (d1 == d2 && d1 >= 0 && d1 <= 99);

	CheckDlgButton(hwnd, IDC_PORTNO_AUTO,
		(bAutoPort) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_SPECDISPLAY,
		(!bAutoPort && bValidDisplay) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_SPECPORT,
		(!bAutoPort && !bValidDisplay) ? BST_CHECKED : BST_UNCHECKED);

	EnableWindow(GetDlgItem(hwnd, IDC_PORTNO_AUTO), bConnectSock);
	EnableWindow(GetDlgItem(hwnd, IDC_SPECDISPLAY), bConnectSock);
	EnableWindow(GetDlgItem(hwnd, IDC_SPECPORT), bConnectSock);

	if (bValidDisplay) {
		SetDlgItemInt(hwnd, IDC_DISPLAYNO, d1, FALSE);
	} else {
		SetDlgItemText(hwnd, IDC_DISPLAYNO, "");
	}
	SetDlgItemInt(hwnd, IDC_PORTRFB, port_rfb, FALSE);
	SetDlgItemInt(hwnd, IDC_PORTHTTP, port_http, FALSE);

	EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO),
		bConnectSock && !bAutoPort && bValidDisplay);
	EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB),
		bConnectSock && !bAutoPort && !bValidDisplay);
	EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP),
		bConnectSock && !bAutoPort && !bValidDisplay);
}

*/
// Functions to load & save the settings
LONG
vncProperties::LoadInt(HKEY key, LPCSTR valname, LONG defval)
{
	LONG pref;
	ULONG type = REG_DWORD;
	ULONG prefsize = sizeof(pref);

	if (RegQueryValueEx(key,
		valname,
		NULL,
		&type,
		(LPBYTE) &pref,
		&prefsize) != ERROR_SUCCESS)
		return defval;

	if (type != REG_DWORD)
		return defval;

	if (prefsize != sizeof(pref))
		return defval;

	return pref;
}


void vncProperties::LoadPassword(HKEY key, char *buffer)
{
	/*
	DWORD type = REG_BINARY;
	int slen=MAXPWLEN;
	char inouttext[MAXPWLEN];

	// Retrieve the encrypted password
	if (RegQueryValueEx(key,
		"Password",
		NULL,
		&type,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen) != ERROR_SUCCESS)
		return;

	if (slen > MAXPWLEN)
		return;

	memcpy(buffer, inouttext, MAXPWLEN);
	*/

	memset(buffer, 0, MAXPWLEN);
}


char *
vncProperties::LoadString(HKEY key, LPCSTR keyname)
{
	DWORD type = REG_SZ;
	DWORD buflen = 0;
	BYTE *buffer = 0;

	// Get the length of the AuthHosts string
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		NULL,
		&buflen) != ERROR_SUCCESS)
		return 0;

	if (type != REG_SZ)
		return 0;
	buffer = new BYTE[buflen];
	if (buffer == 0)
		return 0;

	// Get the AuthHosts string data
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		buffer,
		&buflen) != ERROR_SUCCESS) {
		delete [] buffer;
		return 0;
	}

	// Verify the type
	if (type != REG_SZ) {
		delete [] buffer;
		return 0;
	}

	return (char *)buffer;
}


void
vncProperties::ResetRegistry()
{	
/*
	char username[UNLEN+1];
	HKEY hkLocal, hkLocalUser, hkDefault;
	DWORD dw;

	if (!vncService::CurrentUser((char *)&username, sizeof(username)))
		return;

	// If there is no user logged on them default to SYSTEM
	if (strcmp(username, "") == 0)
		strcpy((char *)&username, "SYSTEM");

	// Try to get the machine registry key for WinVNC
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		WINVNC_REGISTRY_KEY,
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
		{
			hkLocalUser=NULL;
			hkDefault=NULL;
			goto LABELUSERSETTINGS;
		}

	// Now try to get the per-user local key
	if (RegOpenKeyEx(hkLocal,
		username,
		0, KEY_READ,
		&hkLocalUser) != ERROR_SUCCESS)
			hkLocalUser = NULL;

	// Get the default key
	if (RegCreateKeyEx(hkLocal,
		"Default",
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ,
		NULL,
		&hkDefault,
		&dw) != ERROR_SUCCESS)
			hkDefault = NULL;

	if (hkLocalUser != NULL) 
		RegCloseKey(hkLocalUser);

	if (hkDefault != NULL) 
		RegCloseKey(hkDefault);

	if (hkLocal != NULL) 
		RegCloseKey(hkLocal);

	RegCloseKey(HKEY_LOCAL_MACHINE);

LABELUSERSETTINGS:

	if ((strcmp(username, "SYSTEM") != 0))
	{
		HKEY hkGlobalUser;
		if (RegCreateKeyEx(HKEY_CURRENT_USER,
			WINVNC_REGISTRY_KEY,
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_READ, NULL, &hkGlobalUser, &dw) == ERROR_SUCCESS)
		{
			RegCloseKey(hkGlobalUser);
			RegCloseKey(HKEY_CURRENT_USER);
		}
	}
*/
}


int vncProperties::Load(BOOL usersettings)
{
//	if (m_dlgvisible) 
//	{
//		vnclog.Print(LL_INTWARN, VNCLOG("service helper invoked while Properties panel displayed\n"));
//		return;
//	}

	ResetRegistry();

//	if (vncService::RunningAsService()) usersettings=false;
	m_usersettings=usersettings;
	
	char username[UNLEN+1];
//	HKEY hkLocal, hkLocalUser, hkDefault;
//	DWORD dw;
	
	// NEW (R3) PREFERENCES ALGORITHM
	// 1.	Look in HKEY_LOCAL_MACHINE/Software/ORL/WinVNC3/%username%
	//		for sysadmin-defined, user-specific settings.
	// 2.	If not found, fall back to %username%=Default
	// 3.	If AllowOverrides is set then load settings from
	//		HKEY_CURRENT_USER/Software/ORL/WinVNC3

	// GET THE CORRECT KEY TO READ FROM

	// Get the user name / service name
	if (!vncService::CurrentUser((char *)&username, sizeof(username)))
		return ERROR_CONNECTING_TO_PROXY;

	// If there is no user logged on them default to SYSTEM
	if (strcmp(username, "") == 0)
		strcpy_s((char *)&username, UNLEN+1, "SYSTEM");

	// Try to get the machine registry key for WinVNC
//	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
//		WINVNC_REGISTRY_KEY,
//		0, REG_NONE, REG_OPTION_NON_VOLATILE,
//		KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
//		{
//			hkLocalUser=NULL;
//			hkDefault=NULL;
//			goto LABELUSERSETTINGS;
//		}

	// Now try to get the per-user local key
//	if (RegOpenKeyEx(hkLocal,
//		username,
//		0, KEY_READ,
//		&hkLocalUser) != ERROR_SUCCESS)
//			hkLocalUser = NULL;

	// Get the default key
//	if (RegCreateKeyEx(hkLocal,
//		"Default",
//		0, REG_NONE, REG_OPTION_NON_VOLATILE,
//		KEY_READ,
//		NULL,
//		&hkDefault,
//		&dw) != ERROR_SUCCESS)
//			hkDefault = NULL;

	// LOAD THE MACHINE-LEVEL PREFS

	// Logging/debugging prefs
	vnclog.Print(LL_INTINFO, VNCLOG("loading local-only settings"));

	// Disable Tray Icon
	m_server->SetDisableTrayIcon( 0 );//LoadInt(hkLocal, "DisableTrayIcon", false));

	// Authentication required, loopback allowed, loopbackOnly

	m_server->SetLoopbackOnly( true );//LoadInt(hkLocal, "LoopbackOnly", false));

//	m_pref_RequireMSLogon=false;
//	m_pref_RequireMSLogon = 0;//LoadInt(hkLocal, "MSLogonRequired", m_pref_RequireMSLogon);
	m_server->RequireMSLogon(false);

	// Marscha@2004 - authSSP: added NewMSLogon checkbox to admin props page
//	m_pref_NewMSLogon = false;
//	m_pref_NewMSLogon = 0;//LoadInt(hkLocal, "NewMSLogon", m_pref_NewMSLogon);
	m_server->SetNewMSLogon(false);

	// sf@2003 - Moved DSM params here
//	m_pref_UseDSMPlugin=false;
//	m_pref_UseDSMPlugin = 0;// LoadInt(hkLocal, "UseDSMPlugin", m_pref_UseDSMPlugin);
//	LoadDSMPluginName(hkLocal, m_pref_szDSMPlugin);

	if (m_server->LoopbackOnly()) 
		m_server->SetLoopbackOk(true);
	else
		m_server->SetLoopbackOk( true );//LoadInt(hkLocal, "AllowLoopback", false));

	m_server->SetAuthRequired( false );//LoadInt(hkLocal, "AuthRequired", true));

	m_server->SetConnectPriority( 0 );//LoadInt(hkLocal, "ConnectPriority", 0));
/*
	if (!m_server->LoopbackOnly())
	{
		char *authhosts = LoadString(hkLocal, "AuthHosts");
		if (authhosts != 0) 
		{
			m_server->SetAuthHosts(authhosts);
			delete [] authhosts;
		}
		else 
		{
			m_server->SetAuthHosts(0);
		}
	} 
	else 
	{
		m_server->SetAuthHosts(0);
	}
*/
	// If Socket connections are allowed, should the HTTP server be enabled?
//LABELUSERSETTINGS:
	// LOAD THE USER PREFERENCES

	// Set the default user prefs
	vnclog.Print(LL_INTINFO, VNCLOG("clearing user settings"));
//	m_pref_AutoPortSelect=TRUE;
//    m_pref_HTTPConnect = TRUE;
//	m_pref_XDMCPConnect = TRUE;
	m_pref_PortNumber = RFB_PORT_OFFSET; 
//	m_pref_SockConnect=TRUE;
	{
	    vncPasswd::FromClear crypt;
	    memcpy(m_pref_passwd, crypt, MAXPWLEN);
	}
//	m_pref_QuerySetting=2;
//	m_pref_QueryTimeout=10;
//	m_pref_QueryAccept=0;
/*	m_pref_IdleTimeout=0;
	m_pref_EnableRemoteInputs=TRUE;
	m_pref_DisableLocalInputs=FALSE;
	m_pref_LockSettings=-1;

	m_pref_RemoveWallpaper=TRUE;
    m_alloweditclients = TRUE;
	m_allowshutdown = TRUE;
	m_allowproperties = TRUE;
*/
	// Modif sf@2002
/*	m_pref_SingleWindow = FALSE;
	m_pref_UseDSMPlugin = FALSE;
	*m_pref_szDSMPlugin = '\0';
*/
/*	m_pref_EnableFileTransfer = TRUE;
	m_pref_FTUserImpersonation = TRUE;
	m_pref_EnableBlankMonitor = TRUE;
	m_pref_DefaultScale = 1;
	m_pref_CaptureAlphaBlending = FALSE; 
	m_pref_BlackAlphaBlending = FALSE; 
*/

	// Load the local prefs for this user
//	if (hkDefault != NULL)
//	{
//		vnclog.Print(LL_INTINFO, VNCLOG("loading DEFAULT local settings\n"));
//		LoadUserPrefs(hkDefault);
//		m_allowshutdown = LoadInt(hkDefault, "AllowShutdown", m_allowshutdown);
//		m_allowproperties = LoadInt(hkDefault, "AllowProperties", m_allowproperties);
//		m_alloweditclients = LoadInt(hkDefault, "AllowEditClients", m_alloweditclients);
//	}

	// Are we being asked to load the user settings, or just the default local system settings?
	if (usersettings) 
	{
		// We want the user settings, so load them!

//		if (hkLocalUser != NULL)
//		{
//			vnclog.Print(LL_INTINFO, VNCLOG("loading \"%s\" local settings\n"), username);
//			LoadUserPrefs(hkLocalUser);
//			m_allowshutdown = LoadInt(hkLocalUser, "AllowShutdown", m_allowshutdown);
//			m_allowproperties = LoadInt(hkLocalUser, "AllowProperties", m_allowproperties);
//			m_alloweditclients = LoadInt(hkLocalUser, "AllowEditClients", m_alloweditclients);
//		}

		// Now override the system settings with the user's settings
		// If the username is SYSTEM then don't try to load them, because there aren't any...
		if (m_allowproperties && (strcmp(username, "SYSTEM") != 0))
		{
/*
			HKEY hkGlobalUser;
			if (RegCreateKeyEx(HKEY_CURRENT_USER,
				WINVNC_REGISTRY_KEY,
				0, REG_NONE, REG_OPTION_NON_VOLATILE,
				KEY_READ, NULL, &hkGlobalUser, &dw) == ERROR_SUCCESS)
			{
				vnclog.Print(LL_INTINFO, VNCLOG("loading \"%s\" global settings\n"), username);
				LoadUserPrefs(hkGlobalUser);
				RegCloseKey(hkGlobalUser);

				// Close the user registry hive so it can unload if required
				RegCloseKey(HKEY_CURRENT_USER);
			}
*/
		}
	} 
	else 
	{
/*
		if (hkLocalUser != NULL)
		{
			vnclog.Print(LL_INTINFO, VNCLOG("loading \"%s\" local settings\n"), username);
			LoadUserPrefs(hkLocalUser);
			m_allowshutdown = LoadInt(hkLocalUser, "AllowShutdown", m_allowshutdown);
			m_allowproperties = LoadInt(hkLocalUser, "AllowProperties", m_allowproperties);
		    m_alloweditclients = LoadInt(hkLocalUser, "AllowEditClients", m_alloweditclients);
		}
		vnclog.Print(LL_INTINFO, VNCLOG("bypassing user-specific settings (both local and global)\n"));
*/
	}

//	if (hkLocalUser != NULL) RegCloseKey(hkLocalUser);
//	if (hkDefault != NULL) RegCloseKey(hkDefault);
//	if (hkLocal != NULL) RegCloseKey(hkLocal);

	// Make the loaded settings active..
	return ApplyUserPrefs();
}

/*
void
vncProperties::LoadUserPrefs(HKEY appkey)
{
	// LOAD USER PREFS FROM THE SELECTED KEY

	// Modif sf@2002
	m_pref_EnableFileTransfer = LoadInt(appkey, "FileTransferEnabled", m_pref_EnableFileTransfer);
	m_pref_FTUserImpersonation = LoadInt(appkey, "FTUserImpersonation", m_pref_FTUserImpersonation); // sf@2005
	m_pref_EnableBlankMonitor = LoadInt(appkey, "BlankMonitorEnabled", m_pref_EnableBlankMonitor);
	m_pref_DefaultScale = LoadInt(appkey, "DefaultScale", m_pref_DefaultScale);
	m_pref_CaptureAlphaBlending = LoadInt(appkey, "CaptureAlphaBlending", m_pref_CaptureAlphaBlending); // sf@2005
	m_pref_BlackAlphaBlending = LoadInt(appkey, "BlackAlphaBlending", m_pref_BlackAlphaBlending); // sf@2005

	m_pref_UseDSMPlugin = LoadInt(appkey, "UseDSMPlugin", m_pref_UseDSMPlugin);
	LoadDSMPluginName(appkey, m_pref_szDSMPlugin);

	// Connection prefs
	m_pref_SockConnect=LoadInt(appkey, "SocketConnect", m_pref_SockConnect);
	m_pref_HTTPConnect=LoadInt(appkey, "HTTPConnect", m_pref_HTTPConnect);
	m_pref_XDMCPConnect=LoadInt(appkey, "XDMCPConnect", m_pref_XDMCPConnect);
	m_pref_AutoPortSelect=LoadInt(appkey, "AutoPortSelect", m_pref_AutoPortSelect);
	m_pref_PortNumber=LoadInt(appkey, "PortNumber", m_pref_PortNumber);
	m_pref_HttpPortNumber=LoadInt(appkey, "HTTPPortNumber",
									DISPLAY_TO_HPORT(PORT_TO_DISPLAY(m_pref_PortNumber)));
	m_pref_IdleTimeout=LoadInt(appkey, "IdleTimeout", m_pref_IdleTimeout);
	
	m_pref_RemoveWallpaper=LoadInt(appkey, "RemoveWallpaper", m_pref_RemoveWallpaper);

	// Connection querying settings
	m_pref_QuerySetting=LoadInt(appkey, "QuerySetting", m_pref_QuerySetting);
	m_server->SetQuerySetting(m_pref_QuerySetting);
	m_pref_QueryTimeout=LoadInt(appkey, "QueryTimeout", m_pref_QueryTimeout);
	m_server->SetQueryTimeout(m_pref_QueryTimeout);
	m_pref_QueryAccept=LoadInt(appkey, "QueryAccept", m_pref_QueryAccept);
	m_server->SetQueryAccept(m_pref_QueryAccept);


	// Load the password
	LoadPassword(appkey, m_pref_passwd);

	// Remote access prefs
	m_pref_EnableRemoteInputs=LoadInt(appkey, "InputsEnabled", m_pref_EnableRemoteInputs);
	m_pref_LockSettings=LoadInt(appkey, "LockSetting", m_pref_LockSettings);
	m_pref_DisableLocalInputs=LoadInt(appkey, "LocalInputsDisabled", m_pref_DisableLocalInputs);
}
*/
int vncProperties::ApplyUserPrefs()
{
	// APPLY THE CACHED PREFERENCES TO THE SERVER
	// Modif sf@2002
	m_server->EnableFileTransfer(TRUE);
	m_server->FTUserImpersonation(TRUE); // sf@2005
	m_server->CaptureAlphaBlending(FALSE); // sf@2005
	m_server->BlackAlphaBlending(FALSE); // sf@2005

	m_server->BlankMonitorEnabled(TRUE);
	m_server->SetDefaultScale(1);

	// Update the connection querying settings
	m_server->SetQuerySetting(2);
	m_server->SetQueryTimeout(10);
	m_server->SetQueryAccept(0);
	m_server->SetAutoIdleDisconnectTimeout(0);
	m_server->EnableRemoveWallpaper(TRUE);

	// Is the listening socket closing?
	
	vnclog.Print(0, "vncProperties::ApplyUserPrefs(): m_server->SockConnect(TRUE);");
	int res = m_server->SockConnect(TRUE); //#here: try the connection and the startup dialog will be shown inside (in not silent mode)
	if (res == CONNECTION_SUCCESSFUL)
	{
		m_server->EnableHTTPConnect(TRUE);
		m_server->EnableXDMCPConnect(TRUE);

		// Are inputs being disabled?
		m_server->EnableRemoteInputs(TRUE);
		m_server->DisableLocalInputs(FALSE);

		// Update the password
		m_server->SetPassword(m_pref_passwd);

		// Now change the listening port settings
		m_server->SetAutoPortSelect(TRUE);
		// if (!m_pref_AutoPortSelect)
		// m_server->SetPort(m_pref_PortNumber);
		// m_server->SetPorts(m_pref_PortNumber, m_pref_HttpPortNumber); // Tight 1.2.7

		// Remote access prefs
		m_server->EnableRemoteInputs(TRUE);
		m_server->SetLockSettings(-1);
		m_server->DisableLocalInputs(FALSE);

		// DSM Plugin prefs
		// m_server->EnableDSMPlugin(FALSE);
		// m_server->SetDSMPluginName('');
		// if (m_server->IsDSMPluginEnabled()) m_server->SetDSMPlugin();
	}
	else
	{
		switch(res)
		{
		case NO_PROXY_SERVER_FOUND_TO_CONNECT:
		case AUTHENTICATION_FAILED:
			{
				if (ShowConnectAgain())
					res = ApplyUserPrefs();
			}
			break;
		}
	}
	return res;
}

void vncProperties::SaveInt(HKEY key, LPCSTR valname, LONG val)
{
	RegSetValueEx(key, valname, 0, REG_DWORD, (LPBYTE) &val, sizeof(val));
}

void vncProperties::SavePassword(HKEY key, char *buffer)
{
	RegSetValueEx(key, "Password", 0, REG_BINARY, (LPBYTE) buffer, MAXPWLEN);
}
/*
void
vncProperties::SaveDSMPluginName(HKEY key, char *buffer)
{
	RegSetValueEx(key, "DSMPlugin", 0, REG_BINARY, (LPBYTE) buffer, MAXPATH);
}


void
vncProperties::LoadDSMPluginName(HKEY key, char *buffer)
{
	DWORD type = REG_BINARY;
	int slen=MAXPATH;
	char inouttext[MAXPATH];

	if (RegQueryValueEx(key,
		"DSMPlugin",
		NULL,
		&type,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen) != ERROR_SUCCESS)
		return;

	if (slen > MAXPATH)
		return;

	memcpy(buffer, inouttext, MAXPATH);
}
*/

void vncProperties::Save()
{
/*
	HKEY appkey;
	DWORD dw;

	if (!m_allowproperties  || !RunningAsAdministrator ())
		return;

	// NEW (R3) PREFERENCES ALGORITHM
	// The user's prefs are only saved if the user is allowed to override
	// the machine-local settings specified for them.  Otherwise, the
	// properties entry on the tray icon menu will be greyed out.

	// GET THE CORRECT KEY TO READ FROM

	// Have we loaded user settings, or system settings?
	if (m_usersettings) {
		// Verify that we know who is logged on
		char username[UNLEN+1];
		if (!vncService::CurrentUser((char *)&username, sizeof(username)))
			return;
		if (strcmp(username, "") == 0)
			return;

		// Try to get the per-user, global registry key for WinVNC
		if (RegCreateKeyEx(HKEY_CURRENT_USER,
			WINVNC_REGISTRY_KEY,
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_WRITE | KEY_READ, NULL, &appkey, &dw) != ERROR_SUCCESS)
			return;
	} else {
		// Try to get the default local registry key for WinVNC
		HKEY hkLocal;
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
			WINVNC_REGISTRY_KEY,
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS) {
			MessageBox(NULL, sz_ID_MB1, sz_ID_WVNC, MB_OK);
			return;
		}

		if (RegCreateKeyEx(hkLocal,
			"Default",
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_WRITE | KEY_READ, NULL, &appkey, &dw) != ERROR_SUCCESS) {
			RegCloseKey(hkLocal);
			return;
		}
		RegCloseKey(hkLocal);
	}

	// SAVE PER-USER PREFS IF ALLOWED
	SaveUserPrefs(appkey);
	RegCloseKey(appkey);
	RegCloseKey(HKEY_CURRENT_USER);

	// Machine Preferences
	// Get the machine registry key for WinVNC
	HKEY hkLocal,hkDefault;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		WINVNC_REGISTRY_KEY,
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
		return;
	if (RegCreateKeyEx(hkLocal,
		"Default",
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_READ,
		NULL,
		&hkDefault,
		&dw) != ERROR_SUCCESS)
		hkDefault = NULL;
	// sf@2003
	SaveInt(hkLocal, "DebugMode", vnclog.GetMode());
	SaveInt(hkLocal, "DebugLevel", vnclog.GetLevel());
	SaveInt(hkLocal, "AllowLoopback", m_server->LoopbackOk());
	SaveInt(hkLocal, "LoopbackOnly", m_server->LoopbackOnly());
	if (hkDefault) SaveInt(hkDefault, "AllowShutdown", m_allowshutdown);
	if (hkDefault) SaveInt(hkDefault, "AllowProperties",  m_allowproperties);
	if (hkDefault) SaveInt(hkDefault, "AllowEditClients", m_alloweditclients);

	SaveInt(hkLocal, "DisableTrayIcon", m_server->GetDisableTrayIcon());
	SaveInt(hkLocal, "MSLogonRequired", m_server->MSLogonRequired());
	// Marscha@2004 - authSSP: save "New MS-Logon" state
	SaveInt(hkLocal, "NewMSLogon", m_server->GetNewMSLogon());
	// sf@2003 - DSM params here
	SaveInt(hkLocal, "UseDSMPlugin", m_server->IsDSMPluginEnabled());
	SaveInt(hkLocal, "ConnectPriority", m_server->ConnectPriority());
	SaveDSMPluginName(hkLocal, m_server->GetDSMPluginName());
	RegCloseKey(hkDefault);
	RegCloseKey(hkLocal);
*/
}
/*
void
vncProperties::SaveUserPrefs(HKEY appkey)
{
	// SAVE THE PER USER PREFS
	vnclog.Print(LL_INTINFO, VNCLOG("saving current settings to registry\n"));

	// Modif sf@2002
	SaveInt(appkey, "FileTransferEnabled", m_server->FileTransferEnabled());
	SaveInt(appkey, "FTUserImpersonation", m_server->FTUserImpersonation()); // sf@2005
	SaveInt(appkey, "BlankMonitorEnabled", m_server->BlankMonitorEnabled());
	SaveInt(appkey, "CaptureAlphaBlending", m_server->CaptureAlphaBlending()); // sf@2005
	SaveInt(appkey, "BlackAlphaBlending", m_server->BlackAlphaBlending()); // sf@2005

	SaveInt(appkey, "DefaultScale", m_server->GetDefaultScale());

	SaveInt(appkey, "UseDSMPlugin", m_server->IsDSMPluginEnabled());
	SaveDSMPluginName(appkey, m_server->GetDSMPluginName());

	// Connection prefs
	SaveInt(appkey, "SocketConnect", m_server->SockConnected());
	SaveInt(appkey, "HTTPConnect", m_server->HTTPConnectEnabled());
	SaveInt(appkey, "XDMCPConnect", m_server->XDMCPConnectEnabled());
	SaveInt(appkey, "AutoPortSelect", m_server->AutoPortSelect());
	if (!m_server->AutoPortSelect()) {
		SaveInt(appkey, "PortNumber", m_server->GetPort());
		SaveInt(appkey, "HTTPPortNumber", m_server->GetHttpPort());
	}
	SaveInt(appkey, "InputsEnabled", m_server->RemoteInputsEnabled());
	SaveInt(appkey, "LocalInputsDisabled", m_server->LocalInputsDisabled());
	SaveInt(appkey, "IdleTimeout", m_server->AutoIdleDisconnectTimeout());

	// Connection querying settings
	SaveInt(appkey, "QuerySetting", m_server->QuerySetting());
	SaveInt(appkey, "QueryTimeout", m_server->QueryTimeout());
	SaveInt(appkey, "QueryAccept", m_server->QueryAccept());

	// Lock settings
	SaveInt(appkey, "LockSetting", m_server->LockSettings());

	// Wallpaper removal
	SaveInt(appkey, "RemoveWallpaper", m_server->RemoveWallpaperEnabled());

	// Save the password
	char passwd[MAXPWLEN];
	m_server->GetPassword(passwd);
	SavePassword(appkey, passwd);
}
*/
