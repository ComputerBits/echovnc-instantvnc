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


// vncUserNameDlg.cpp

// Implementation of the vncUserNameDlg dialog!

#include "stdhdrs.h"

#include "WinVNC.h"
#include "vncUserNameDlg.h"
#include "../COMMON/CommonBase.h"
#include "Localization.h"
//--------------------------------------------------------------------------------------

// Constructor/destructor
vncUserNameDlg::vncUserNameDlg()
{
	memset(m_szUsername, 0, 71);
	m_nType = 0;
}
//--------------------------------------------------------------------------------------

vncUserNameDlg::~vncUserNameDlg()
{
}
//--------------------------------------------------------------------------------------

// Initialisation
BOOL vncUserNameDlg::Init( HWND hDlg)
{
	SendMessage( ::GetDlgItem(hDlg, IDC_EDIT_NAME),  EM_LIMITTEXT, 70, 0 );
	switch (m_nType)
	{
	case 0:
		SetWindowText(::GetDlgItem(hDlg, IDC_STATIC_LABEL), sz_IDS_STRING41084);
		SetWindowText(hDlg, sz_IDS_STRING41087);
		break;
	case 1:
		SetWindowText(::GetDlgItem(hDlg, IDC_STATIC_LABEL), sz_IDS_STRING41085);
		SetWindowText(hDlg, sz_IDS_STRING41088);
		break;
	case 2:
		SetWindowText(::GetDlgItem(hDlg, IDC_STATIC_LABEL), sz_IDS_STRING41086);
		SetWindowText(hDlg, sz_IDS_STRING41089);
		SendMessage( ::GetDlgItem(hDlg, IDC_EDIT_NAME),  EM_SETPASSWORDCHAR, '*', 0 );
		break;
	}
	SetWindowText( ::GetDlgItem(hDlg, IDOK), sz_IDS_OK41058);
	SetWindowText( ::GetDlgItem(hDlg, IDCANCEL), sz_IDS_Cancel41059);
	return TRUE;
}
//--------------------------------------------------------------------------------------

// Dialog box handling functions
int vncUserNameDlg::Show(BOOL show)
{
	int ret =  DialogBoxParam(hAppInstance,
		MAKEINTRESOURCE(IDD_USERNAME), 
		NULL,
		(DLGPROC) DialogProc,
		(LONG) this);
	return ret;
}
//--------------------------------------------------------------------------------------

BOOL CALLBACK vncUserNameDlg::DialogProc(HWND hwnd,
					 UINT uMsg,
					 WPARAM wParam,
					 LPARAM lParam )
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncUserNameDlg *_this = (vncUserNameDlg *) GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			_this = (vncUserNameDlg *) lParam;

			// Show the dialog
			SetForegroundWindow(hwnd);

			return _this->Init(hwnd);
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDCANCEL:
			EndDialog(hwnd, FALSE);
			return TRUE;
		case IDOK:
			if (_this->CheckData(hwnd))
			{
				EndDialog(hwnd, TRUE);
				return TRUE;
			}
			else
				return FALSE;
		}
		break;
	case WM_DESTROY:
		EndDialog(hwnd, FALSE);
		return TRUE;
	}
	return 0;
}
//--------------------------------------------------------------------------------------

bool vncUserNameDlg::CheckData( HWND hDlg )
{
	m_szUsername[0] = 0;
	SendMessage( ::GetDlgItem(hDlg, IDC_EDIT_NAME), WM_GETTEXT, 70, (LPARAM)m_szUsername );
	if (strlen(m_szUsername) == 0)
	{
		switch (m_nType)
		{
		case 0:
			NS_Common::Message_Box( sz_IDS_INSTANTVNC_NOADDRESS41050, MB_OK | MB_ICONSTOP, hDlg);
			break;
		case 1:
			NS_Common::Message_Box( sz_IDS_INSTANTVNC_NONAME41051, MB_OK | MB_ICONSTOP, hDlg);
			break;
		case 2:
			NS_Common::Message_Box( sz_IDS_INSTANTVNC_NOPASSWORD1052, MB_OK | MB_ICONSTOP, hDlg);
			break;
		}		
		SetFocus( ::GetDlgItem(hDlg, IDC_EDIT_NAME) );
		return false;
	}
	return true;
}
//--------------------------------------------------------------------------------------

int vncUserNameDlg::GetEnteredData(char* pName, int len)
{
	if (len < (int) strlen(m_szUsername))
	{
		return strlen(m_szUsername);
	}
	memset(pName, 0, len);
	strcpy_s(pName, len, m_szUsername);
	return 0;
}
//--------------------------------------------------------------------------------------

void vncUserNameDlg::setType(int nType)
{
	m_nType = nType;
}