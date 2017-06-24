// ConfigDlg.h: interface for the CConfigDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIGDLG_H__FA1AC7F2_9111_433C_98E9_39C1D245FEC9__INCLUDED_)
#define AFX_CONFIGDLG_H__FA1AC7F2_9111_433C_98E9_39C1D245FEC9__INCLUDED_


class CSerializeData;

#include "..\PicLib\PicLib.h"

class CConfigDlg  
{
public:

	static bool AskRun( CSerializeData* pData, HINSTANCE hInstance = 0 );

	CConfigDlg(CSerializeData* pUserData);

	virtual ~CConfigDlg();

	void OnInitDialog( HWND hDlg );

	void OnLButtonDown( LPARAM lParam );

	void OnMouseMove( LPARAM lParam );

	int SetCursor();

	DWORD OnCtlColorStatic( WPARAM wParam, LPARAM lParam );

	bool CheckData( HWND hDlg );

	void OnPaint( HDC hDC );

	void OnBtnLoad(HWND hDlg);

	void OnBtnChangeIcon(HWND hDlg);
	void OnBtnChangeText(HWND hDlg);

	void CheckLockStates();

	CSerializeData* m_pUserData;

protected:

	HWND m_hWnd;
	HWND m_hwndToolTip;
	void CreateToolTip();

	CPicLib m_pic;

	static const int m_iMaxLenString;

	HWND m_hWndAboutWinVnc;
	RECT m_rcAboutWinVnc;

	HWND m_hWndAbouteEchoWare;
	RECT m_rcAbouteEchoWare;

	HBRUSH m_Brush;

	HWND m_hAddress;
	HWND m_hPassword;
	HWND m_hUserName;

	bool IsInsideHyperLink( LPARAM lParam)const;

	void CheckInside( LPARAM lParam);
	bool m_bIsInside;

	bool CheckIcon(TCHAR *szPath);

public:
	bool m_bLockAddr;
	bool m_bLockName;
	bool m_bLockPwd;
	bool m_bLockAdvProps;
};

#endif // !defined(AFX_CONFIGDLG_H__FA1AC7F2_9111_433C_98E9_39C1D245FEC9__INCLUDED_)
