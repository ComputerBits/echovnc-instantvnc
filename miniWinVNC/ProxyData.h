// ProxyData.h: interface for the CProxyData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROXYDATA_H__A8FCC714_CF89_427B_92EE_17D2126C73D1__INCLUDED_)
#define AFX_PROXYDATA_H__A8FCC714_CF89_427B_92EE_17D2126C73D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

class CProxyData  
{
public:

	std::string m_strHost;
	std::string m_strPort;
	std::string m_strUsername;
	std::string m_strPassword;

	CProxyData()
	:	m_strHost(""),
		m_strPort(""),
		m_strUsername(""),
		m_strPassword("")
	{
	}
};


class CProxyDataDlg
{
public:

	static bool AskProxyData( CProxyData* pData, HINSTANCE hInstance, HWND hParent );

	CProxyDataDlg(CProxyData* pData);

	void OnInitDialog(HWND hDlg);

	bool CheckData();

protected:
	CProxyData* m_pData;
	HWND m_hWnd;
	HWND m_hWndHost;
	HWND m_hWndUser;
	HWND m_hWndPassword;
};

#endif // !defined(AFX_PROXYDATA_H__A8FCC714_CF89_427B_92EE_17D2126C73D1__INCLUDED_)
