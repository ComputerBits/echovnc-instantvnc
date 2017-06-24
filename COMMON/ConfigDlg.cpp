//  Copyright (C) 2005-2010, Echogent Systems, Inc. All Rights Reserved.
//
//  This file is part of the InstantVNC application.
//
//  InstantVNC is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//  ConfigDlg.cpp: implementation of the CConfigDlg class.
//
//////////////////////////////////////////////////////////////////////
#include "windows.h"

#ifndef IDD_AskRunDlgProc

#define IDD_AskRunDlgProc				700
#define IDC_EDIT_ADDR					701
#define IDC_EDIT_PWD					702
#define IDC_EDIT_NAME					703
#define IDC_STATIC_VNC_VERSION			704
#define IDC_STATIC_DLL_VERSION			705
#define IDC_STATIC_NUMBER_DLL_VERSION   1121
#define IDC_BTN_ADVANCED                1123

#define IDR_BANNER                      150

#endif

#include "irUtility.h"	//#IR:100303 +

#include "UserData.h" 
#include "ConfigDlg.h"
#include "CommonBase.h"
#include "ParseCmdLine.h"
#include "..\miniWinVNC\ProxyData.h"
#include <shellapi.h>

#include "..\miniWinVNC\Localization.h"
#include "..\miniWinVNC\resource.h"

LRESULT CALLBACK AskRunDlgProc( HWND hDlg,UINT iMsg,WPARAM wParam,LPARAM lParam);

/////////////////////////////////////////////////

static CConfigDlg* g_pDlg = 0;
static HINSTANCE hInstanceMod = 0;
bool bShowMaxSplash = true;

bool CConfigDlg::AskRun( CUserData* pData, HINSTANCE hInstance )//static
{
	hInstanceMod = hInstance;
	g_pDlg = new CConfigDlg( pData );
	
	//(#IR +
	if (NULL == g_pDlg) {
		return false;
	}
	g_pDlg->m_pic.SetInstanceMod(hInstance);
	//)

	int iRes = DialogBox( 
		hInstance, 
		MAKEINTRESOURCE(IDD_AskRunDlgProc), 
		0,//m_hWnd,
		( DLGPROC)AskRunDlgProc);
	delete g_pDlg;
	g_pDlg = 0;
	
	return (iRes == IDOK ) ? true:false;
}



// ServerIpDlgProc is the Message Handler and Dispatcher for CServerDataDlg.
LRESULT CALLBACK AskRunDlgProc( HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint( hDlg, &ps);
			g_pDlg->OnPaint( hDC );
			EndPaint( hDlg, &ps);
			break;
		}

		case WM_INITDIALOG:
		{
			g_pDlg->OnInitDialog(hDlg);
			break;
		}

		case WM_LBUTTONDOWN:
		{
			g_pDlg->OnLButtonDown( lParam );
			break;
		}

		case WM_MOUSEMOVE:
		{
			g_pDlg->OnMouseMove( lParam);
			break;
		}

		case WM_SETCURSOR:
		{
			return g_pDlg->SetCursor();
		}

		case WM_CTLCOLORSTATIC:
		{
			DWORD dw = g_pDlg->OnCtlColorStatic( wParam, lParam );
			if (dw)
				return dw;
			else
				break;
		}

		case WM_COMMAND:
		{
			UINT uiCommand = LOWORD(wParam);
			switch(uiCommand)
			{
				case IDC_BTN_ADVANCED:
				{
					CProxyData data;
					CProxyDataDlg::AskProxyData( 
						&g_pDlg->m_pUserData->m_proxy,
						hInstanceMod, hDlg );
					return TRUE;
				}

				case IDOK:
				{
					if (g_pDlg->CheckData( hDlg ))
					{
						EndDialog( hDlg, uiCommand);
						return TRUE;
					}
					else
						break;
				}

				case IDCANCEL:
				{
					EndDialog( hDlg, uiCommand);
					return TRUE;
				}

				default:
					break;
			}

		}

		default:
			return 0;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const int CConfigDlg::m_iMaxLenString = 70;

#define _COLOR_HIPER_LINK RGB( 0, 0, 225)

CConfigDlg::CConfigDlg(CUserData* pUserData)
:	m_pUserData(pUserData),
	m_hWndAboutWinVnc(0),
	m_hWndAbouteEchoWare(0),
	m_bIsInside(false),
	m_hAddress(0),
	m_hPassword(0),
	m_hUserName(0)
{
	LOGBRUSH lBr;
	lBr.lbStyle = BS_HOLLOW;			// UINT     lbStyle; 
	lBr.lbColor = _COLOR_HIPER_LINK;	// COLORREF lbColor; 
	lBr.lbHatch = HS_CROSS;				// LONG     lbHatch
	m_Brush = ::CreateBrushIndirect( &lBr );
}


CConfigDlg::~CConfigDlg()
{
}

extern const char szAppName[];

extern const char* g_szInstantVNCVersion;
extern const char* g_szEchoWareVersion;

void CConfigDlg::OnInitDialog( HWND hDlg )
{
	//Load strings:

	if (bShowMaxSplash)
	{
		::SetWindowText(::GetDlgItem(hDlg, IDC_STATIC41053), sz_IDS_ADDRESS41053);
		::SetWindowText(::GetDlgItem(hDlg, IDC_STATIC41054), sz_IDS_PASSWORD41054);
		::SetWindowText(::GetDlgItem(hDlg, IDC_STATIC41055), sz_IDS_LOGINNAME41055);
		::SetWindowText(::GetDlgItem(hDlg, IDC_STATIC41057), sz_IDS_HINT41057);
		::SetWindowText(::GetDlgItem(hDlg, IDC_BTN_ADVANCED), sz_IDS_ADVSETTINGS41066);
	}
	else
	{
		::ShowWindow(::GetDlgItem(hDlg, IDC_STATIC41053), SW_HIDE);
		::ShowWindow(::GetDlgItem(hDlg, IDC_STATIC41054), SW_HIDE);
		::ShowWindow(::GetDlgItem(hDlg, IDC_STATIC41055), SW_HIDE);
		::ShowWindow(::GetDlgItem(hDlg, IDC_STATIC41057), SW_HIDE);
		::ShowWindow(::GetDlgItem(hDlg, IDC_BTN_ADVANCED), SW_HIDE);
	}

	::SetWindowText(::GetDlgItem(hDlg, IDC_STATIC41056), sz_IDS_HINT41056);
	::SetWindowText(::GetDlgItem(hDlg, IDOK), sz_IDS_OK41058);
	::SetWindowText(::GetDlgItem(hDlg, IDCANCEL), sz_IDS_Cancel41059);
	char szVer[32];

	sprintf_s( szVer, 32, "version %s", g_szEchoWareVersion );
	::SetWindowText( ::GetDlgItem( hDlg, IDC_STATIC_NUMBER_DLL_VERSION), szVer);

	sprintf_s( szVer, 32, "version %s", g_szInstantVNCVersion );
	::SetWindowText( ::GetDlgItem( hDlg, IDC_STATIC_NUMBER_VNC_VERSION), szVer);

	// IDC_STATIC_NUMBER_DLL_VERSION
	m_hWndAboutWinVnc		= ::GetDlgItem( hDlg, IDC_STATIC_VNC_VERSION);
	NS_Common::CalcRectCtlDialog( hDlg, m_hWndAboutWinVnc, m_rcAboutWinVnc );

	m_hWndAbouteEchoWare	= ::GetDlgItem( hDlg, IDC_STATIC_DLL_VERSION);
	NS_Common::CalcRectCtlDialog( hDlg, m_hWndAbouteEchoWare, m_rcAbouteEchoWare );

	if (bShowMaxSplash)
	{
		char sz[m_iMaxLenString + 1];

		if (m_pUserData->m_szPort.compare("1328") == 0 || m_pUserData->m_szPort.length() == 0)
			sprintf_s( sz, m_iMaxLenString + 1, "%s", m_pUserData->m_szAddress.c_str() );
		else
			sprintf_s( sz, m_iMaxLenString + 1, "%s:%s", m_pUserData->m_szAddress.c_str(), m_pUserData->m_szPort.c_str() );
		m_hAddress = ::GetDlgItem( hDlg, IDC_EDIT_ADDR);
		::SetWindowText( m_hAddress, sz);

		sprintf_s( sz, m_iMaxLenString + 1, "%s", m_pUserData->m_szPassword.c_str() );
		m_hPassword = ::GetDlgItem( hDlg, IDC_EDIT_PWD);
		::SetWindowText( m_hPassword, sz);

		sprintf_s( sz, m_iMaxLenString + 1, "%s", m_pUserData->m_szUsername.c_str() );
		m_hUserName = ::GetDlgItem( hDlg, IDC_EDIT_NAME);
		::SetWindowText( m_hUserName, sz);

		SendMessage( m_hAddress,  EM_LIMITTEXT, m_iMaxLenString, 0 );
		SendMessage( m_hPassword, EM_LIMITTEXT, m_iMaxLenString, 0 );
		SendMessage( m_hUserName, EM_LIMITTEXT, m_iMaxLenString, 0 );
	}
	else
	{
		::ShowWindow(::GetDlgItem(hDlg, IDC_EDIT_ADDR), SW_HIDE);
		::ShowWindow(::GetDlgItem(hDlg, IDC_EDIT_PWD), SW_HIDE);
		::ShowWindow(::GetDlgItem(hDlg, IDC_EDIT_NAME), SW_HIDE);
	}

	std::string sCaption = NS_Common::GetAppCaption();
	SendMessage( hDlg, WM_SETTEXT, 0, (LPARAM)sCaption.c_str());

	if (m_pUserData->m_szImagePath.length() != 0 )
	{
		m_pic.LoadFromFile(m_pUserData->m_szImagePath.c_str());
	}

	if (!m_pic.IsLoad())
	{
		m_pic.LoadFromRes( IDR_BANNER );	//#here: load banner on the InstantVNC dialog
	}

	if (bShowMaxSplash)
	{
		if (m_pUserData->m_bLockAddress)  ::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_ADDR), FALSE);
		if (m_pUserData->m_bLockUsername) ::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_NAME), FALSE);
		if (m_pUserData->m_bLockPassword) ::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_PWD), FALSE);
	}

	if (m_pUserData->m_bLockAdvProps) ::EnableWindow(::GetDlgItem(hDlg, IDC_BTN_ADVANCED), FALSE);
	
	if (!bShowMaxSplash)
	{
		RECT rect;
		POINT p1;

		::GetWindowRect(::GetDlgItem(hDlg, IDC_STATIC41056), &rect);
		p1.x = rect.left; p1.y = rect.top;
		ScreenToClient(hDlg, &p1);
		SetWindowPos(GetDlgItem(hDlg, IDC_STATIC41056), NULL, p1.x, p1.y - 95, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		::GetWindowRect(::GetDlgItem(hDlg, IDOK), &rect);
		p1.x = rect.left; p1.y = rect.top;
		ScreenToClient(hDlg, &p1);
		SetWindowPos(GetDlgItem(hDlg, IDOK), NULL, p1.x, p1.y - 90, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		::GetWindowRect(::GetDlgItem(hDlg, IDCANCEL), &rect);
		p1.x = rect.left; p1.y = rect.top;
		ScreenToClient(hDlg, &p1);
		SetWindowPos(GetDlgItem(hDlg, IDCANCEL), NULL, p1.x, p1.y - 90, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		::GetWindowRect(::GetDlgItem(hDlg, IDC_STATIC_VNC_VERSION), &rect);
		p1.x = rect.left; p1.y = rect.top;
		ScreenToClient(hDlg, &p1);
		SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_VNC_VERSION), NULL, p1.x, p1.y - 90, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		::GetWindowRect(::GetDlgItem(hDlg, IDC_STATIC_NUMBER_DLL_VERSION), &rect);
		p1.x = rect.left; p1.y = rect.top;
		ScreenToClient(hDlg, &p1);
		SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_NUMBER_DLL_VERSION), NULL, p1.x, p1.y - 90, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		::GetWindowRect(::GetDlgItem(hDlg, IDC_STATIC_DLL_VERSION), &rect);
		p1.x = rect.left; p1.y = rect.top;
		ScreenToClient(hDlg, &p1);
		SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_DLL_VERSION), NULL, p1.x, p1.y - 90, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		::GetWindowRect(::GetDlgItem(hDlg, IDC_STATIC_NUMBER_VNC_VERSION), &rect);
		p1.x = rect.left; p1.y = rect.top;
		ScreenToClient(hDlg, &p1);
		SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_NUMBER_VNC_VERSION), NULL, p1.x, p1.y - 90, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		::GetWindowRect(hDlg, &rect);
		::MoveWindow(hDlg, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top - 90, true);
	}
	Utility::CenterWindow(hDlg);	//#IR:100303 + 
}


void CConfigDlg::OnPaint( HDC hDC )
{
	if (m_pic.IsLoad())
	{
		//UINT uiWidth, uiHeight;
		//m_pic.GetSize( hDC, uiWidth, uiHeight);
		RECT rc;
		rc.left = 9;
		rc.top = 4;
		rc.right = rc.left + 360;//uiWidth;
		rc.bottom = rc.top + 60;//uiHeight;
		m_pic.Draw( hDC, rc);
	}
}


void CConfigDlg::OnLButtonDown( LPARAM lParam )
{
	if (IsInsideHyperLink(lParam))
	{
		::ShellExecute(NULL, "open", "http:\\\\www.EchoVNC.com", NULL,NULL, SW_SHOW);
	}
}


void CConfigDlg::OnMouseMove( LPARAM lParam )
{
	CheckInside(lParam);
}


int CConfigDlg::SetCursor()
{
	if (m_bIsInside)
	{
		HCURSOR hCur = ::LoadCursor( 0, MAKEINTRESOURCE(32649) );//IDC_HAND
		if (hCur)
		{
			::SetCursor(hCur);
			return 1;
		}
	}
	return 0;
}


DWORD CConfigDlg::OnCtlColorStatic( WPARAM wParam, LPARAM lParam )
{
	// handle to static control (HWND)
	HWND hControl = (HWND) lParam;

	// handle to DC (HDC)
	HDC hDC = (HDC) wParam;

	if ( hControl == m_hWndAboutWinVnc ||
		 hControl == m_hWndAbouteEchoWare)
	{
		SetBkMode( hDC, TRANSPARENT);
		SetTextColor( hDC, _COLOR_HIPER_LINK);
		return ((DWORD)m_Brush);
	}
	else
		return 0;
}


void CConfigDlg::CheckInside( LPARAM lParam)
{
	m_bIsInside = IsInsideHyperLink( lParam );
}


bool CConfigDlg::IsInsideHyperLink( LPARAM lParam)const
{
	int xPos = LOWORD(lParam);
	int yPos = HIWORD(lParam);

	if (m_rcAboutWinVnc.left < xPos && xPos < m_rcAboutWinVnc.right &&
		m_rcAboutWinVnc.top  < yPos && yPos < m_rcAboutWinVnc.bottom )
	{
		return true;
	}

	if (m_rcAbouteEchoWare.left < xPos && xPos < m_rcAbouteEchoWare.right &&
		m_rcAbouteEchoWare.top  < yPos && yPos < m_rcAbouteEchoWare.bottom )
	{
		return true;
	}

	return false;
}

bool CConfigDlg::CheckPort(char *szAddr)
{
	char *pColonPos = strchr(szAddr, ':');
	if (pColonPos != NULL) {
		int port;
		if (sscanf_s(pColonPos + 1, "%d", &port) != 1) return false;
		m_pUserData->m_szPort = pColonPos + 1;
		*pColonPos = '\0';
		m_pUserData->m_szAddress = szAddr;
	} else {
		m_pUserData->m_szAddress = szAddr;
		m_pUserData->m_szPort = "1328";
	}
	return true;
}

bool CConfigDlg::CheckData( HWND hDlg )
{
	if (!bShowMaxSplash)
		return true;
	char szAddress[m_iMaxLenString + 1] = {0};
	SendMessage( m_hAddress, WM_GETTEXT, m_iMaxLenString, (LPARAM)szAddress );
	if (strlen(szAddress) == 0)
	{
		NS_Common::Message_Box( sz_IDS_INSTANTVNC_NOADDRESS41050, MB_OK | MB_ICONSTOP, hDlg);
		SetFocus( m_hAddress );
		return false;
	}

	char szPassword[m_iMaxLenString + 1] = {0};
	SendMessage( m_hPassword, WM_GETTEXT, m_iMaxLenString, (LPARAM)szPassword );
	if (strlen(szPassword) == 0)
	{
		NS_Common::Message_Box( sz_IDS_INSTANTVNC_NOPASSWORD1052, MB_OK | MB_ICONSTOP, hDlg);
		SetFocus( m_hPassword );
		return false;
	}

	char szUsername[m_iMaxLenString + 1] = {0};
	SendMessage( m_hUserName, WM_GETTEXT, m_iMaxLenString, (LPARAM)szUsername );
	if (strlen(szUsername) == 0)
	{
		NS_Common::Message_Box( sz_IDS_INSTANTVNC_NONAME41051, MB_OK | MB_ICONSTOP, hDlg);
		SetFocus( m_hUserName );
		return false;
	}

	CheckPort(szAddress);

	m_pUserData->m_szPassword = szPassword;
	m_pUserData->m_szUsername = szUsername;

	return true;
}
