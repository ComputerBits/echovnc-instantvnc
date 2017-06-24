///////////////////////////////////////////////////////////////////////////////////////////////////
// Interface/declarations of CirDlg_Help class.
//
// @history:
//	Created/Added by Igor Rumyantsev, 2010-03-03 (100303)
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(CirDlg_Help_H_INCLUDED)
#define CirDlg_Help_H_INCLUDED

#pragma once

#include "windows.h"

#include <string>

#include "resource.h"


class CirDlg_Help
{
public:
	CirDlg_Help(const char* pszHelp=NULL, int id=IDD_Help);
	virtual ~CirDlg_Help(void);

private:
	UINT _idDlg;	// dialog id in resources
	std::string _sHelp;

public:
	UINT GetIDD() { return _idDlg; }
	
	static bool ShowHelp(const char* pszHelp=NULL, HINSTANCE hInstance = 0);

	void OnInit( HWND hDlg );
	//void OnPaint( HDC hDC );

};

//void CenterWindow(HWND hWnd, bool bRedraw=false, int iXOffset=0, int iYOffset=0);


#endif //CirDlg_Help_H_INCLUDED
