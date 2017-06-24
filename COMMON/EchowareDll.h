// EchowareDll.h: interface for the CEchowareDll class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ECHOWAREDLL_H__7CE84082_3C02_47B8_B9FE_781B90B55531__INCLUDED_)
#define AFX_ECHOWAREDLL_H__7CE84082_3C02_47B8_B9FE_781B90B55531__INCLUDED_

#include <vector>
#include <string>

#define ID_STRING_SIZE		255

#include "StartupDlg.h"
#include "InterfaceDllProxyInfo.h"

typedef void  (*LPFN_PROXYDLL_AUTOCONNECT)();

typedef void* (*LPFN_PROXYDLL_GET_PROXY_INFO_OBJECT)();

typedef void  (*LPFN_PROXYDLL_DELETE_PROXY_INFO_OBJECT)(void*);

typedef BOOL  (*LPFN_PROXYDLL_INITIALIZE_PROXYDLL)();

typedef BOOL  (*LPFN_PROXYDLL_CONNECTPROXY)(void*);

typedef BOOL  (*LPFN_PROXYDLL_DISCONNECT_PROXY)(void*);

typedef void  (*LPFN_PROXYDLL_SEND_DISCONNECT_MSG_TO_ALL_PROXY)();

typedef void  (*LPFN_PROXYDLL_SET_DLL_LOGGING_OPTIONS)(BOOL, char*);

typedef BOOL  (*LPFN_PROXYDLL_FIND_PARTNER_VIA_PROXY)(char*);

typedef BOOL  (*LPFN_PROXYDLL_SET_PORT_FOR_OFFLOADING_DATA)(int);

typedef int   (*LPFN_PROXYDLL_ESTABLISH_NEW_DATA_CHANNEL)(void*, char* );

typedef char* (*LPFN_PROXYDLL_GET_DLLVERSION)(void);

typedef void (*LPFN_PROXYDLL_SET_LOCAL_PROXY_INFO)( const char* ip, const char* port, const char* username, const char* password);

typedef void (*LPFN_PROXYDLL_SET_ENCRYPTION_LEVEL)(int, void*);

//////////////////////////////////////////////////////////
//class CEchowareDll wraps echoWare.dll using
class CEchowareDll
{
public:
	CEchowareDll();

	virtual ~CEchowareDll();

	bool Init();

	const char* GetDllVersion()const;

	BOOL InitializeProxyDll()const;

	void SetLoggingOptions(BOOL bEnablelogging, char *szLogPath)const;

	IDllProxyInfo* CreateProxyInfoClassObject()const;

	void DeleteProxyInfoClassObject(IDllProxyInfo* pProxyInfo);

	BOOL SetPortForOffLoadingData(int DataOffLoadingPort)const;

	int ConnectProxy(IDllProxyInfo* pProxyInfo);

	BOOL DisconnectProxy(IDllProxyInfo* pProxyInfo);

	int EstablishNewDataChannel( IDllProxyInfo* pProxyInfo, char* IDOfPartner);

	void AutoReconnect();

	void SetLocalProxyInfo(const char* szIP, const char* szPort, const char* szUsername, const char* szPassword);

protected:

	HMODULE m_hModEchowareProxyDll; 

	LPFN_PROXYDLL_AUTOCONNECT m_pAutoConnect;
	LPFN_PROXYDLL_GET_PROXY_INFO_OBJECT m_pProxyInfoObject;
	LPFN_PROXYDLL_DELETE_PROXY_INFO_OBJECT m_pDeleteDllProxyInfoObject;
	LPFN_PROXYDLL_INITIALIZE_PROXYDLL m_pInitializeProxyDll;
	LPFN_PROXYDLL_CONNECTPROXY m_pConnectProxy;
	LPFN_PROXYDLL_DISCONNECT_PROXY m_pDisconnectProxy;
	LPFN_PROXYDLL_SEND_DISCONNECT_MSG_TO_ALL_PROXY m_pSendDisconnectMsgToAllProxy;
	LPFN_PROXYDLL_SET_DLL_LOGGING_OPTIONS m_pSetLoggingOptions;	
	LPFN_PROXYDLL_SET_PORT_FOR_OFFLOADING_DATA m_pSetPortForOffLoadingData;
	LPFN_PROXYDLL_ESTABLISH_NEW_DATA_CHANNEL m_pEstablishNewDataChannel;
	LPFN_PROXYDLL_GET_DLLVERSION m_pGetDllVersion;
	LPFN_PROXYDLL_SET_LOCAL_PROXY_INFO m_pSetLocalProxyInfo;
    LPFN_PROXYDLL_SET_ENCRYPTION_LEVEL m_pSetEncryptionLevel;
};


/////////////////////////////////////////////////////////////////////////////
class CUserData;
class CEchoWareChannel  
{
public:
	CEchoWareChannel();
	virtual ~CEchoWareChannel();

	bool LoadDll();

	struct ST_LOCALP_ROXY_DATA
	{
		std::string szIP;
		std::string szPort;
		std::string szUsername;
		std::string szPassword;
		ST_LOCALP_ROXY_DATA()
		:	szIP(""), szPort(""), szUsername(""), szPassword("")
		{
		}
	};

	int DoConfig(const CUserData* pData, UINT uiLoopbackPortNumber, 
		ST_LOCALP_ROXY_DATA* pProxyData = 0);

	CEchowareDll*	m_pDll;
	IDllProxyInfo*	m_pProxy;

	void closeStartupDlg();
	void DeleteProxy();

protected:

	StartupDlg *m_pStartupDlg;

	//struct ST_PROXY_DATA
	//{
	//	IDllProxyInfo*	pProxy;
	//	UINT			uiLoopbackPortNumber;
	//	ST_PROXY_DATA( IDllProxyInfo* pr, UINT uiPort )
	//	:	pProxy(pr), uiLoopbackPortNumber(uiPort)
	//	{
	//	}
	//};

	//std::vector<ST_PROXY_DATA> m_vectData;

	//void DeleteAllProxy();

public:
	//void DeleteProxy(IDllProxyInfo*	pProxy);
	//void AddObject( IDllProxyInfo* pr, UINT uiPort );
	//bool RemoveObject( UINT uiPort );
};

#endif // !defined(AFX_ECHOWAREDLL_H__7CE84082_3C02_47B8_B9FE_781B90B55531__INCLUDED_)
