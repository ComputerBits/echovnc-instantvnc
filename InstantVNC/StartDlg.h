#pragma once

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include "resource.h"

class CStartDlg
{
public:
    static int ShowStartup(HINSTANCE);
    void CStartDlg::OnInitDialog(HWND);
	void OnPaint(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	int result;

private:
	HFONT hFontTitle;
	HFONT hFontMessage;
	HFONT hFontBold;
    CStartDlg(void);
    ~CStartDlg(void);
};

