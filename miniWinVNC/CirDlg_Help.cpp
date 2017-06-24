///////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of CirDlg_Help class.
//
// @history:
//	Created/Added by Igor Rumyantsev, 2010-03-03 (100303)
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "CirDlg_Help.h"

#include "Localization.h"

#include "..\Common\irUtility.h"


static CirDlg_Help* g_pDlg = 0;
static HINSTANCE hInstanceMod = 0;


std::string g_sHelp = 
	"The application accepts the following command-line parameter=value options and switches:\n\n"
	"Address=<echoServer address>\n"
	"Username=<echoServer login name>\n"
	"Password=<echoServer login password>\n"
	"Image=<image>\n"
	"Haddr=<details of proxy host>\n"
	"Huser=<details of proxy user>\n"
	"Hpwd=<details of proxy password>\n"
	"-lockAddress : lock the address field\n"
	"-lockUsername :lock the username field\n"
	"-lockPassword :lock the password field\n"
	"-lockAdvSettings :lock the advanced setting button field\n"
	"-debug : enables logging\n"
	"-exitafterlastviewer : tell to exit in 30 sec after last viewer disconnects.\n"
	"-exitifnoviewer : tell to exit in 30 sec in no viewer connection.\n"
	"-help : to show this help and exit\n"
	"-silent : tell to run in silent (no GUI) mode. Both Address and Password options must be specified for that.\n\n"
	"Important:\n"
	"All parameters are case sensitive and the switch –silent must be the last one.";


LRESULT CALLBACK HelpDlgProc( HWND hDlg,UINT iMsg,WPARAM wParam,LPARAM lParam);


CirDlg_Help::CirDlg_Help(const char* pszHelp, int id)
{
	if (NULL != pszHelp) {
		_sHelp = pszHelp;
	}
	else {
		_sHelp = g_sHelp;
	}
	_idDlg = id;
}


CirDlg_Help::~CirDlg_Help(void)
{
}


//static
bool CirDlg_Help::ShowHelp(const char* pszHelp, HINSTANCE hInstance)
{
	hInstanceMod = hInstance;
	g_pDlg = new CirDlg_Help(pszHelp);
	
	//(#IR +
	if (NULL == g_pDlg) {
		return false;
	}
//	g_pDlg->m_pic.SetInstanceMod(hInstance);
	//)

	int iRes = DialogBox( 
		hInstance, 
		MAKEINTRESOURCE(g_pDlg->GetIDD()), 
		0,//m_hWnd,
		( DLGPROC)HelpDlgProc);

	delete g_pDlg;
	g_pDlg = 0;

	return (iRes == IDOK ) ? true:false;
}


LRESULT CALLBACK HelpDlgProc( HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		//case WM_PAINT:
		//{
		//	PAINTSTRUCT ps;
		//	HDC hDC = BeginPaint( hDlg, &ps);
		//	g_pDlg->OnPaint( hDC );
		//	EndPaint( hDlg, &ps);
		//	break;
		//}

		case WM_INITDIALOG:
		{
			g_pDlg->OnInit(hDlg);
			break;
		}

		case WM_COMMAND:
		{
			UINT uiCommand = LOWORD(wParam);
			switch(uiCommand)
			{
				case IDOK:
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


void CirDlg_Help::OnInit( HWND hDlg )
{
	HWND hWnd = ::GetDlgItem(hDlg, IDC_ST_TEXT);
	if (NULL != hWnd) {
		::SetWindowText(hWnd, _sHelp.c_str());
	}
	//if (!m_pic.IsLoad()) {
	//	m_pic.LoadFromRes( IDR_BANNER );
	//}
	Utility::CenterWindow(hDlg);
}


//void CirDlg_Help::OnPaint( HDC hDC )
//{
	//if (m_pic.IsLoad())
	//{
	//	RECT rc;
	//	rc.left = 9;
	//	rc.top = 4;
	//	rc.right = rc.left + 360;//uiWidth;
	//	rc.bottom = rc.top + 60;//uiHeight;
	//	m_pic.Draw( hDC, rc);
	//}
//}



