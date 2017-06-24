// SerializeData.h: interface for the CSerializeData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALIZEDATA_H__4810AB26_85E4_459C_9204_7C0E4A8DFB67__INCLUDED_)
#define AFX_SERIALIZEDATA_H__4810AB26_85E4_459C_9204_7C0E4A8DFB67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable : 4786)

#include "windows.h"
#include "..\AskParameter\ProxyData.h"
#include <string>
#include "TextData.h"


#define _MAX_ADDRESS	80
#define _MAX_USERNAME	80
#define _MAX_PASSWORD	80

#define S_DEBUG	"-debug"
#define S_AUTO_EXIT_AFTER_LAST_VIEWER	"-exitafterlastviewer"
#define S_AUTO_EXIT_IF_NO_VIEWER	"-exitifnoviewer"
#define S_CREATE_RANDOM_PASS	"-generatepassword"
#define S_ENABLE_DUAL_MON	"-enabledualmonitors"
#define S_USE_MINI_GUI		"-useminigui"
#define S_DISPLAY_STARTUP_HELP  "-startuphelp"

extern TCHAR *szComputerName;

class CSerializeData  
{
public:

	struct ST_DATA
	{
		std::string m_szAddress;
		std::string m_szUsername;
		std::string m_szPassword;
		std::string m_szDebug;
		std::string m_szImagePath;
		std::string m_szLockAddress;
		std::string m_szLockUsername;
		std::string m_szLockPassword;
		std::string m_szLockAdvProps;
		BYTE* m_pBinImage;
		DWORD m_dwSizeBinImage;
		std::string m_szIcon;
		std::string m_szAutoExitAfterLastViewer;
		std::string m_szAutoExitIfNoViewer;
		std::string m_szCreatePasswords;
		std::string m_szEnableDualMonitors;
		std::string m_szUseMiniGui;
		std::string m_szDisplayStartupHelp;
		CTextData *m_objData;

		CProxyData m_ProxyData;

		ST_DATA()
		:	m_szAddress("demo.echovnc.com"),
			m_szUsername(szComputerName),
			m_szPassword("demo2010"),
			m_szDebug(""),
			m_szImagePath(""),
			m_szLockAddress(""),
			m_szLockUsername(""),
			m_szLockPassword(""),
			m_szLockAdvProps(""),
			m_pBinImage(0),
			m_dwSizeBinImage(0),
			m_szIcon(""),
			m_szAutoExitAfterLastViewer(""),
			m_szAutoExitIfNoViewer(""),
			m_szCreatePasswords(""),
			m_szEnableDualMonitors(""),
			m_szUseMiniGui(""),
			m_szDisplayStartupHelp("")
		{
			m_objData = new CTextData();
		}

		~ST_DATA()
		{
			if (m_pBinImage)
			{
				delete [] m_pBinImage;
			}
			delete m_objData;
		}
	};

	ST_DATA m_data;

	// 773DD5BE-6C0C-4a91-8997-AE25DA7A2B6A
	static const char m_szLabel[36 + 1];

	CSerializeData();

	int GetLength()const;

	bool ReadFromFile(const char* szFullPath, bool bDefaultSettings);

	bool WriteIntoFile(const char* szFullPath);

protected:

	// Header at the end of file:
	// m_szLabel[36 + 1] + (data_length = 4 bytes)

	bool SimpleReadFromFile(const char* szFullPath);

	bool ReadFromFile(HANDLE hFile);
	bool CheckHeader(HANDLE hFile, UINT& rUiDataSize)const;
	static UINT GetHeaderLength();
	bool ReadFromMemory( BYTE* pBuf, UINT uiSize );

	bool IsData(const char* szFullPath, UINT* pDataSize)const;
	bool RemoveData(const char* szFullPath, UINT uiOldDataSize)const;
	bool WriteDataToBuf( BYTE* pBuf, UINT uiSize );
	bool GetSizeImageFromDrive( DWORD& rDwImageSize)const;

	BOOL ChangeAppIcon(const char* szFullPath, BYTE *pBuffer, DWORD dwBufferSize);
	BOOL ReadIconToBuffer(const char* szFullPath, BYTE *&pBuffer, DWORD &dwBuferSize);
};

#endif // !defined(AFX_SERIALIZEDATA_H__4810AB26_85E4_459C_9204_7C0E4A8DFB67__INCLUDED_)
