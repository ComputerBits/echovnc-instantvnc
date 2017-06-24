//////////////////////////////////////////////////////////////////////
//
// StartupDlg.h: interface for the StartupDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_STARTUPDLG_)
#define _STARTUPDLG_

#include <windows.h>
#include <commctrl.h>
#include "UserData.h" 

#include <..\miniWinVNC\resource.h>
#include "..\PicLib\PicLib.h"

class StartupDlg  
{
public:
	CUserData* m_pUserData;

	StartupDlg(CUserData* pUserData);
	~StartupDlg();
	
	void waitAndProcessDlgMsgs(UINT timeInterval);
	
	static BOOL CALLBACK StartupDlgProc(HWND hDlg, UINT iMsg, 
										WPARAM wParam, LPARAM lParam);
	
	bool show(char *szText);
	bool next(char *szText);

	void destroy();

	bool isContinue() { return m_bContinue; }

private:
	CPicLib	m_banner;
	HIMAGELIST m_hImgLst;
	HWND m_hWnd;
	HWND m_hTextLabel;
	bool m_bContinue;

	void onButton();
	bool loadImgLst();
	bool show(int step);
	void showIcons(int icon1, int icon2, int icon3, int icon4);

	char m_szText[_MAX_PATH];

	int m_stepNum;
};

#endif // !defined(_STARTUPDLG_)
