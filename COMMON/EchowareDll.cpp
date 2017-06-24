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
//  EchowareDll.cpp: implementation of the CEchowareDll class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "EchowareDll.h"
#include "UserData.h"
#include "CommonBase.h"
#include "StartupDlg.h"
#include <stdio.h>
#include "..\miniWinVNC\Localization.h"

#define _MSG_LEN	512

extern CUserData* g_pUserData;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEchowareDll::CEchowareDll()
:	m_hModEchowareProxyDll(0),
	m_pAutoConnect(0),
	m_pProxyInfoObject(0),
	m_pDeleteDllProxyInfoObject(0),
	m_pInitializeProxyDll(0),
	m_pConnectProxy(0),
	m_pDisconnectProxy(0),
	m_pSendDisconnectMsgToAllProxy(0),
	m_pSetLoggingOptions(0),
	m_pSetPortForOffLoadingData(0),
	m_pEstablishNewDataChannel(0),
	m_pGetDllVersion(0),
	m_pSetLocalProxyInfo(0),
    m_pSetEncryptionLevel(0)
{
}


CEchowareDll::~CEchowareDll()
{
	if (m_hModEchowareProxyDll)
	{
		::FreeLibrary(m_hModEchowareProxyDll);
//#ifdef _DEBUG
		//printf("\n");
//		printf("Unload echoWare DLL\n");
		//printf("<=Unload echoWare DLL returned\n");
//#endif
	}
}


bool CEchowareDll::Init()
{
	m_hModEchowareProxyDll = LoadLibrary("Echoware.dll");
	if (!m_hModEchowareProxyDll)
	{
#ifdef _DEBUG
		printf("Failed to LoadLibrary.\n");
#endif
		return false;
	}

	m_pAutoConnect = (LPFN_PROXYDLL_AUTOCONNECT)
		GetProcAddress(m_hModEchowareProxyDll,"AutoConnect");
	if (m_pAutoConnect == NULL) 
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: AutoConnect is NULL");
#endif
		return false;
	}

	m_pProxyInfoObject = (LPFN_PROXYDLL_GET_PROXY_INFO_OBJECT)
		GetProcAddress(m_hModEchowareProxyDll,"CreateProxyInfoClassObject");
	if (m_pProxyInfoObject == NULL) 
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: ProxyInfoObject is NULL");
#endif
		return false;
	}

	m_pDeleteDllProxyInfoObject = (LPFN_PROXYDLL_DELETE_PROXY_INFO_OBJECT)
		GetProcAddress(m_hModEchowareProxyDll,"DeleteProxyInfoClassObject");
	if (m_pDeleteDllProxyInfoObject == NULL) 
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: DeleteDllProxyInfoObject is NULL");
#endif
		return false;
	}

	m_pInitializeProxyDll = (LPFN_PROXYDLL_INITIALIZE_PROXYDLL)
		GetProcAddress(m_hModEchowareProxyDll,"InitializeProxyDll");
	if (m_pInitializeProxyDll == NULL) 
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: InitializeProxyDll is NULL");
#endif
		return false;
	}

	m_pConnectProxy = (LPFN_PROXYDLL_CONNECTPROXY)
		GetProcAddress(m_hModEchowareProxyDll,"ConnectProxy");
	if (m_pConnectProxy == NULL)
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: ConnectProxy is NULL");
#endif
		return false;
	}

	m_pDisconnectProxy = (LPFN_PROXYDLL_DISCONNECT_PROXY)
		GetProcAddress(m_hModEchowareProxyDll,"DisconnectProxy");						
	if (m_pDisconnectProxy == NULL)
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: DisconnectProxy is NULL");
#endif
		return false;
	}

	m_pSendDisconnectMsgToAllProxy = (LPFN_PROXYDLL_SEND_DISCONNECT_MSG_TO_ALL_PROXY)
		GetProcAddress(m_hModEchowareProxyDll,"DisconnectAllProxies");
	if (m_pSendDisconnectMsgToAllProxy == NULL) 
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: SendDisconnectMsgToAllProxy is NULL");
#endif
		return false;
	}

	m_pSetLoggingOptions = (LPFN_PROXYDLL_SET_DLL_LOGGING_OPTIONS)
		GetProcAddress(m_hModEchowareProxyDll,"SetLoggingOptions");
	if (m_pSetLoggingOptions == NULL) 
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: pSetLoggingOptions is NULL");
#endif
		return false;
	}

	m_pSetPortForOffLoadingData = (LPFN_PROXYDLL_SET_PORT_FOR_OFFLOADING_DATA)
			GetProcAddress(m_hModEchowareProxyDll,"SetPortForOffLoadingData");
	if (m_pSetPortForOffLoadingData == NULL) 
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: SetPortForOffLoadingData is NULL");
#endif
		return false;
	}

	m_pEstablishNewDataChannel = (LPFN_PROXYDLL_ESTABLISH_NEW_DATA_CHANNEL)
			GetProcAddress(m_hModEchowareProxyDll,"EstablishNewDataChannel");
	if (m_pEstablishNewDataChannel == NULL)
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: EstablishNewDataChannel is NULL");
#endif
		return false;
	}

	m_pGetDllVersion = (LPFN_PROXYDLL_GET_DLLVERSION)
		GetProcAddress(m_hModEchowareProxyDll,"GetDllVersion");
	if (m_pGetDllVersion == NULL)
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: GetDllVersion is NULL");
#endif
		return false;
	}

	m_pSetLocalProxyInfo = (LPFN_PROXYDLL_SET_LOCAL_PROXY_INFO)
		GetProcAddress(m_hModEchowareProxyDll,"SetLocalProxyInfo");
	if (m_pSetLocalProxyInfo == NULL)
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: SetLocalProxyInfo is NULL");
#endif
		return false;
	}

	m_pSetEncryptionLevel = (LPFN_PROXYDLL_SET_ENCRYPTION_LEVEL)
		GetProcAddress(m_hModEchowareProxyDll,"SetEncryptionLevel");
	if (m_pSetEncryptionLevel == NULL)
	{
#ifdef _DEBUG
		printf("Warning - Function Pointer: SetEncryptionLevel is NULL");
#endif
	}

	return true;
}


const char* CEchowareDll::GetDllVersion()const
{
	return m_pGetDllVersion();
}


BOOL CEchowareDll::InitializeProxyDll()const
{
	return m_pInitializeProxyDll();
}


void CEchowareDll::SetLoggingOptions(BOOL bEnablelogging, char *szLogPath)const
{
	m_pSetLoggingOptions( bEnablelogging, szLogPath);
}


IDllProxyInfo* CEchowareDll::CreateProxyInfoClassObject()const
{
	IDllProxyInfo *pProxyInfo = (IDllProxyInfo*)m_pProxyInfoObject();

	if (m_pSetEncryptionLevel != NULL) 
		m_pSetEncryptionLevel(1, (void *)pProxyInfo);

	return pProxyInfo;
}


BOOL CEchowareDll::SetPortForOffLoadingData(int DataOffLoadingPort)const
{
	return m_pSetPortForOffLoadingData(DataOffLoadingPort);
}


int CEchowareDll::ConnectProxy(IDllProxyInfo* pProxyInfo)
{
	return m_pConnectProxy(pProxyInfo);
}


BOOL CEchowareDll::DisconnectProxy(IDllProxyInfo* pProxyInfo)
{
	return m_pDisconnectProxy(pProxyInfo);
}


void CEchowareDll::DeleteProxyInfoClassObject(IDllProxyInfo* pProxyInfo)
{
	m_pDeleteDllProxyInfoObject(pProxyInfo);
}


int CEchowareDll::EstablishNewDataChannel( IDllProxyInfo* pProxyInfo , char* IDOfPartner)
{
	return m_pEstablishNewDataChannel(pProxyInfo, IDOfPartner);
}


void CEchowareDll::AutoReconnect()
{
	m_pAutoConnect;
}


void CEchowareDll::SetLocalProxyInfo(
	const char* szIP, 
	const char* szPort, 
	const char* szUsername, 
	const char* szPassword)
{
	m_pSetLocalProxyInfo( szIP, szPort, szUsername, szPassword);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEchoWareChannel::CEchoWareChannel()
:	m_pDll(0),
	m_pProxy(0),
	m_pStartupDlg(0)
{
}


CEchoWareChannel::~CEchoWareChannel()
{
//	DeleteAllProxy();

	if (m_pProxy)
	{
		m_pDll->DisconnectProxy(m_pProxy);
		m_pDll->DeleteProxyInfoClassObject(m_pProxy);
	}

	if (m_pDll)
	{
		delete m_pDll;
	}

	if (m_pStartupDlg)
	{
		delete m_pStartupDlg;
	}
}


bool CEchoWareChannel::LoadDll()
{
	if (m_pDll)
	{
		delete m_pDll;
	}

	m_pDll = new CEchowareDll();

	return m_pDll->Init();
}


static void IpAddressToString( DWORD dwIpAddress, std::string& rStrIPAddress)
{
	rStrIPAddress = "";
	in_addr adr;
	adr.S_un.S_addr = dwIpAddress;
	for( int i = 0; i < 4; i++)
	{
		char str[_MAX_PATH];
		if (i==0)
			sprintf_s( str, _MAX_PATH, "%d.", (int)adr.S_un.S_un_b.s_b1);
		else if (i==1)
			sprintf_s( str, _MAX_PATH, "%d.", (int)adr.S_un.S_un_b.s_b2);
		else if (i==2)
			sprintf_s( str, _MAX_PATH, "%d.", (int)adr.S_un.S_un_b.s_b3);
		else //if (i==3)
			sprintf_s( str, _MAX_PATH, "%d", (int)adr.S_un.S_un_b.s_b4);

		rStrIPAddress += str;
	}
}

/*
//#IR:100315: - : previous version of the CEchoWareChannel::DoConfig()
int CEchoWareChannel::DoConfig(const CUserData* pData, UINT uiLoopbackPortNumber,
								ST_LOCALP_ROXY_DATA* pProxyData) // 0
{
	char szResolvedIP[_MAX_PATH] = {0};
	char szText[_MAX_PATH] = {0};

	if (m_pStartupDlg != NULL) {
		delete m_pStartupDlg;
	}

	m_pStartupDlg = new StartupDlg;
	if (m_pStartupDlg == NULL) {
		NS_Common::Message_Box("Sorry... Cannot create startup dialog");
		return ERROR_CONNECTING_TO_PROXY;
	}

	bool bOK = true;
	//if (!CUserData::IsIPAddressChar(pData->m_szAddress.c_str()))
	{
		bOK = false;
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD( 2, 2 );
		::WSAStartup( wVersionRequested, &wsaData );

		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41075, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41075, szAddr - sz_IDS_ERROR41075);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41075);

		sprintf_s( szText, _MAX_PATH, szTmpMsg, pData->m_szAddress.c_str());
		
		if (!m_pStartupDlg->show(szText)) {	//#here : show the Startup dialog from the EchowareDll.cpp
			NS_Common::Message_Box("Sorry... Cannot open startup dialog");
			m_pStartupDlg->destroy();
			return ERROR_CONNECTING_TO_PROXY;
		}
	
//		m_pStartupDlg->waitAndProcessDlgMsgs(0);

		HOSTENT* pHost = ::gethostbyname( pData->m_szAddress.c_str() );
		if ( pHost )
		{
			DWORD dwIP = 0;
			memcpy( &dwIP, pHost->h_addr_list[0], pHost->h_length );
			std::string szA;
			IpAddressToString( dwIP, szA);

			//char szMsg[512];
			//sprintf( szMsg, "The address <%s> has been successfully resolved as <%s>", 
			//	pData->m_szAddress.c_str(), szA.c_str());
			//NS_Common::Message_Box( szMsg, MB_OK, 0);
			//pData->m_szAddress = std::string(szA);
			strcpy_s( szResolvedIP, _MAX_PATH, szA.c_str());
			bOK = true;
		}
		::WSACleanup();
	}

	char szMsg[_MSG_LEN];
	if (!bOK)
	{
		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41068, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41068, szAddr - sz_IDS_ERROR41068);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41068);
		sprintf_s( szMsg, _MSG_LEN, szTmpMsg, pData->m_szAddress.c_str());
		NS_Common::Message_Box( szMsg, MB_OK, 0);
		m_pStartupDlg->destroy();
		return NO_PROXY_SERVER_FOUND_TO_CONNECT;
	}

	m_pStartupDlg->waitAndProcessDlgMsgs(1000);
	{
		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41073, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41073, szAddr - sz_IDS_ERROR41073);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41073);
		sprintf_s(szText, _MAX_PATH, szTmpMsg, szResolvedIP);
	}
	if ((!m_pStartupDlg->next(szText)) || (!m_pStartupDlg->isContinue())) {
		m_pStartupDlg->destroy();
		return ERROR_CONNECTING_TO_PROXY;
	}

	m_pStartupDlg->waitAndProcessDlgMsgs(0);

	if (pProxyData && 
		pProxyData->szIP.length() != 0 &&
		pProxyData->szUsername.length() != 0 &&
		pProxyData->szPassword.length() != 0 &&
		pProxyData->szPort.length() != 0)
	{
		m_pDll->SetLocalProxyInfo(
			pProxyData->szIP.c_str(), 
			pProxyData->szPort.c_str(), 
			pProxyData->szUsername.c_str(), 
			pProxyData->szPassword.c_str());
	}

	m_pStartupDlg->waitAndProcessDlgMsgs(0);

	if ( m_pProxy == 0 )
	{
		if (m_pDll->InitializeProxyDll())
		{
			m_pStartupDlg->waitAndProcessDlgMsgs(0);
			m_pProxy = m_pDll->CreateProxyInfoClassObject();
		}
	}

	if ( m_pProxy == 0 )
	{
		m_pStartupDlg->destroy();
		return ERROR_CONNECTING_TO_PROXY;
	}

	bOK = false;

	m_pStartupDlg->waitAndProcessDlgMsgs(1000);

	if ((!m_pStartupDlg->next(sz_IDS_ERROR41074)) || (!m_pStartupDlg->isContinue())) {
		m_pStartupDlg->destroy();
		m_pDll->DeleteProxyInfoClassObject(m_pProxy);
		return ERROR_CONNECTING_TO_PROXY;
	}

	try
	{
		//--- Set IP
		m_pProxy->SetIP(szResolvedIP);

		//--- Set Port
		m_pProxy->SetPort(pData->m_szPort.c_str());

		//--- Set Password
		m_pProxy->SetPassword(pData->m_szPassword.c_str());

		int iErrCode = 0;

		//--- Set My ID
		if (m_pProxy->SetMyID(pData->m_szUsername.c_str()))
		{
			//--- Set Port For OffLoading Data
			m_pDll->SetPortForOffLoadingData(uiLoopbackPortNumber);

			m_pStartupDlg->waitAndProcessDlgMsgs(0);

			//--- Connect Proxy
			iErrCode = m_pDll->ConnectProxy(m_pProxy);

			m_pStartupDlg->waitAndProcessDlgMsgs(0);

			if (iErrCode==0)
			{
				//--- If that connection works, you can set AutoConnect().
				m_pDll->AutoReconnect();
				bOK = true;
			}
		}

		if (!bOK)
		{
			int res = ERROR_CONNECTING_TO_PROXY;
			// Определение ошибки 
			switch(iErrCode)
			{
				case NO_PROXY_SERVER_FOUND_TO_CONNECT:
				{
					res = NO_PROXY_SERVER_FOUND_TO_CONNECT;
					char szTmpMsg[_MSG_LEN];
					memset(szTmpMsg, 0, _MSG_LEN);
					char *szAddr = strstr(sz_IDS_ERROR41070, "#ADDRESS");
					if (szAddr)
					{
						memcpy(szTmpMsg, sz_IDS_ERROR41070, szAddr - sz_IDS_ERROR41070);
						strcat_s(szTmpMsg, _MSG_LEN, "%s");
						strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
					}
					else
						strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41070);

					char szTmpMsg2[_MSG_LEN];
					memset(szTmpMsg2, 0, _MSG_LEN);
					szAddr = strstr(szTmpMsg, "#PORT");
					if (szAddr)
					{
						memcpy(szTmpMsg2, szTmpMsg, szAddr - szTmpMsg);
						strcat_s(szTmpMsg2, _MSG_LEN, "%s");
						strcat_s(szTmpMsg2, _MSG_LEN, szAddr + strlen("#PORT"));
					}
					else
						strcpy_s(szTmpMsg2, _MSG_LEN, szTmpMsg);

					sprintf_s( szMsg, _MSG_LEN, szTmpMsg2, szResolvedIP, pData->m_szPort.c_str() );
					break;
				}

				default:
				{
					char szTmpMsg[_MSG_LEN];
					memset(szTmpMsg, 0, _MSG_LEN);
					char *szAddr = strstr(sz_IDS_ERROR41070, "#ADDRESS");
					if (szAddr)
					{
						memcpy(szTmpMsg, sz_IDS_ERROR41070, szAddr - sz_IDS_ERROR41070);
						strcat_s(szTmpMsg, _MSG_LEN, "%s");
						strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
					}
					else
						strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41070);
					res = AUTHENTICATION_FAILED;
					sprintf_s( szMsg, _MSG_LEN, szTmpMsg, szResolvedIP );
					break;
				}
			}
	//ERROR_CONNECTING_TO_PROXY "Error in connecting to proxy"
	//CONNECTION_SUCCESSFUL
	//	NO_PROXY_SERVER_FOUND_TO_CONNECT "Proxy was not found"
	//	AUTHENTICATION_FAILED ""
	//PROXY_ALREADY_CONNECTED "Proxy already connected"
	//CONNECTION_TIMED_OUT
	//ID_FOUND_EMPTY
			
			NS_Common::Message_Box( szMsg, MB_OK, 0);
			DeleteProxy();
			m_pStartupDlg->destroy();
			return res;
		}
	}
	catch(...)
	{
		m_pStartupDlg->destroy();
		return ERROR_CONNECTING_TO_PROXY;
	}

	m_pStartupDlg->waitAndProcessDlgMsgs(0);

	std::string sUserName = m_pProxy->GetMyID();
	int nIndex = sUserName.find_first_of(":");
	if (nIndex >= 0)
		sUserName.erase(nIndex);

	if (pData->m_vncPassword.length() == 0)
	{
		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41071, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41071, szAddr - sz_IDS_ERROR41071);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41071);

		char szTmpMsg2[_MSG_LEN];
		memset(szTmpMsg2, 0, _MSG_LEN);
		szAddr = strstr(szTmpMsg, "#USER");
		if (szAddr)
		{
			memcpy(szTmpMsg2, szTmpMsg, szAddr - szTmpMsg);
			strcat_s(szTmpMsg2, _MSG_LEN, "%s");
			strcat_s(szTmpMsg2, _MSG_LEN, szAddr + strlen("#USER"));
		}
		else
			strcpy_s(szTmpMsg2, _MSG_LEN, szTmpMsg);
		sprintf_s(szText, _MAX_PATH, szTmpMsg2, pData->m_szAddress.c_str(), sUserName.c_str());
		memset(sz_IDS_STRING41090, 0, 128);
	}
	else
	{
		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41072, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41072, szAddr - sz_IDS_ERROR41072);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41072);

		char szTmpMsg2[_MSG_LEN];
		memset(szTmpMsg2, 0, _MSG_LEN);
		szAddr = strstr(szTmpMsg, "#USER");
		if (szAddr)
		{
			memcpy(szTmpMsg2, szTmpMsg, szAddr - szTmpMsg);
			strcat_s(szTmpMsg2, _MSG_LEN, "%s");
			strcat_s(szTmpMsg2, _MSG_LEN, szAddr + strlen("#USER"));
		}
		else
			strcpy_s(szTmpMsg2, _MSG_LEN, szTmpMsg);

		char szTmpMsg3[_MSG_LEN];
		memset(szTmpMsg3, 0, _MSG_LEN);
		szAddr = strstr(szTmpMsg2, "#PASS");
		if (szAddr)
		{
			memcpy(szTmpMsg3, szTmpMsg2, szAddr - szTmpMsg2);
			strcat_s(szTmpMsg3, _MSG_LEN, "%s");
			strcat_s(szTmpMsg3, _MSG_LEN, szAddr + strlen("#PASS"));
		}
		else
			strcpy_s(szTmpMsg3, _MSG_LEN, szTmpMsg2);

		sprintf_s(szText, _MAX_PATH, szTmpMsg3, pData->m_szAddress.c_str(), sUserName.c_str(), pData->m_vncPassword.c_str());


		memset(szTmpMsg, 0, _MSG_LEN);
		szAddr = strstr(sz_IDS_STRING41090, "#PASS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_STRING41090, szAddr - sz_IDS_STRING41090);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#PASS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_STRING41090);

		sprintf_s(sz_IDS_STRING41090, 128, szTmpMsg, pData->m_vncPassword.c_str());
	}
	if ((!m_pStartupDlg->next(szText)) || (!m_pStartupDlg->isContinue())) {
		m_pStartupDlg->destroy();
		return ERROR_CONNECTING_TO_PROXY;
	}
	m_pStartupDlg->waitAndProcessDlgMsgs(0);

	return CONNECTION_SUCCESSFUL;
}
*/

//#IR:100315: + : new version of the CEchoWareChannel::DoConfig()
extern bool	g_bSilentMode;	//#IR:100315 +
int CEchoWareChannel::DoConfig(const CUserData* pData, UINT uiLoopbackPortNumber,
								ST_LOCALP_ROXY_DATA* pProxyData) // 0
{
	char szResolvedIP[_MAX_PATH] = {0};
	char szText[_MAX_PATH] = {0};

	if (m_pStartupDlg != NULL) {
		delete m_pStartupDlg;
		m_pStartupDlg = NULL; //#IR: +
	}

//	if (!g_bSilentMode) {
		m_pStartupDlg = new StartupDlg(g_pUserData);
		if (m_pStartupDlg == NULL) {
			NS_Common::Message_Box("Sorry... Cannot create startup dialog");
			return ERROR_CONNECTING_TO_PROXY;
		}
//	}

	bool bOK = true;
	//if (!CUserData::IsIPAddressChar(pData->m_szAddress.c_str()))
	{
		bOK = false;
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD( 2, 2 );
		::WSAStartup( wVersionRequested, &wsaData );

		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41075, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41075, szAddr - sz_IDS_ERROR41075);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41075);

		sprintf_s( szText, _MAX_PATH, szTmpMsg, pData->m_szAddress.c_str());
		
		if (!g_bSilentMode) {
			if (!m_pStartupDlg->show(szText)) {	//#here : show the Startup dialog from the EchowareDll.cpp
				NS_Common::Message_Box("Sorry... Cannot open startup dialog");
				m_pStartupDlg->destroy();
				return ERROR_CONNECTING_TO_PROXY;
			}
		}
	
//		m_pStartupDlg->waitAndProcessDlgMsgs(0);

		HOSTENT* pHost = ::gethostbyname( pData->m_szAddress.c_str() );
		if ( pHost )
		{
			DWORD dwIP = 0;
			memcpy( &dwIP, pHost->h_addr_list[0], pHost->h_length );
			std::string szA;
			IpAddressToString( dwIP, szA);

			//char szMsg[512];
			//sprintf( szMsg, "The address <%s> has been successfully resolved as <%s>", 
			//	pData->m_szAddress.c_str(), szA.c_str());
			//NS_Common::Message_Box( szMsg, MB_OK, 0);
			//pData->m_szAddress = std::string(szA);
			strcpy_s( szResolvedIP, _MAX_PATH, szA.c_str());
			bOK = true;
		}
		::WSACleanup();
	}

	char szMsg[_MSG_LEN];
	if (!bOK)
	{
		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41068, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41068, szAddr - sz_IDS_ERROR41068);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41068);
		sprintf_s( szMsg, _MSG_LEN, szTmpMsg, pData->m_szAddress.c_str());
		if (!g_bSilentMode)	
		{
			NS_Common::Message_Box( szMsg, MB_OK, 0);
		}
		if (NULL != m_pStartupDlg) m_pStartupDlg->destroy();
		return NO_PROXY_SERVER_FOUND_TO_CONNECT;
	}

//	if (NULL != m_pStartupDlg) 
	{
		m_pStartupDlg->waitAndProcessDlgMsgs(1000);
	}
	{
		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41073, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41073, szAddr - sz_IDS_ERROR41073);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41073);
		sprintf_s(szText, _MAX_PATH, szTmpMsg, szResolvedIP);
	}
	if (!g_bSilentMode){
		if ((!m_pStartupDlg->next(szText)) || (!m_pStartupDlg->isContinue())) {
			m_pStartupDlg->destroy();
			return ERROR_CONNECTING_TO_PROXY;
		}
	}

//	if (NULL != m_pStartupDlg) 
	{
		m_pStartupDlg->waitAndProcessDlgMsgs(0);
	}

	if (pProxyData && 
		pProxyData->szIP.length() != 0 &&
		pProxyData->szUsername.length() != 0 &&
		pProxyData->szPassword.length() != 0 &&
		pProxyData->szPort.length() != 0)
	{
		m_pDll->SetLocalProxyInfo(
			pProxyData->szIP.c_str(), 
			pProxyData->szPort.c_str(), 
			pProxyData->szUsername.c_str(), 
			pProxyData->szPassword.c_str());
	}

//	if (NULL != m_pStartupDlg) 
	{
		m_pStartupDlg->waitAndProcessDlgMsgs(0);
	}

	if ( m_pProxy == 0 )
	{
		if (m_pDll->InitializeProxyDll())
		{
			//if (NULL != m_pStartupDlg) 
			{
				m_pStartupDlg->waitAndProcessDlgMsgs(0);
			}
			m_pProxy = m_pDll->CreateProxyInfoClassObject();
		}
	}

	if ( m_pProxy == 0 )
	{
		if (NULL != m_pStartupDlg) m_pStartupDlg->destroy();
		return ERROR_CONNECTING_TO_PROXY;
	}

	bOK = false;

//	if (NULL != m_pStartupDlg) 
	{
		m_pStartupDlg->waitAndProcessDlgMsgs(1000);
	}

	if (!g_bSilentMode) {
		if ((!m_pStartupDlg->next(sz_IDS_ERROR41074)) || (!m_pStartupDlg->isContinue())) {
			m_pStartupDlg->destroy();
			m_pDll->DeleteProxyInfoClassObject(m_pProxy);
			return ERROR_CONNECTING_TO_PROXY;
		}
	}

	try
	{
		//--- Set IP
		m_pProxy->SetIP(szResolvedIP);

		//--- Set Port
		m_pProxy->SetPort(pData->m_szPort.c_str());

		//--- Set Password
		m_pProxy->SetPassword(pData->m_szPassword.c_str());

		int iErrCode = 0;

		//--- Set My ID
		if (m_pProxy->SetMyID(pData->m_szUsername.c_str()))
		{
			//--- Set Port For OffLoading Data
			m_pDll->SetPortForOffLoadingData(uiLoopbackPortNumber);
			
			//if (NULL != m_pStartupDlg) 
			{
				m_pStartupDlg->waitAndProcessDlgMsgs(0);
			}

			//--- Connect Proxy
//#IR:100315: i : I did get the "First-chance exception at 0x7c96df51 in miniWinVNC.exe: 0xC0000005: Access violation reading location 0xfffffff9" on the line below (Debug mode)
			iErrCode = m_pDll->ConnectProxy(m_pProxy);

			//if (NULL != m_pStartupDlg) 
			{
				m_pStartupDlg->waitAndProcessDlgMsgs(0);
			}

			if (iErrCode==0)
			{
				//--- If that connection works, you can set AutoConnect().
				m_pDll->AutoReconnect();
				bOK = true;
			}
		}

		if (!bOK)
		{
			int res = ERROR_CONNECTING_TO_PROXY;
			// Определение ошибки 
			switch(iErrCode)
			{
				case NO_PROXY_SERVER_FOUND_TO_CONNECT:
				{
					res = NO_PROXY_SERVER_FOUND_TO_CONNECT;
					char szTmpMsg[_MSG_LEN];
					memset(szTmpMsg, 0, _MSG_LEN);
					char *szAddr = strstr(sz_IDS_ERROR41070, "#ADDRESS");
					if (szAddr)
					{
						memcpy(szTmpMsg, sz_IDS_ERROR41070, szAddr - sz_IDS_ERROR41070);
						strcat_s(szTmpMsg, _MSG_LEN, "%s");
						strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
					}
					else
						strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41070);

					char szTmpMsg2[_MSG_LEN];
					memset(szTmpMsg2, 0, _MSG_LEN);
					szAddr = strstr(szTmpMsg, "#PORT");
					if (szAddr)
					{
						memcpy(szTmpMsg2, szTmpMsg, szAddr - szTmpMsg);
						strcat_s(szTmpMsg2, _MSG_LEN, "%s");
						strcat_s(szTmpMsg2, _MSG_LEN, szAddr + strlen("#PORT"));
					}
					else
						strcpy_s(szTmpMsg2, _MSG_LEN, szTmpMsg);

					sprintf_s( szMsg, _MSG_LEN, szTmpMsg2, szResolvedIP, pData->m_szPort.c_str() );
					break;
				}

				default:
				{
					char szTmpMsg[_MSG_LEN];
					memset(szTmpMsg, 0, _MSG_LEN);
					char *szAddr = strstr(sz_IDS_ERROR41070, "#ADDRESS");
					if (szAddr)
					{
						memcpy(szTmpMsg, sz_IDS_ERROR41070, szAddr - sz_IDS_ERROR41070);
						strcat_s(szTmpMsg, _MSG_LEN, "%s");
						strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
					}
					else
						strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41070);
					res = AUTHENTICATION_FAILED;
					sprintf_s( szMsg, _MSG_LEN, szTmpMsg, szResolvedIP );
					break;
				}
			}
	//ERROR_CONNECTING_TO_PROXY "Error in connecting to proxy"
	//CONNECTION_SUCCESSFUL
	//	NO_PROXY_SERVER_FOUND_TO_CONNECT "Proxy was not found"
	//	AUTHENTICATION_FAILED ""
	//PROXY_ALREADY_CONNECTED "Proxy already connected"
	//CONNECTION_TIMED_OUT
	//ID_FOUND_EMPTY
			
			extern bool g_bSilentMode;
			if (!g_bSilentMode)
			{
				NS_Common::Message_Box( szMsg, MB_OK, 0);
			}
			DeleteProxy();
			if (NULL != m_pStartupDlg) m_pStartupDlg->destroy();
			return res;
		}
	}
	catch(...)
	{
		if (NULL != m_pStartupDlg) m_pStartupDlg->destroy();
		return ERROR_CONNECTING_TO_PROXY;
	}

	m_pStartupDlg->waitAndProcessDlgMsgs(0);

	std::string sUserName = m_pProxy->GetMyID();
	int nIndex = sUserName.find_first_of(":");
	if (nIndex >= 0)
		sUserName.erase(nIndex);

	if (pData->m_vncPassword.length() == 0)
	{
		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41071, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41071, szAddr - sz_IDS_ERROR41071);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41071);

		char szTmpMsg2[_MSG_LEN];
		memset(szTmpMsg2, 0, _MSG_LEN);
		szAddr = strstr(szTmpMsg, "#USER");
		if (szAddr)
		{
			memcpy(szTmpMsg2, szTmpMsg, szAddr - szTmpMsg);
			strcat_s(szTmpMsg2, _MSG_LEN, "%s");
			strcat_s(szTmpMsg2, _MSG_LEN, szAddr + strlen("#USER"));
		}
		else
			strcpy_s(szTmpMsg2, _MSG_LEN, szTmpMsg);
		sprintf_s(szText, "\r\n");
		sprintf_s(szText + 2, _MAX_PATH - 2, szTmpMsg2, pData->m_szAddress.c_str(), sUserName.c_str());
		memset(sz_IDS_STRING41090, 0, 128);
	}
	else
	{
		char szTmpMsg[_MSG_LEN];
		memset(szTmpMsg, 0, _MSG_LEN);
		char *szAddr = strstr(sz_IDS_ERROR41072, "#ADDRESS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_ERROR41072, szAddr - sz_IDS_ERROR41072);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#ADDRESS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_ERROR41072);

		char szTmpMsg2[_MSG_LEN];
		memset(szTmpMsg2, 0, _MSG_LEN);
		szAddr = strstr(szTmpMsg, "#USER");
		if (szAddr)
		{
			memcpy(szTmpMsg2, szTmpMsg, szAddr - szTmpMsg);
			strcat_s(szTmpMsg2, _MSG_LEN, "%s");
			strcat_s(szTmpMsg2, _MSG_LEN, szAddr + strlen("#USER"));
		}
		else
			strcpy_s(szTmpMsg2, _MSG_LEN, szTmpMsg);

		char szTmpMsg3[_MSG_LEN];
		memset(szTmpMsg3, 0, _MSG_LEN);
		szAddr = strstr(szTmpMsg2, "#PASS");
		if (szAddr)
		{
			memcpy(szTmpMsg3, szTmpMsg2, szAddr - szTmpMsg2);
			strcat_s(szTmpMsg3, _MSG_LEN, "%s");
			strcat_s(szTmpMsg3, _MSG_LEN, szAddr + strlen("#PASS"));
		}
		else
			strcpy_s(szTmpMsg3, _MSG_LEN, szTmpMsg2);

		sprintf_s(szText, _MAX_PATH, szTmpMsg3, pData->m_szAddress.c_str(), sUserName.c_str(), pData->m_vncPassword.c_str());


		memset(szTmpMsg, 0, _MSG_LEN);
		szAddr = strstr(sz_IDS_STRING41090, "#PASS");
		if (szAddr)
		{
			memcpy(szTmpMsg, sz_IDS_STRING41090, szAddr - sz_IDS_STRING41090);
			strcat_s(szTmpMsg, _MSG_LEN, "%s");
			strcat_s(szTmpMsg, _MSG_LEN, szAddr + strlen("#PASS"));
		}
		else
			strcpy_s(szTmpMsg, _MSG_LEN, sz_IDS_STRING41090);

		sprintf_s(sz_IDS_STRING41090, 128, szTmpMsg, pData->m_vncPassword.c_str());
	}
	if (!g_bSilentMode) 
	{
		if ((!m_pStartupDlg->next(szText)) || (!m_pStartupDlg->isContinue())) {
			m_pStartupDlg->destroy();
			return ERROR_CONNECTING_TO_PROXY;
		}
	}
	m_pStartupDlg->waitAndProcessDlgMsgs(0);

	return CONNECTION_SUCCESSFUL;
}


void CEchoWareChannel::DeleteProxy()
{
	if (m_pDll && m_pProxy)
	{
		m_pDll->DeleteProxyInfoClassObject(m_pProxy);
		m_pProxy = NULL;
	}
}

void CEchoWareChannel::closeStartupDlg()
{
	if (m_pStartupDlg != NULL) m_pStartupDlg->destroy();
}


//void CEchoWareChannel::DeleteProxy(IDllProxyInfo* pProxy)
//{
//	m_pDll->DisconnectProxy(pProxy);
//	m_pDll->DeleteProxyInfoClassObject(pProxy);
//}


//void CEchoWareChannel::DeleteAllProxy()
//{
//	std::vector<ST_PROXY_DATA>::iterator iter = m_vectData.begin();
//	for( iter = m_vectData.begin(); iter != m_vectData.end(); ++iter)
//	{
//		DeleteProxy(iter->pProxy);
//	}
//}


//bool CEchoWareChannel::RemoveObject( UINT uiPort )
//{
//	std::vector<ST_PROXY_DATA>::iterator iter = m_vectData.begin();
//	for( iter = m_vectData.begin(); iter != m_vectData.end(); ++iter)
//	{
//		if (iter->uiLoopbackPortNumber == uiPort )
//		{
//			DeleteProxy(iter->pProxy);
//			m_vectData.erase(iter);
//			return true;
//		}
//	}
//	return false;
//}


//void CEchoWareChannel::AddObject( IDllProxyInfo* proxy, UINT uiPort )
//{
//	ST_PROXY_DATA data( proxy, uiPort);
//	m_vectData.push_back(data);
//}
