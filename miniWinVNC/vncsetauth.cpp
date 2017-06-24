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


// vncSetAuth.cpp

// Implementation of the About dialog!

#include "stdhdrs.h"

#include "WinVNC.h"
#include "vncsetAuth.h"

#define MAXSTRING 254

const TCHAR REGISTRY_KEY [] = "Software\\UltraVnc";

void
vncSetAuth::OpenRegistry()
{
	DWORD dw;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		REGISTRY_KEY,
		0,REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ,
		NULL, &hkLocal, &dw) != ERROR_SUCCESS)
		return;
	if (RegCreateKeyEx(hkLocal,
		"mslogon",
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_READ,
		NULL, &hkDefault, &dw) != ERROR_SUCCESS)
		return;
}

void
vncSetAuth::CloseRegistry()
{
	if (hkDefault != NULL) RegCloseKey(hkDefault);
	if (hkUser != NULL) RegCloseKey(hkUser);
	if (hkLocal != NULL) RegCloseKey(hkLocal);
}

LONG
vncSetAuth::LoadInt(HKEY key, LPCSTR valname, LONG defval)
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

TCHAR *
vncSetAuth::LoadString(HKEY key, LPCSTR keyname)
{
	DWORD type = REG_SZ;
	DWORD buflen = 256*sizeof(TCHAR);
	TCHAR *buffer = 0;

	// Get the length of the string
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		NULL,
		&buflen) != ERROR_SUCCESS)
		return 0;

	if (type != REG_BINARY)
		return 0;
	buflen = 256*sizeof(TCHAR);
	buffer = new TCHAR[buflen];
	if (buffer == 0)
		return 0;

	// Get the string data
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		(BYTE*)buffer,
		&buflen) != ERROR_SUCCESS) {
		delete [] buffer;
		return 0;
	}

	// Verify the type
	if (type != REG_BINARY) {
		delete [] buffer;
		return 0;
	}

	return (TCHAR *)buffer;
}

void
vncSetAuth::SaveInt(HKEY key, LPCSTR valname, LONG val)
{
	RegSetValueEx(key, valname, 0, REG_DWORD, (LPBYTE) &val, sizeof(val));
}

void
vncSetAuth::SaveString(HKEY key,LPCSTR valname, TCHAR *buffer)
{
	RegSetValueEx(key, valname, 0, REG_BINARY, (LPBYTE) buffer, MAXSTRING);
}

void
vncSetAuth::savegroup1(TCHAR *value)
{
	OpenRegistry();
	if (hkDefault)SaveString(hkDefault, "group1", value);
	CloseRegistry();
}
TCHAR*
vncSetAuth::Readgroup1()
{
	TCHAR *value=NULL;
	OpenRegistry();
	if (hkDefault) value=LoadString (hkDefault, "group1");
	CloseRegistry();
	return value;
}

void
vncSetAuth::savegroup2(TCHAR *value)
{
	OpenRegistry();
	if (hkDefault)SaveString(hkDefault, "group2", value);
	CloseRegistry();
}
TCHAR*
vncSetAuth::Readgroup2()
{
	TCHAR *value=NULL;
	OpenRegistry();
	if (hkDefault) value=LoadString (hkDefault, "group2");
	CloseRegistry();
	return value;
}

void
vncSetAuth::savegroup3(TCHAR *value)
{
	OpenRegistry();
	if (hkDefault)SaveString(hkDefault, "group3", value);
	CloseRegistry();
}
TCHAR*
vncSetAuth::Readgroup3()
{
	TCHAR *value=NULL;
	OpenRegistry();
	if (hkDefault) value=LoadString (hkDefault, "group3");
	CloseRegistry();
	return value;
}

LONG
vncSetAuth::Readlocdom1(LONG returnvalue)
{
	OpenRegistry();
	if (hkDefault) returnvalue=LoadInt(hkDefault, "locdom1",returnvalue);
	CloseRegistry();
	return returnvalue;
}

void
vncSetAuth::savelocdom1(LONG value)
{
	OpenRegistry();
	if (hkDefault)SaveInt(hkDefault, "locdom1", value);
	CloseRegistry();

}

LONG
vncSetAuth::Readlocdom2(LONG returnvalue)
{
	OpenRegistry();
	if (hkDefault) returnvalue=LoadInt(hkDefault, "locdom2",returnvalue);
	CloseRegistry();
	return returnvalue;
}

void
vncSetAuth::savelocdom2(LONG value)
{
	OpenRegistry();
	if (hkDefault)SaveInt(hkDefault, "locdom2", value);
	CloseRegistry();

}

LONG
vncSetAuth::Readlocdom3(LONG returnvalue)
{
	OpenRegistry();
	if (hkDefault) returnvalue=LoadInt(hkDefault, "locdom3",returnvalue);
	CloseRegistry();
	return returnvalue;
}

void
vncSetAuth::savelocdom3(LONG value)
{
	OpenRegistry();
	if (hkDefault)SaveInt(hkDefault, "locdom3", value);
	CloseRegistry();

}

///////////////////////////////////////////////////////////



// Constructor/destructor
vncSetAuth::vncSetAuth()
{
	m_dlgvisible = FALSE;
	hkLocal=NULL;
	hkDefault=NULL;
	hkUser=NULL;
	locdom1=0;
	locdom2=0;
	locdom3=0;
	group1=Readgroup1();
	group2=Readgroup2();
	group3=Readgroup3();
	locdom1=Readlocdom1(locdom1);
	locdom2=Readlocdom2(locdom2);
	locdom3=Readlocdom3(locdom3);
	if (group1){strcpy_s(pszgroup1,256,group1);delete group1;}
	else strcpy_s(pszgroup1,256,"");
	if (group2){strcpy_s(pszgroup2,256,group2);delete group2;}
	else strcpy_s(pszgroup2,256,"");
	if (group3){strcpy_s(pszgroup3,256,group3);delete group3;}
	else strcpy_s(pszgroup3,256,"");
}

vncSetAuth::~vncSetAuth()
{
}

// Initialisation
BOOL
vncSetAuth::Init(vncServer *server)
{
	m_server = server;
	m_dlgvisible = FALSE;
	hkLocal=NULL;
	hkDefault=NULL;
	hkUser=NULL;
	locdom1=0;
	locdom2=0;
	locdom3=0;
	group1=Readgroup1();
	group2=Readgroup2();
	group3=Readgroup3();
	locdom1=Readlocdom1(locdom1);
	locdom2=Readlocdom2(locdom2);
	locdom3=Readlocdom3(locdom3);
	if (group1){strcpy_s(pszgroup1,256,group1);delete group1;}
	else strcpy_s(pszgroup1,256,"");
	if (group2){strcpy_s(pszgroup2,256,group2);delete group2;}
	else strcpy_s(pszgroup2,256,"");
	if (group3){strcpy_s(pszgroup3,256,group3);delete group3;}
	else strcpy_s(pszgroup3,256,"");
	return TRUE;
}

// Dialog box handling functions
void
vncSetAuth::Show(BOOL show)
{
	if (show)
	{
		if (!m_dlgvisible)
		{
			DialogBoxParam(hAppInstance,
				MAKEINTRESOURCE(IDD_MSLOGON), 
				NULL,
				(DLGPROC) DialogProc,
				(LONG) this);
		}
	}
}

BOOL CALLBACK
vncSetAuth::DialogProc(HWND hwnd,
					 UINT uMsg,
					 WPARAM wParam,
					 LPARAM lParam )
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncSetAuth *_this = (vncSetAuth *) GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			_this = (vncSetAuth *) lParam;
			SetDlgItemText(hwnd, IDC_GROUP1, _this->pszgroup1);
			SetDlgItemText(hwnd, IDC_GROUP2, _this->pszgroup2);
			SetDlgItemText(hwnd, IDC_GROUP3, _this->pszgroup3);
			if (_this->locdom1==1 || _this->locdom1==3)
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG1L);
				SendMessage(hG1l,BM_SETCHECK,true,0);
			}
			else
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG1L);
				SendMessage(hG1l,BM_SETCHECK,false,0);
			}
			if (_this->locdom1==2 || _this->locdom1==3)
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG1D);
				SendMessage(hG1l,BM_SETCHECK,true,0);
			}
			else
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG1D);
				SendMessage(hG1l,BM_SETCHECK,false,0);
			}
			if (_this->locdom2==1 || _this->locdom2==3)
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG2L);
				SendMessage(hG1l,BM_SETCHECK,true,0);
			}
			else
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG2L);
				SendMessage(hG1l,BM_SETCHECK,false,0);
			}
			if (_this->locdom2==2 || _this->locdom2==3)
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG2D);
				SendMessage(hG1l,BM_SETCHECK,true,0);
			}
			else
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG2D);
				SendMessage(hG1l,BM_SETCHECK,false,0);
			}
			if (_this->locdom3==1 || _this->locdom3==3)
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG3L);
				SendMessage(hG1l,BM_SETCHECK,true,0);
			}
			else
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG3L);
				SendMessage(hG1l,BM_SETCHECK,false,0);
			}
			if (_this->locdom3==2 || _this->locdom3==3)
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG3D);
				SendMessage(hG1l,BM_SETCHECK,true,0);
			}
			else
			{
				HWND hG1l = GetDlgItem(hwnd, IDC_CHECKG3D);
				SendMessage(hG1l,BM_SETCHECK,false,0);
			}
			//already handled by vncproperties
			//if we get at thgis place
			//IDC_MSLOGON_CHECKD was checked
			/*HWND hMSLogon = GetDlgItem(hwnd, IDC_MSLOGON_CHECKD);
            SendMessage(hMSLogon, BM_SETCHECK, _this->m_server->MSLogonRequired(), 0);
			if (SendMessage(hMSLogon, BM_GETCHECK,0, 0) == BST_CHECKED)
				{
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG1D), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG2D), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG3D), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG1L), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG2L), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG3L), true);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP1), true);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP2), true);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP3), true);
				}
			else
				{
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG1D), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG2D), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG3D), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG1L), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG2L), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG3L), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP1), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP2), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP3), FALSE);
				}*/



			// Show the dialog
			SetForegroundWindow(hwnd);

			_this->m_dlgvisible = TRUE;

			return TRUE;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDCANCEL:
			EndDialog(hwnd, TRUE);
				_this->m_dlgvisible = FALSE;
				return TRUE;
		case IDOK:
			{
				_this->locdom1=0;
				_this->locdom2=0;
				_this->locdom3=0;
				if (SendMessage(GetDlgItem(hwnd, IDC_CHECKG1L),BM_GETCHECK,0,0) == BST_CHECKED) _this->locdom1=_this->locdom1+1;
				if (SendMessage(GetDlgItem(hwnd, IDC_CHECKG1D),BM_GETCHECK,0,0) == BST_CHECKED) _this->locdom1=_this->locdom1+2;

				if (SendMessage(GetDlgItem(hwnd, IDC_CHECKG2L),BM_GETCHECK,0,0) == BST_CHECKED) _this->locdom2=_this->locdom2+1;
				if (SendMessage(GetDlgItem(hwnd, IDC_CHECKG2D),BM_GETCHECK,0,0) == BST_CHECKED) _this->locdom2=_this->locdom2+2;

				if (SendMessage(GetDlgItem(hwnd, IDC_CHECKG3L),BM_GETCHECK,0,0) == BST_CHECKED) _this->locdom3=_this->locdom3+1;
				if (SendMessage(GetDlgItem(hwnd, IDC_CHECKG3D),BM_GETCHECK,0,0) == BST_CHECKED) _this->locdom3=_this->locdom3+2;

				GetDlgItemText(hwnd, IDC_GROUP1, (LPSTR) _this->pszgroup1, 240);
				GetDlgItemText(hwnd, IDC_GROUP2, (LPSTR) _this->pszgroup2, 240);
				GetDlgItemText(hwnd, IDC_GROUP3, (LPSTR) _this->pszgroup3, 240);

				_this->savegroup1(_this->pszgroup1);
				_this->savegroup2(_this->pszgroup2);
				_this->savegroup3(_this->pszgroup3);
				_this->savelocdom1(_this->locdom1);
				_this->savelocdom2(_this->locdom2);
				_this->savelocdom3(_this->locdom3);

				EndDialog(hwnd, TRUE);
				_this->m_dlgvisible = FALSE;
				return TRUE;
			}
		// rdv not needed, already handled by vncproperties
		// 
		/*case IDC_MSLOGON_CHECKD:
			{
				HWND hMSLogon = GetDlgItem(hwnd, IDC_MSLOGON_CHECKD);
				_this->m_server->RequireMSLogon(SendMessage(hMSLogon, BM_GETCHECK, 0, 0) == BST_CHECKED);
				if (SendMessage(hMSLogon, BM_GETCHECK,0, 0) == BST_CHECKED)
					{
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG1D), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG2D), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG3D), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG1L), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG2L), true);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG3L), true);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP1), true);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP2), true);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP3), true);
						
					}
				else
					{
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG1D), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG2D), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG3D), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG1L), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG2L), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_CHECKG3L), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP1), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP2), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_GROUP3), FALSE);
					}
			}*/
		}

		break;

	case WM_DESTROY:
		EndDialog(hwnd, FALSE);
		_this->m_dlgvisible = FALSE;
		return TRUE;
	}
	return 0;
}
