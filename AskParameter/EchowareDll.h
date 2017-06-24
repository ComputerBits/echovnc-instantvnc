// EchowareDll.h: interface for the CEchowareDll class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ECHOWAREDLL_H__7CE84082_3C02_47B8_B9FE_781B90B55531__INCLUDED_)
#define AFX_ECHOWAREDLL_H__7CE84082_3C02_47B8_B9FE_781B90B55531__INCLUDED_

#include <string>

#define ID_STRING_SIZE		255

typedef char* (*LPFN_PROXYDLL_GET_DLLVERSION)(void);

//////////////////////////////////////////////////////////
//class CEchowareDll wraps echoWare.dll using
class CEchowareDll
{
public:
	CEchowareDll();

	virtual ~CEchowareDll();

	bool Init();

	const char* GetDllVersion()const;

protected:

	HMODULE m_hModEchowareProxyDll; 

	LPFN_PROXYDLL_GET_DLLVERSION m_pGetDllVersion;
};

#endif // !defined(AFX_ECHOWAREDLL_H__7CE84082_3C02_47B8_B9FE_781B90B55531__INCLUDED_)
