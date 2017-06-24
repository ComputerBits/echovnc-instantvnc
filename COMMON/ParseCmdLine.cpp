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
//  ParseCmdLine.cpp: implementation of the CParseCmdLine class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <windows.h>
#include "ParseCmdLine.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParseCmdLine::CParseCmdLine(int argc, const char* argv[])
:	m_argc(argc),
	m_argv(argv),
	m_data(),
	m_bNoRun(false),
	m_bConfig(false),
	m_bDebug(false),
	m_bCheckString(false),
	m_sDebugPath(),
	m_bStartTest(false),
	m_bAutoExitAfterLastViewer(false),
	m_bAutoExitIfNoViewer(false),
	m_bGeneratePassword(false),
	m_bEnableDualMonitors(false),
	m_bUseMiniGui(false),
	m_bAutoRun(false),
	m_bIsHelperTray(false),
	m_bIsSubService(false)
{
}

std::string CParseCmdLine::GetDebugPath()
{
	return m_sDebugPath;
}

const char* CParseCmdLine::GetAddress()const
{ 
	return m_data.m_szAddress.c_str();
}

const char* CParseCmdLine::GetUsername()const
{
	return m_data.m_szUsername.c_str();
}

const char* CParseCmdLine::GetVncPwd() const
{
	return m_data.m_vncPwd.c_str();
}

const char* CParseCmdLine::GetPassword()const
{
	return m_data.m_szPassword.c_str();
}

bool CParseCmdLine::IsAddress()const
{
	return m_data.m_szAddress.length() > 0;
}

bool CParseCmdLine::IsUsername()const
{
	return m_data.m_szUsername.length() > 0;
}

bool CParseCmdLine::IsPassword()const
{
	return m_data.m_szPassword.length() > 0;
}

bool CParseCmdLine::IsImagePath()const
{
	return m_data.m_szImagePath.length() > 0;
}

bool CParseCmdLine::GetNoRun()const
{ 
	return m_bNoRun; 
}

bool CParseCmdLine::GetConfig()const
{
	return m_bConfig;
}

bool CParseCmdLine::GetDebug()const
{
#ifdef _DEBUG
	return true;
#else
	return m_bDebug;
#endif
}

bool CParseCmdLine::GetAutoExitAfterLastViewer()const
{
	return m_bAutoExitAfterLastViewer;
}

bool CParseCmdLine::GetAutoExitIfNoViewer()const
{
	return m_bAutoExitIfNoViewer;
}

bool CParseCmdLine::GetGeneratePassword()
{
	return m_bGeneratePassword;
}

bool CParseCmdLine::GetEnableDualMonitors()
{
	return m_bEnableDualMonitors;
}

bool CParseCmdLine::GetAutoRun()
{
	return m_bAutoRun;
}

bool CParseCmdLine::IsMiniGui()const
{
	return m_bUseMiniGui;
}

bool CParseCmdLine::IsHelperTray()
{
	return m_bIsHelperTray;
}

bool CParseCmdLine::IsSubService()
{
	return m_bIsSubService;
}

static const std::string g_strKnownParameter[] =
{
	"Address=",
	"Username=",
	"Password=",
	"Image=",
	"Haddr=",
	"Huser=",
	"Hpwd=",
	"-lockAddress",
	"-lockUsername",
	"-lockPassword",
	"-lockAdvSettings",
	"-debug",
	"-exitafterlastviewer",
	"-exitifnoviewer",
	"-generatepassword",
	"-enabledualmonitors",
	"-silent", //#IR +
	"-help", //#IR:100303 +
	"UseIni=",
	"vncPwd",
	"-useminigui",
	"-autorun",
	"-helperTray",
	"-subService"
};

static bool FindPosition(const char* sCmdLn, int& rIndexParameter, int& rPosition)
{
	int iCountPar = sizeof(g_strKnownParameter)/sizeof(const std::string);
	for( int iPos = 0; iPos < (int) strlen(sCmdLn); iPos++)
	{
		const char* szCmdLine = sCmdLn + iPos;
		int iLenCmd = strlen(szCmdLine);
		for( int iPar = 0; iPar < iCountPar; iPar++)
		{
			std::string str = g_strKnownParameter[iPar];
			str.insert(0, "\"");
			int iLen = str.length();
			if ( iLenCmd >= iLen )
			{
				if ( memcmp( szCmdLine, str.c_str(), iLen ) == 0 )
				{
					rIndexParameter = iPar;
					rPosition = iPos;
					return true;
				}
			}
		}
	}

	return false;
}

bool CParseCmdLine::ExtractParameter(std::string& rSrcCmdLine, std::string& rParameter )//static
{
	int iFirstParameter;
	int iFirstPos = 0;
	if ( FindPosition( rSrcCmdLine.c_str(), iFirstParameter, iFirstPos) &&
		iFirstPos == 0)
	{
		const char* szNext = rSrcCmdLine.c_str();
		szNext++;

		int iSecondParameter;
		int iSecondPos;
		if ( FindPosition( szNext, iSecondParameter, iSecondPos) &&
			iFirstParameter != iSecondParameter)
		{
			rParameter = std::string( rSrcCmdLine.c_str(), iSecondPos - iFirstPos);
			const char* sz = rSrcCmdLine.c_str();
			sz += rParameter.length() + 1;
			rSrcCmdLine = std::string( sz );
			return true;
		}
		else 
		{
			rParameter = rSrcCmdLine;
			rSrcCmdLine= std::string("");
			return true;
		}
		
	}

	return false;
}

bool CParseCmdLine::Parse()
{
	bool bOK = true;
	for( int n = 1; n < m_argc; n++)
	{
		bOK = false;
		const char* szNext = m_argv[n];
		if (!m_bNoRun && CheckNoRun(szNext))
		{
			bOK = true;
		}
		if (!m_bConfig && CheckConfig(szNext))
		{
			bOK = true;
		}
		if (!m_bDebug && CheckDebug(szNext))
		{
			bOK = true;
		}
		if (!m_bAutoExitAfterLastViewer && CheckAutoExitAfterLastViewer(szNext))
		{
			bOK = true;
		}
		if (!m_bAutoExitIfNoViewer && CheckAutoExitIfNoViewer(szNext))
		{
			bOK = true;
		}
		if (!m_bGeneratePassword && CheckGeneratePassword(szNext))
		{
			bOK = true;
		}
		if (!m_bEnableDualMonitors && CheckEnableDualMonitors(szNext))
		{
			bOK = true;
		}
		if (!m_bUseMiniGui && CheckUseMiniGui(szNext))
		{
			bOK = true;
		}
		//(#IR: + : added 'if' to call CheckSilent()
		if (!m_data.m_bSilent) {
			if (CheckSilent(szNext)) {
				bOK = true;
			}
		}
		//)
		//(#IR:100303: + : added 'if'
		if (!m_data.m_bHelp && CheckHelp(szNext))
		{
			bOK = true;
		}
		//)
		if (!m_bCheckString && CheckString(szNext))
		{
			m_bCheckString = false;
			bOK = true;
		}
		if (CheckLockSettings(szNext))
		{
			bOK = true;
		}
		if (CheckAutoRun(szNext))
		{
			bOK = true;
		}
		if (CheckHelperTray(szNext))
		{
			bOK = true;
		}
		if (CheckSubService(szNext))
		{
			bOK = true;
		}
		if (!bOK)
		{
			break;
		}
	}
	if (bOK)
	{
		CUserData dataFromReg;

		if (m_bAutoRun)
		{
			m_bGeneratePassword = false;
			m_bAutoExitIfNoViewer = false;
			m_bAutoExitAfterLastViewer = false;
		}


		if (!IsAddress())
		{
			if ( strlen ( dataFromReg.m_szAddress.c_str() ) > 0 )
			{
				m_data.m_szAddress = dataFromReg.m_szAddress;
			}
			else
			{
			m_data.m_szAddress = "demo.echovnc.com";
			m_data.m_szPort = "443";
			}
		}
		if (!IsPassword())
		{
			if ( strlen ( dataFromReg.m_szPassword.c_str() ) > 0 )
			{
				m_data.m_szPassword = dataFromReg.m_szPassword;
			}
			else
			{
				m_data.m_szPassword = "demo2010";
			}
		}
		if (!IsUsername())
		{
			if ( strlen ( dataFromReg.m_szUsername.c_str() ) > 0 )
			{
				m_data.m_szUsername = dataFromReg.m_szUsername;
			}
			else
			{
				DWORD len = MAX_COMPUTERNAME_LENGTH + 1;
				char szHostName[MAX_COMPUTERNAME_LENGTH + 1 + 1];
				memset(szHostName, 0, len + 1);

				if (!GetComputerName(szHostName, &len))
				{
					sprintf_s(szHostName, len+1, "PC-%d", GetLastError());
				}

				m_data.m_szUsername = szHostName;
			}
		}

		//(#R +
		//If this switch is set, it checks to see both Address and Password are set.
		//If they are not, it throws an error: 
		//"Sorry...silent mode requires both at least an echoServer address and an echoServer password".
		if (IsSilent()) {
			if (!IsAddress() || !IsPassword()) {	//#break_point
				throw("Sorry...silent mode requires both at least an echoServer address and an echoServer password!");
			}
		} 
		//)
	}
	
	return bOK;
}

bool CParseCmdLine::CheckString( const char* szNext )
{
	m_bCheckString = true;
	std::string str(szNext);
	return CheckSubString(str);
}

// static
bool CParseCmdLine::CheckParameter( const std::string strData, const char* sPrefix, std::string& rStrForSave )
{
	if (m_bStartTest)
		MessageBox(NULL, strData.c_str(), "dfgdfg", MB_OK);
	std::string strCheckPrefix(sPrefix);
	strCheckPrefix += "=";
	if (strData.length() > 0)
		if (strData.at(0) == '\"')
			strCheckPrefix.insert(0, "\"");
	if (m_bStartTest)
		MessageBox(NULL, strData.c_str(), sPrefix, MB_OK);
	int iLenText = strData.length();
	int iLenPref = strCheckPrefix.length();
	if ( iLenText >= iLenPref )
	{
		if ( memcmp( strData.c_str(), strCheckPrefix.c_str(), iLenPref ) == 0 )
		{
			if ( iLenText >= iLenPref )
			{
				rStrForSave = std::string( strData.c_str() + iLenPref);
				if (rStrForSave.length() > 0)
					if (rStrForSave.at(rStrForSave.length() - 1) == '\"')
						rStrForSave.erase(rStrForSave.length() - 1);
			}
			else
			{
				rStrForSave.empty();
			}

			return true;
		}
	}

	return false;
}

bool CParseCmdLine::CheckSubString( const std::string strNext )
{
	//#IR:100303: * : modified code block to parse port number as well
	//previous approach ...
	//if (CheckParameter( strNext, "Address", m_data.m_szAddress ))
	//	return true;
	//new approach ...
	if (CheckParameter( strNext, "Address", m_data.m_szAddress )) {
		//(#IR:100303: + : parse and extract port (HTTP port) number as well
		if (m_data.m_szAddress.length() > 0) {
			const char* p = strchr(m_data.m_szAddress.c_str(), ':');
			if (NULL != p) {
				std::string sAddress = m_data.m_szAddress.substr(0, p - m_data.m_szAddress.c_str());
				//#ifdef _DEBUG
				//try {
				//	UINT portHTTP = 0;
				//	portHTTP = atoi(p+1);
				//} catch(...) {}
				//#endif
				m_data.m_szPort = ++p;
				m_data.m_szAddress = sAddress;
			}
			else {
				m_data.m_szPort = "1328";
			}
		}
		//)
		return true;
	}

	if (CheckParameter( strNext, "Username", m_data.m_szUsername ))
		return true;

	if (CheckParameter( strNext, "Password", m_data.m_szPassword ))
		return true;

	if (CheckParameter( strNext, "Image", m_data.m_szImagePath ))
		return true;

	if (CheckParameter( strNext, "Haddr", m_data.m_proxy.m_strHost ))
		return true;

	if (CheckParameter( strNext, "Huser", m_data.m_proxy.m_strUsername ))
		return true;

	if (CheckParameter( strNext, "Hpwd", m_data.m_proxy.m_strPassword ))
		return true;

	if (CheckParameter( strNext, "UseIni", m_data.m_szIniPath ))
		return true;

	if (CheckParameter(strNext, "vncPwd", m_data.m_vncPwd))
		return true;

	return false;
}


//#IR + added region pragma
#pragma region Check

bool CParseCmdLine::CheckNoRun(const char* szNext)
{
	int k1 = 0;
	int k2 = strlen(szNext);
	if (k2 > 0)
	{
		if (szNext[0] == '\"')
			k1++;
		if (szNext[k2 - 1] == '\"')
			k2--;
	}

	char szNextcpy[100];
	memset(szNextcpy, 0, 100);
	memcpy(szNextcpy, szNext + k1, k2 - 1);

	m_bNoRun = (_stricmp( "-norun", szNextcpy ) == 0 );

	return m_bNoRun;
}

bool CParseCmdLine::CheckDebug(const char* szNext)
{
	m_bDebug = CheckParameter(std::string(szNext), "-debug", m_sDebugPath);
	return m_bDebug;
}

bool CParseCmdLine::CheckAutoRun(const char* szNext)
{
	if (!m_bAutoRun)
	{
		m_bAutoRun = _stricmp(szNext, "-autorun") == 0;
		return m_bAutoRun;
	}
	return false;
}

bool CParseCmdLine::CheckHelperTray(const char *szNext)
{
	if (!m_bIsHelperTray)
	{
		m_bIsHelperTray = _stricmp(szNext, "-helperTray") == 0;
		return m_bIsHelperTray;
	}
	return false;
}

bool CParseCmdLine::CheckSubService(const char *szNext)
{
	if (!m_bIsSubService)
	{
		m_bIsSubService = _stricmp(szNext, "-subService") == 0;
		return m_bIsSubService;
	}
	return false;
}

bool CParseCmdLine::CheckConfig(const char* szNext)
{
	int k1 = 0;
	int k2 = strlen(szNext);
	if (k2 > 0)
	{
		if (szNext[0] == '\"')
			k1++;
		if (szNext[k2 - 1] == '\"')
			k2--;
	}

	char szNextcpy[100];
	memset(szNextcpy, 0, 100);
	memcpy(szNextcpy, szNext + k1, k2 - 1);

	m_bConfig = (_stricmp( "-config", szNextcpy ) == 0 );

	return m_bConfig;
}

bool CParseCmdLine::CheckAutoExitAfterLastViewer(const char* szNext)
{
	int k1 = 0;
	int k2 = strlen(szNext);
	if (k2 > 0)
	{
		if (szNext[0] == '\"')
			k1++;
		if (szNext[k2 - 1] == '\"')
			k2--;
	}

	char szNextcpy[100];
	memset(szNextcpy, 0, 100);
	memcpy(szNextcpy, szNext + k1, k2 - 1);

	m_bAutoExitAfterLastViewer = (_stricmp( "-exitafterlastviewer", szNextcpy ) == 0 );

	return m_bAutoExitAfterLastViewer;
}

bool CParseCmdLine::CheckUseMiniGui(const char* szNext)
{
	int k1 = 0;
	int k2 = strlen(szNext);
	if (k2 > 0)
	{
		if (szNext[0] == '\"')
			k1++;
		if (szNext[k2 - 1] == '\"')
			k2--;
	}

	char szNextcpy[100];
	memset(szNextcpy, 0, 100);
	memcpy(szNextcpy, szNext + k1, k2 - 1);

	m_bUseMiniGui = (_stricmp( "-useminigui", szNextcpy ) == 0 );

	return m_bUseMiniGui;
}

bool CParseCmdLine::CheckAutoExitIfNoViewer(const char* szNext)
{
	int k1 = 0;
	int k2 = strlen(szNext);
	if (k2 > 0)
	{
		if (szNext[0] == '\"')
			k1++;
		if (szNext[k2 - 1] == '\"')
			k2--;
	}

	char szNextcpy[100];
	memset(szNextcpy, 0, 100);
	memcpy(szNextcpy, szNext + k1, k2 - 1);

	m_bAutoExitIfNoViewer = (_stricmp( "-exitifnoviewer", szNextcpy ) == 0 );

	return m_bAutoExitIfNoViewer;
}

bool CParseCmdLine::CheckGeneratePassword(const char* szNext)
{
	int k1 = 0;
	int k2 = strlen(szNext);
	if (k2 > 0)
	{
		if (szNext[0] == '\"')
			k1++;
		if (szNext[k2 - 1] == '\"')
			k2--;
	}

	char szNextcpy[100];
	memset(szNextcpy, 0, 100);
	memcpy(szNextcpy, szNext + k1, k2 - 1);

	m_bGeneratePassword = (_stricmp( "-generatepassword", szNextcpy ) == 0 );
	return m_bGeneratePassword;
}

bool CParseCmdLine::CheckEnableDualMonitors(const char* szNext)
{
	int k1 = 0;
	int k2 = strlen(szNext);
	if (k2 > 0)
	{
		if (szNext[0] == '\"')
			k1++;
		if (szNext[k2 - 1] == '\"')
			k2--;
	}

	char szNextcpy[100];
	memset(szNextcpy, 0, 100);
	memcpy(szNextcpy, szNext + k1, k2 - 1);

	m_bEnableDualMonitors = (_stricmp( "-enabledualmonitors", szNextcpy ) == 0 );
	return m_bEnableDualMonitors;
}

bool CParseCmdLine::CheckLockSettings(const char* szNext)
{
	bool bResult = false;

	int k1 = 0;
	int k2 = strlen(szNext);
	if (k2 > 0)
	{
		if (szNext[0] == '\"')
			k1++;
		if (szNext[k2 - 1] == '\"')
			k2--;
	}

	char szNextcpy[100];
	memset(szNextcpy, 0, 100);
	memcpy(szNextcpy, szNext + k1, k2 - 1);

	if (_stricmp("-lockAddress", szNextcpy) == 0) 
	{
		m_data.m_bLockAddress = true;
		bResult = true;
	}
	else if (_stricmp("-lockUsername", szNextcpy) == 0)
	{
		m_data.m_bLockUsername = true;
		bResult = true;
	}
	else if (_stricmp("-lockPassword", szNextcpy) == 0)
	{
		m_data.m_bLockPassword = true;
		bResult = true;
	}
	else if (_stricmp("-lockAdvSettings", szNextcpy) == 0)
	{
		m_data.m_bLockAdvProps = true;
		bResult = true;
	}

	return bResult;
}

//#IR: + : added method CParseCmdLine::CheckSilent()
bool CParseCmdLine::CheckSilent(const char* szNext)
{
//#IR:100315: + : new version of the CParseCmdLine::CheckSilent()
	if (NULL == szNext) {
		return false;
	}
	int len = strlen(szNext);
	if (0 == len) {
		return false;
	}
	m_data.m_bSilent = (0 == _stricmp( "-silent", szNext));
	return m_data.m_bSilent;

//#IR: the previous version was implemeneted with the same (bad) approach like other Check***() methods in the original sources
	//int k1 = 0;
	//int k2 = strlen(szNext);
	//if (k2 > 0) {
	//	if (szNext[0] == '\"') k1++;
	//	if (szNext[k2 - 1] == '\"')	k2--;
	//}

	//char szNextcpy[100];
	//memset(szNextcpy, 0, 100);
	//memcpy(szNextcpy, szNext + k1, k2);

	//m_data.m_bSilent = (0 == _stricmp( "-silent", szNextcpy ));

	//return m_data.m_bSilent;
}

//#IR:100303: + : added method
bool CParseCmdLine::CheckHelp(const char* szNext)
{
	int k1 = 0;
	int k2 = strlen(szNext);
	if (k2 > 0) {
		if (szNext[0] == '\"') k1++;
		if (szNext[k2 - 1] == '\"')	k2--;
	}

	char szNextcpy[100];
	memset(szNextcpy, 0, 100);
	memcpy(szNextcpy, szNext + k1, k2);

	m_data.m_bHelp = (0 == _stricmp( "-help", szNextcpy ));

	return m_data.m_bHelp;
}

#pragma endregion // Check
