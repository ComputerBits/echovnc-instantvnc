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
// ConfigDlg.cpp: implementation of the CConfigDlg class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "windows.h"
#include <commctrl.h>

#include "resource.h"
#include "AskDlg.h"
#include "..\InstantVNC\SerializeData.h" 
#include "..\InstantVNC\CommonBase.h"
#include "ProxyData.h"
#include <shellapi.h>
#include <string>
#include "TextLister.h"

LRESULT CALLBACK AskRunDlgProc( HWND hDlg,UINT iMsg,WPARAM wParam,LPARAM lParam);

/////////////////////////////////////////////////

static void IpAddressToString( DWORD dwIpAddress, std::string& rStrIPAddress)
{
	rStrIPAddress = "";
	in_addr adr;
	adr.S_un.S_addr = dwIpAddress;
	for( int i = 0; i < 4; i++)
	{
		char str[_MAX_PATH];
		if (i==0)
			sprintf_s(str, _MAX_PATH, "%d.", (int)adr.S_un.S_un_b.s_b1);
		else if (i==1)
			sprintf_s( str, _MAX_PATH, "%d.", (int)adr.S_un.S_un_b.s_b2);
		else if (i==2)
			sprintf_s( str, _MAX_PATH, "%d.", (int)adr.S_un.S_un_b.s_b3);
		else //if (i==3)
			sprintf_s( str, _MAX_PATH, "%d", (int)adr.S_un.S_un_b.s_b4);

		rStrIPAddress += str;
	}
}

static CConfigDlg* g_pDlg = 0;
static HINSTANCE hInstanceMod = 0;

bool CConfigDlg::AskRun( CSerializeData* pData, HINSTANCE hInstance )//static
{
	hInstanceMod = hInstance;
	g_pDlg = new CConfigDlg( pData );

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

				case IDC_BTN_LOAD:
				{
					g_pDlg->OnBtnLoad(hDlg);
					return TRUE;
				}

				case IDC_BTN_ADVANCED:
				{
					CProxyDataDlg::AskProxyData( &g_pDlg->m_pUserData->m_data.m_ProxyData, hInstanceMod, hDlg );
					return TRUE;
				}

				case IDC_CHANGE_ICON_BTN:
				{
					g_pDlg->OnBtnChangeIcon(hDlg);
					return TRUE;
				}

				case IDC_CHANGE_TEXT_BTN:
				{
					g_pDlg->OnBtnChangeText(hDlg);
					return TRUE;
				}

				case IDC_LOCK_ADDR:
				{
					if (g_pDlg->m_bLockAddr) g_pDlg->m_bLockAddr = false; else g_pDlg->m_bLockAddr = true;
					g_pDlg->CheckLockStates();
					return FALSE;
				}

				case IDC_LOCK_NAME:
				{
					if (g_pDlg->m_bLockName) g_pDlg->m_bLockName = false; else g_pDlg->m_bLockName = true;
					g_pDlg->CheckLockStates();
					return FALSE;
				}

				case IDC_LOCK_PWD:
				{
					if (g_pDlg->m_bLockPwd) g_pDlg->m_bLockPwd = false; else g_pDlg->m_bLockPwd = true;
					g_pDlg->CheckLockStates();
					return FALSE;
				}

				case IDC_LOCK_ADVBTN:
				{
					if (g_pDlg->m_bLockAdvProps) g_pDlg->m_bLockAdvProps = false; else g_pDlg->m_bLockAdvProps = true;
					g_pDlg->CheckLockStates();
					return FALSE;
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

CConfigDlg::CConfigDlg(CSerializeData* pUserData)
:	m_pUserData(pUserData),
	m_hWndAboutWinVnc(0),
	m_hWndAbouteEchoWare(0),
	m_bIsInside(false),
	m_hAddress(0),
	m_hPassword(0),
	m_hUserName(0),
	m_hWnd(0),
	m_hwndToolTip(0),
	m_bLockAddr(false),
	m_bLockName(false),
	m_bLockPwd(false),
	m_bLockAdvProps(false)
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

void CConfigDlg::OnPaint( HDC hDC )
{
	if (m_pic.IsLoad())
	{
		//UINT uiWidth, uiHeight;
		//m_pic.GetSize( hDC, uiWidth, uiHeight);
		RECT rc;
		rc.left = 11;//9;
		rc.top = 4;
		rc.right = rc.left + 360;//uiWidth;
		rc.bottom = rc.top + 60;//uiHeight;
		m_pic.Draw(hDC, rc);
	}
}

void CConfigDlg::CreateToolTip()
{

	// create the tooltip window - we will use the default delays,
	// but if you want to change that, you can use the TTM_SETDELAYTIME message 
	m_hwndToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, 
		10, 10, m_hWnd, NULL, hInstanceMod, NULL);

	TOOLINFO ti;
	//add the button to the tooltip
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	// TTF_SUBCLASS causes the tooltip to automatically subclass the window and 
	// look for the messages it is interested in.
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = m_hWnd;
	HWND hwndButton = GetDlgItem( m_hWnd, IDC_BTN_LOAD);
	ti.uId = (UINT)hwndButton;
	ti.lpszText = "Load a 360x60 GIF or JPG image here";
	SendMessage(m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

}

#include "EchowareDll.h"

void CConfigDlg::OnInitDialog( HWND hDlg )
{
	m_hWnd = hDlg;
	m_hWndAboutWinVnc		= ::GetDlgItem( hDlg, IDC_STATIC_VNC_VERSION);
	NS_Common::CalcRectCtlDialog( hDlg, m_hWndAboutWinVnc, m_rcAboutWinVnc );

	m_hWndAbouteEchoWare	= ::GetDlgItem( hDlg, IDC_STATIC_DLL_VERSION);
	NS_Common::CalcRectCtlDialog( hDlg, m_hWndAbouteEchoWare, m_rcAbouteEchoWare );

	HWND hWndNumberDLL_VERSION = ::GetDlgItem(hDlg, IDC_STATIC_NUMBER_DLL_VERSION);
	char szVer[_MAX_PATH];
	memset(szVer, 0, _MAX_PATH);

	CEchowareDll dll;
	if (dll.Init())
		sprintf_s(szVer, _MAX_PATH, "version %s", dll.GetDllVersion());
	else
		strcpy_s(szVer, _MAX_PATH, "not found");
	::SetWindowText(hWndNumberDLL_VERSION, szVer);
	
	char sz[m_iMaxLenString + 1];

	sprintf_s( sz, m_iMaxLenString + 1, "%s", m_pUserData->m_data.m_szAddress.c_str() );
	m_hAddress = ::GetDlgItem( hDlg, IDC_EDIT_ADDR);
	::SetWindowText( m_hAddress, sz);

	sprintf_s( sz, m_iMaxLenString + 1, "%s", m_pUserData->m_data.m_szPassword.c_str() );
	m_hPassword = ::GetDlgItem( hDlg, IDC_EDIT_PWD);
	::SetWindowText( m_hPassword, sz);

	sprintf_s( sz, m_iMaxLenString + 1, "%s", m_pUserData->m_data.m_szUsername.c_str() );
	m_hUserName = ::GetDlgItem( hDlg, IDC_EDIT_NAME);
	::SetWindowText( m_hUserName, sz);

	if (_strcmpi(m_pUserData->m_data.m_szDebug.c_str(), S_DEBUG) == 0)
		CheckDlgButton(hDlg, IDC_DEBUG_LOGGING, BST_CHECKED);
	else
		CheckDlgButton(hDlg, IDC_DEBUG_LOGGING, BST_UNCHECKED);

	if (_strcmpi(m_pUserData->m_data.m_szAutoExitAfterLastViewer.c_str(), S_AUTO_EXIT_AFTER_LAST_VIEWER) == 0)
		CheckDlgButton(hDlg, IDC_AUTO_EXIT_AFTER_LAST_VIEWER, BST_CHECKED);
	else
		CheckDlgButton(hDlg, IDC_AUTO_EXIT_AFTER_LAST_VIEWER, BST_UNCHECKED);

	if (_strcmpi(m_pUserData->m_data.m_szAutoExitIfNoViewer.c_str(), S_AUTO_EXIT_IF_NO_VIEWER) == 0)
		CheckDlgButton(hDlg, IDC_AUTO_EXIT_IF_NO_VIEWER, BST_CHECKED);
	else
		CheckDlgButton(hDlg, IDC_AUTO_EXIT_IF_NO_VIEWER, BST_UNCHECKED);

	if (_strcmpi(m_pUserData->m_data.m_szCreatePasswords.c_str(), S_CREATE_RANDOM_PASS) == 0)
		CheckDlgButton(hDlg, IDC_CREATE_RANDOM_PASSWORDS, BST_CHECKED);
	else
		CheckDlgButton(hDlg, IDC_CREATE_RANDOM_PASSWORDS, BST_UNCHECKED);

	if (_strcmpi(m_pUserData->m_data.m_szEnableDualMonitors.c_str(), S_ENABLE_DUAL_MON) == 0)
		CheckDlgButton(hDlg, IDC_ALLOW_DUAL_MON, BST_CHECKED);
	else
		CheckDlgButton(hDlg, IDC_ALLOW_DUAL_MON, BST_UNCHECKED);

	if (_strcmpi(m_pUserData->m_data.m_szUseMiniGui.c_str(), S_USE_MINI_GUI) == 0)
		CheckDlgButton(hDlg, IDC_USE_MINI_GUI, BST_CHECKED);
	else
		CheckDlgButton(hDlg, IDC_USE_MINI_GUI, BST_UNCHECKED);

	if (_strcmpi(m_pUserData->m_data.m_szDisplayStartupHelp.c_str(), S_DISPLAY_STARTUP_HELP) == 0)
		CheckDlgButton(hDlg, IDC_DISPLAY_STARTUP_HELP, BST_CHECKED);
	else
		CheckDlgButton(hDlg, IDC_DISPLAY_STARTUP_HELP, BST_UNCHECKED);

	SendMessage( m_hAddress,  EM_LIMITTEXT, m_iMaxLenString, 0 );
	SendMessage( m_hPassword, EM_LIMITTEXT, m_iMaxLenString, 0 );
	SendMessage( m_hUserName, EM_LIMITTEXT, m_iMaxLenString, 0 );

	long lExStyle = GetWindowLong( hDlg, GWL_EXSTYLE );
	lExStyle |= WS_EX_TOPMOST;
	SetWindowLong( hDlg, GWL_EXSTYLE, lExStyle);

	SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);

	SetActiveWindow(hDlg);

	SetForegroundWindow(hDlg);

//	std::string sCaption = NS_Common::GetAppCaption();
//	SendMessage( hDlg, WM_SETTEXT, 0, (LPARAM)sCaption.c_str());

	if (m_pUserData->m_data.m_szImagePath.length() != 0 )
	{
		m_pic.LoadFromFile(m_pUserData->m_data.m_szImagePath.c_str());
	}

	if (!m_pic.IsLoad())
	{
		m_pic.LoadFromRes(IDR_BANNER);
	}

	if (strlen(m_pUserData->m_data.m_szLockAddress.c_str()) != 0)
		m_bLockAddr = true;
	if (strlen(m_pUserData->m_data.m_szLockUsername.c_str()) != 0)
		m_bLockName = true;
	if (strlen(m_pUserData->m_data.m_szLockPassword.c_str()) != 0)
		m_bLockPwd = true;
	if (strlen(m_pUserData->m_data.m_szLockAdvProps.c_str()) != 0)
		m_bLockAdvProps = true;

	CheckLockStates();

	CreateToolTip();
}

void CConfigDlg::OnLButtonDown( LPARAM lParam )
{
	if (IsInsideHyperLink(lParam))
	{
		::ShellExecute(NULL, "open", "http://www.echovnc.com", NULL, NULL, SW_SHOW);
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
		HCURSOR hCur = ::LoadCursor( 0, MAKEINTRESOURCE(32649) );//IDC_HAND);
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

bool CConfigDlg::CheckData( HWND hDlg )
{
	char szAddress[m_iMaxLenString + 1] = {0};
	SendMessage( m_hAddress, WM_GETTEXT, m_iMaxLenString, (LPARAM)szAddress );
//	if (strlen(szAddress) == 0)
//	{
//		NS_Common::Message_Box( "Empty address has been found", MB_OK | MB_ICONSTOP, hDlg);
//		SetFocus( m_hAddress );
//		return false;
//	}

	char szPassword[m_iMaxLenString + 1] = {0};
	SendMessage( m_hPassword, WM_GETTEXT, m_iMaxLenString, (LPARAM)szPassword );
//	if (strlen(szPassword) == 0)
//	{
//		NS_Common::Message_Box( "Empty password has been found", MB_OK | MB_ICONSTOP, hDlg);
//		SetFocus( m_hPassword );
//		return false;
//	}

	char szUsername[m_iMaxLenString + 1] = {0};
	SendMessage( m_hUserName, WM_GETTEXT, m_iMaxLenString, (LPARAM)szUsername );
//	if (strlen(szUsername) == 0)
//	{
//		NS_Common::Message_Box( "Empty Login Name has been found", MB_OK | MB_ICONSTOP, hDlg);
//		SetFocus( m_hUserName );
//		return false;
//	}

	m_pUserData->m_data.m_szAddress  = szAddress;
	m_pUserData->m_data.m_szPassword = szPassword;
	m_pUserData->m_data.m_szUsername = szUsername;

	if (IsDlgButtonChecked(m_hWnd, IDC_DEBUG_LOGGING) == BST_CHECKED)
		m_pUserData->m_data.m_szDebug = S_DEBUG;
	else
		m_pUserData->m_data.m_szDebug = "";

	if (IsDlgButtonChecked(m_hWnd, IDC_LOCK_ADDR) == BST_CHECKED)
		m_pUserData->m_data.m_szLockAddress = "-lockAddress";
	else
		m_pUserData->m_data.m_szLockAddress = "";

	if (IsDlgButtonChecked(m_hWnd, IDC_LOCK_NAME) == BST_CHECKED)
		m_pUserData->m_data.m_szLockUsername = "-lockUsername";
	else
		m_pUserData->m_data.m_szLockUsername = "";

	if (IsDlgButtonChecked(m_hWnd, IDC_LOCK_PWD) == BST_CHECKED)
		m_pUserData->m_data.m_szLockPassword = "-lockPassword";
	else
		m_pUserData->m_data.m_szLockPassword = "";

	if (IsDlgButtonChecked(m_hWnd, IDC_LOCK_ADVBTN) == BST_CHECKED)
		m_pUserData->m_data.m_szLockAdvProps = "-lockAdvSettings";
	else
		m_pUserData->m_data.m_szLockAdvProps = "";

	if (IsDlgButtonChecked(m_hWnd, IDC_AUTO_EXIT_AFTER_LAST_VIEWER) == BST_CHECKED)
		m_pUserData->m_data.m_szAutoExitAfterLastViewer = S_AUTO_EXIT_AFTER_LAST_VIEWER;
	else
		m_pUserData->m_data.m_szAutoExitAfterLastViewer = "";

	if (IsDlgButtonChecked(m_hWnd, IDC_AUTO_EXIT_IF_NO_VIEWER) == BST_CHECKED)
		m_pUserData->m_data.m_szAutoExitIfNoViewer = S_AUTO_EXIT_IF_NO_VIEWER;
	else
		m_pUserData->m_data.m_szAutoExitIfNoViewer = "";
	
	if (IsDlgButtonChecked(m_hWnd, IDC_CREATE_RANDOM_PASSWORDS) == BST_CHECKED)
		m_pUserData->m_data.m_szCreatePasswords = S_CREATE_RANDOM_PASS;
	else
		m_pUserData->m_data.m_szCreatePasswords = "";

	if (IsDlgButtonChecked(m_hWnd, IDC_ALLOW_DUAL_MON) == BST_CHECKED)
		m_pUserData->m_data.m_szEnableDualMonitors = S_ENABLE_DUAL_MON;
	else
		m_pUserData->m_data.m_szEnableDualMonitors = "";

	if (IsDlgButtonChecked(m_hWnd, IDC_USE_MINI_GUI) == BST_CHECKED)
		m_pUserData->m_data.m_szUseMiniGui = S_USE_MINI_GUI;
	else
		m_pUserData->m_data.m_szUseMiniGui = "";

	if (IsDlgButtonChecked(m_hWnd, IDC_DISPLAY_STARTUP_HELP) == BST_CHECKED)
		m_pUserData->m_data.m_szDisplayStartupHelp = S_DISPLAY_STARTUP_HELP;
	else
		m_pUserData->m_data.m_szDisplayStartupHelp = "";

	return true;
}

void CConfigDlg::OnBtnLoad(HWND hDlg)
{
	OPENFILENAME st;
	memset( &st, 0, sizeof(st));
	st.lStructSize = sizeof(st);

	char szPath[_MAX_PATH] = { 0 };
	if (m_pUserData->m_data.m_szImagePath.length() != 0)
	{
		strcpy_s( szPath, _MAX_PATH, m_pUserData->m_data.m_szImagePath.c_str() );
	}
	st.lpstrFile = szPath;
	st.nMaxFile  = sizeof(szPath);

	st.hwndOwner = hDlg;
	st.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	char* szFilter = "All supported formats\0*.gif;*.jpg\0GIF file\0*.gif\0JPEG file\0*.jpg\0\0";
	st.lpstrFilter = szFilter;

	st.nFilterIndex = 1;

	const char* szCaption = "Please select a 360x60 GIF or JPG image";
	st.lpstrTitle = szCaption;

	if (GetOpenFileName(&st)) 
	{
		CPicLib pic;
		if ( pic.LoadFromFile( szPath ))
		{
			m_pUserData->m_data.m_szImagePath = szPath;
			m_pic.LoadFromFile( szPath );
			InvalidateRect( hDlg, 0, true);
		}
	}
}

void CConfigDlg::CheckLockStates()
{
	if (m_bLockAddr)
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_ADDR), FALSE);
		::CheckDlgButton(m_hWnd, IDC_LOCK_ADDR, BST_CHECKED);
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_ADDR), TRUE);
		::CheckDlgButton(m_hWnd, IDC_LOCK_ADDR, BST_UNCHECKED);
	}
	if (m_bLockName)
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_NAME), FALSE);
		::CheckDlgButton(m_hWnd, IDC_LOCK_NAME, BST_CHECKED);
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_NAME), TRUE);
		::CheckDlgButton(m_hWnd, IDC_LOCK_NAME, BST_UNCHECKED);
	}
	if (m_bLockPwd)
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PWD), FALSE);
		::CheckDlgButton(m_hWnd, IDC_LOCK_PWD, BST_CHECKED);
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_EDIT_PWD), TRUE);
		::CheckDlgButton(m_hWnd, IDC_LOCK_PWD, BST_UNCHECKED);
	}
	if (m_bLockAdvProps)
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_BTN_ADVANCED), FALSE);
		::CheckDlgButton(m_hWnd, IDC_LOCK_ADVBTN, BST_CHECKED);
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_BTN_ADVANCED), TRUE);
		::CheckDlgButton(m_hWnd, IDC_LOCK_ADVBTN, BST_UNCHECKED);
	}
}

void CConfigDlg::OnBtnChangeIcon(HWND hDlg)
{
	OPENFILENAME st;
	memset(&st, 0, sizeof(st));
	st.lStructSize = sizeof(st);

	char szPath[_MAX_PATH];
	ZeroMemory(szPath, _MAX_PATH);

	st.lpstrFile = szPath;
	st.nMaxFile  = sizeof(szPath);

	st.hwndOwner = hDlg;
	st.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	char* szFilter = "ICON\0*.ico\0\0";
	st.lpstrFilter = szFilter;

	st.nFilterIndex = 1;

	const char* szCaption = "Please select ICON image";
	st.lpstrTitle = szCaption;

	if (GetOpenFileName(&st))
	{
		if (CheckIcon(szPath))
			m_pUserData->m_data.m_szIcon = szPath;
		else
			MessageBox(hDlg, "The Icon file constains images with unsupported dimensions.\nOnly 16x16, 32x32 and 48x48 dimensions are supported.", "Warning", MB_OK | MB_ICONWARNING);
	}
}

void CConfigDlg::OnBtnChangeText(HWND hDlg)
{
	CTextLister *objLister = new CTextLister();
	objLister->Show(hInstanceMod, m_hWnd);
}

typedef struct
{
    BYTE        bWidth;          // Width, in pixels, of the image
    BYTE        bHeight;         // Height, in pixels, of the image
    BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;       // Reserved ( must be 0)
    WORD        wPlanes;         // Color Planes
    WORD        wBitCount;       // Bits per pixel
    DWORD       dwBytesInRes;    // How many bytes in this resource?
    DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
    WORD			 idReserved;   // Reserved (must be 0)
    WORD			 idType;       // Resource Type (1 for icons)
    WORD			 idCount;      // How many images?
    LPICONDIRENTRY   idEntries; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;

typedef struct
{
   BITMAPINFOHEADER   icHeader;      // DIB header
   RGBQUAD         icColors[1];   // Color table
   BYTE            icXOR[1];      // DIB bits for XOR mask
   BYTE            icAND[1];      // DIB bits for AND mask
} ICONIMAGE, *LPICONIMAGE;


bool CConfigDlg::CheckIcon(TCHAR *szPath)
{
	bool bRes = false;
	// open file
	HANDLE hFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) // is it ok?
	{
		DWORD dwBytesRead;
		WORD wReserved;
		WORD wType;
		WORD wCount;

		// Read the Reserved word
		ReadFile(hFile, &wReserved, sizeof(wReserved), &dwBytesRead, NULL);
		// Read the Type word - make sure it is 1 for icons
		ReadFile(hFile, &wType, sizeof(wType), &dwBytesRead, NULL);
		// Read the count - how many images in this file?
		ReadFile(hFile, &wCount, sizeof(wCount), &dwBytesRead, NULL);

		// Allocate IconDir so that idEntries has enough room for idCount elements
		LPICONDIR pIconDir = (LPICONDIR)malloc(sizeof(ICONDIR));
		if (pIconDir)
		{
			ZeroMemory(pIconDir, sizeof(ICONDIR));

			pIconDir->idReserved = wReserved;
			pIconDir->idType = wType;
			pIconDir->idCount = wCount;

			// Allocate memro for entries
			pIconDir->idEntries = (LPICONDIRENTRY)malloc(wCount * sizeof(ICONDIRENTRY));

			// Loop through and read in each image
			if (pIconDir->idCount > 0 && pIconDir->idEntries)
			{
				ZeroMemory(pIconDir->idEntries, wCount * sizeof(ICONDIRENTRY));

				// Read the ICONDIRENTRY elements
				ReadFile(hFile, pIconDir->idEntries, wCount * sizeof(ICONDIRENTRY), &dwBytesRead, NULL);

				bool bOk = true;
				for(int i = 0; i < pIconDir->idCount; i++)
				{
					BYTE bWidth = pIconDir->idEntries[i].bWidth;
					BYTE bHeight = pIconDir->idEntries[i].bHeight;

					bOk = bOk && ((bWidth == 16 && bHeight == 16) || (bWidth == 32 && bHeight == 32) || (bWidth == 48 && bHeight == 48));
					if (!bOk) break;
				}
				bRes = bOk;
			}
			// Clean up icon entries memory
			if (pIconDir->idEntries)
				free(pIconDir->idEntries);
			// Clean up the ICONDIR memory
			free(pIconDir);
		}
		// close file
		CloseHandle(hFile);
	}
	return bRes;
}
