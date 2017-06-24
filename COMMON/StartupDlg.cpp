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
//////////////////////////////////////////////////////////////////////
//
//  StartupDlg.cpp: implementation of the StartupDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "StartupDlg.h"

#include "stdio.h"
#include "..\miniWinVNC\Localization.h"

#include "irUtility.h"	//#IR:100303 +

//extern bool	g_bSilentMode;	//#IR:100315 +

StartupDlg::StartupDlg(CUserData* pUserData) : m_pUserData(pUserData)
{
	m_hWnd = NULL;
	m_hImgLst = NULL;
	m_hTextLabel = NULL;
	m_szText[0] = NULL;
	m_bContinue = true;
	m_banner.SetInstanceMod(GetModuleHandle(0));
}

StartupDlg::~StartupDlg()
{
	destroy();
}

bool 
StartupDlg::show(char *szText)
{
	if (!loadImgLst()) return false;

	if (m_hWnd != NULL) return false;

	if (!m_banner.IsLoad())
	{
		if (m_pUserData->m_szImagePath.length() != 0)
		{
			m_banner.LoadFromFile(m_pUserData->m_szImagePath.c_str());
		}

		if (!m_banner.IsLoad())
		{
			m_banner.LoadFromRes(IDR_BANNER);
		}
	}

	m_hWnd = CreateDialogParam(GetModuleHandle(0), 
							   MAKEINTRESOURCE(IDD_STARTUP), 
							   NULL, 
							   (DLGPROC) StartupDlgProc, 
							   (LPARAM) this);

	if (m_hWnd == NULL) return false;

	m_hTextLabel = GetDlgItem(m_hWnd, IDC_STATIC_ABOUT);

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	m_stepNum = 1;
	strcpy_s(m_szText, _MAX_PATH, szText);
	show(m_stepNum);
	
	return true;
}

bool 
StartupDlg::next(char *szText)
{
	if (m_hWnd == NULL) return false;
	if (!m_bContinue) return false;

	m_stepNum++;
	if (m_stepNum > 4) m_stepNum = 4;

	if (m_stepNum < 4)
	{
		m_szText[0] = '\r';
		m_szText[1] = '\n';
		strcpy_s(m_szText + 2, _MAX_PATH - 2, szText);
	}
	else
	{
		strcpy_s(m_szText, _MAX_PATH, szText);
	}
	show(m_stepNum);

	return m_bContinue;
}

void 
StartupDlg::destroy()
{
	if (DestroyWindow(m_hWnd)) m_hWnd = NULL;
	if (ImageList_Destroy(m_hImgLst)) m_hImgLst = NULL;
	m_stepNum = 1;
}

bool 
StartupDlg::show(int step)
{
	if (m_banner.IsLoad())
	{
		RECT rc;
		rc.left = 0;
		rc.top = 2;
		rc.right = rc.left + 360;
		rc.bottom = rc.top + 60;
		m_banner.Draw(GetDC(m_hWnd), rc);
	}    

	SetWindowText(m_hTextLabel, m_szText);
	UpdateWindow(m_hWnd);

	switch (step)
	{
	case 1:
		showIcons(0, 1, 1, 1);
		break;
	case 2:
		showIcons(1, 0, 1, 1);
		break;
	case 3:
		showIcons(1, 1, 0, 1);
		break;
	case 4:
		showIcons(1, 1, 1, 0);
		SetDlgItemText(m_hWnd, IDCANCEL, sz_IDS_OK41058);
		EnableMenuItem(GetSystemMenu(m_hWnd, false), SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
		break;
	default:		
		return false;
	}
	
	return true;
}

void
StartupDlg::showIcons(int icon1, int icon2, int icon3, int icon4)
{
	int y = 125; 

	ImageList_Draw(m_hImgLst, icon1, GetDC(m_hWnd), 12, y, ILD_NORMAL);
	ImageList_Draw(m_hImgLst, icon2, GetDC(m_hWnd), 109, y, ILD_NORMAL);
	ImageList_Draw(m_hImgLst, icon3, GetDC(m_hWnd), 206, y, ILD_NORMAL);
	ImageList_Draw(m_hImgLst, icon4, GetDC(m_hWnd), 303, y, ILD_NORMAL);
}

bool
StartupDlg::loadImgLst()
{
	m_hImgLst = ImageList_Create(32, 32, ILC_COLOR24, 2, 2);

	if (m_hImgLst == NULL) return false;

	UINT uiIDs [] =
	{
		IDR_ICON_BLUE_EYE,
		IDR_ICON_BLACK_EYE,
	};

	bool bResult = true;

	for( int i = 0; i < sizeof(uiIDs)/sizeof(UINT); i++)
	{
		if (ImageList_AddIcon(m_hImgLst, LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(uiIDs[i]))) < 0) {
			bResult = false;
			break;
		}
	}

	if (!bResult) {
		ImageList_Destroy(m_hImgLst);
	}

	return bResult;
}

void
StartupDlg::onButton()
{
	if (m_stepNum < 4) {
		m_bContinue = false;
		SetWindowText(m_hTextLabel, "Wait while last operation will be finished...");
	}

	if (m_stepNum == 4) {
		m_bContinue = true;
		destroy();
	}
}

void 
StartupDlg::waitAndProcessDlgMsgs(UINT timeInterval)
{
	UINT uIDTimer = 333;
	bool bTimer = false;
	if (timeInterval > 0) {
		if (SetTimer(m_hWnd, uIDTimer, timeInterval, NULL) != 0) {
			bTimer = true;
		} else {
			m_bContinue = false;
			return;
		}
	}

    while (true)
    {
        MSG msg; 

		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
			if (msg.message == WM_TIMER) {
				if (bTimer) KillTimer(m_hWnd, uIDTimer);
				DispatchMessage(&msg);
				return;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if ((!isContinue()) || (timeInterval == 0)) return;
    }
}

BOOL CALLBACK 
StartupDlg::StartupDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	StartupDlg *_this = (StartupDlg *) GetWindowLong(hDlg, GWL_USERDATA);

	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemText(hDlg, IDCANCEL, sz_IDS_Cancel41059);
			SetWindowLong(hDlg, GWL_USERDATA, (LONG) lParam);
			EnableMenuItem(GetSystemMenu(hDlg, false), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
			Utility::CenterWindow(hDlg, false, 0, -50); //#IR:100303 +
			//if (g_bSilentMode) { //#IR:100315 + :added this 'if'
			//	LONG lStyle = ::GetWindowLong(hDlg, GWL_STYLE);
			//	if (0 != lStyle) {
			//		lStyle &= ~WS_VISIBLE;
			//		lStyle = ::SetWindowLong(hDlg, GWL_STYLE, lStyle);
			//	}
			//}
			return FALSE;

		case WM_PAINT:
			_this->show(_this->m_stepNum);
			return FALSE;

		case WM_COMMAND:
		{
			UINT uiCommand = LOWORD(wParam);
			switch(uiCommand)
			{
				case IDCANCEL:
					_this->onButton();
					return FALSE;
			}
			break;
		}

		case WM_CLOSE:
			_this->onButton();
			return FALSE;
	}
	return FALSE;
}
