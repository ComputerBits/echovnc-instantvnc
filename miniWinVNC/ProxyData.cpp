// ProxyData.cpp: implementation of the CProxyData class.
//
//////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include "ProxyData.h"
#include "Resource.h"
#include "Localization.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


static CProxyDataDlg* g_pDlg = 0;

LRESULT CALLBACK ProxyDataDlgProc( HWND hDlg,UINT iMsg,WPARAM wParam,LPARAM lParam);

//static 
bool CProxyDataDlg::AskProxyData( CProxyData* pData, HINSTANCE hInstance, HWND hParent )
{
	g_pDlg = new CProxyDataDlg( pData );

	int iRes = DialogBox( 
		hInstance, 
		MAKEINTRESOURCE(IDD_HOST_DATA), 
		hParent,
		( DLGPROC)ProxyDataDlgProc);

	delete g_pDlg;
	g_pDlg = 0;

	return (iRes == IDOK ) ? true:false;
}


LRESULT CALLBACK ProxyDataDlgProc( HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		case WM_INITDIALOG:
		{
			g_pDlg->OnInitDialog(hDlg);
			break;
		}

		case WM_COMMAND:
		{
			UINT uiCommand = LOWORD(wParam);
			switch(uiCommand)
			{
				case IDOK:
				{
					if (g_pDlg->CheckData())
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


CProxyDataDlg::CProxyDataDlg(CProxyData* pData)
:	m_pData(pData),
	m_hWnd(0),
	m_hWndHost(0),
	m_hWndUser(0)
{
}

#define _MAX_LEN 100
void CProxyDataDlg::OnInitDialog(HWND hDlg)
{
	m_hWnd = hDlg;

	::SetWindowText(::GetDlgItem( hDlg, IDC_STATIC41060), sz_IDS_PROXYHOST41060);
	::SetWindowText(::GetDlgItem( hDlg, IDC_STATIC41061), sz_IDS_PROXYUSER41061);
	::SetWindowText(::GetDlgItem( hDlg, IDC_STATIC41062), sz_IDS_PROXYPASS41062);
	::SetWindowText(::GetDlgItem( hDlg, IDOK), sz_IDS_OK41058);
	::SetWindowText(::GetDlgItem( hDlg, IDCANCEL), sz_IDS_Cancel41059);

	m_hWndHost = ::GetDlgItem( m_hWnd, IDC_EDIT_PROXY_HOST);
	::SetWindowText( m_hWndHost, m_pData->m_strHost.c_str() );
	SendMessage( m_hWndHost,  EM_LIMITTEXT, _MAX_LEN, 0 );

	m_hWndUser = ::GetDlgItem( m_hWnd, IDC_EDIT_PROXY_USERNAME);
	::SetWindowText( m_hWndUser, m_pData->m_strUsername.c_str() );
	SendMessage( m_hWndUser,  EM_LIMITTEXT, _MAX_LEN, 0 );

	m_hWndPassword = ::GetDlgItem( m_hWnd, IDC_EDIT_PROXY_PASSWORD);
	::SetWindowText( m_hWndPassword, m_pData->m_strPassword.c_str() );
	SendMessage( m_hWndPassword,  EM_LIMITTEXT, _MAX_LEN, 0 );
}


bool CProxyDataDlg::CheckData()
{
	char szHost[_MAX_LEN + 1] = {0};
	SendMessage( m_hWndHost, WM_GETTEXT, _MAX_LEN, (LPARAM)szHost );
//	if ( szHost[0] == 0 )
//	{
//		::MessageBox( m_hWnd, "Invalid Host", "Proxy Data", MB_OK);
//		return false;
//	}

	char szUser[_MAX_LEN + 1] = {0};
	SendMessage( m_hWndUser, WM_GETTEXT, _MAX_LEN, (LPARAM)szUser );
//	if ( szUser[0] == 0 )
//	{
//		::MessageBox( m_hWnd, "Invalid Username", "Proxy Data", MB_OK);
//		return false;
//	}

	char szPassword[_MAX_LEN + 1] = {0};
	SendMessage( m_hWndPassword, WM_GETTEXT, _MAX_LEN, (LPARAM)szPassword );
//	if ( szUser[0] == 0 )
//	{
//		::MessageBox( m_hWnd, "Invalid Password", "Proxy Data", MB_OK);
//		return false;
//	}

	m_pData->m_strHost = szHost;
	m_pData->m_strUsername = szUser;
	m_pData->m_strPassword = szPassword;
	return true;
}
