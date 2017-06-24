// UserData.h: interface for the CUserData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERDATA_H__2DDDCEC4_A3F7_4FE3_941C_A44039AC5373__INCLUDED_)
#define AFX_USERDATA_H__2DDDCEC4_A3F7_4FE3_941C_A44039AC5373__INCLUDED_


#include <string>

//class CSerialize;
#include "..\miniWinVNC\ProxyData.h"

class CUserData  
{
public:

	CUserData();

	std::string m_szAddress;
	std::string m_szUsername;
	std::string m_szPassword;
	std::string m_szImagePath;
	std::string m_szPort;
	std::string m_vncPassword;
	std::string m_szIniPath;
	std::string m_vncPwd;

	bool m_bLockAddress;
	bool m_bLockUsername;
	bool m_bLockPassword;
	bool m_bLockAdvProps;
	bool m_bUseMiniGui;

	bool m_bSilent;	//#IR: + : added m_bSilent to CUserData
	bool m_bHelp;	//#IR:100303 +

	CProxyData m_proxy;
	
	BOOL m_bClose;
	static int ReplaceString(std::string &str, char* find, char* replace);
};

#endif // !defined(AFX_USERDATA_H__2DDDCEC4_A3F7_4FE3_941C_A44039AC5373__INCLUDED_)
