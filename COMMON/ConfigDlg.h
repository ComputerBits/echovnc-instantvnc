// ConfigDlg.h: interface for the CConfigDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIGDLG_H__FA1AC7F2_9111_433C_98E9_39C1D245FEC9__INCLUDED_)
#define AFX_CONFIGDLG_H__FA1AC7F2_9111_433C_98E9_39C1D245FEC9__INCLUDED_

#include "..\PicLib\PicLib.h"

class CUserData;

class CConfigDlg  
{
public:

	static bool AskRun( CUserData* pData, HINSTANCE hInstance = 0 );

	CConfigDlg(CUserData* pUserData);

	virtual ~CConfigDlg();

	void OnInitDialog( HWND hDlg );

	void OnLButtonDown( LPARAM lParam );

	void OnMouseMove( LPARAM lParam );

	int SetCursor();

	DWORD OnCtlColorStatic( WPARAM wParam, LPARAM lParam );

	bool CheckData( HWND hDlg );

	void OnPaint( HDC hDC );

	CUserData* m_pUserData;

	bool CheckPort(char *szAddr);

protected:

	CPicLib	m_pic;

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
};

#endif // !defined(AFX_CONFIGDLG_H__FA1AC7F2_9111_433C_98E9_39C1D245FEC9__INCLUDED_)
