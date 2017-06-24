// ParseCmdLine.h: interface for the CParseCmdLine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSECMDLINE_H__5D83B50F_812C_4AC7_AEA7_0E311D64D455__INCLUDED_)
#define AFX_PARSECMDLINE_H__5D83B50F_812C_4AC7_AEA7_0E311D64D455__INCLUDED_



#include <string>
#include "UserData.h"

class CParseCmdLine  
{
public:
	CParseCmdLine( int argc, const char* argv[]);

	bool Parse();

	const char* GetAddress()const;
	const char* GetUsername()const;
	const char* GetPassword()const;
	const char* GetVncPwd()const;

	bool IsAddress()const;
	bool IsUsername()const;
	bool IsPassword()const;
	bool IsImagePath()const;
	bool IsMiniGui()const;

	bool IsSilent()const { return m_data.m_bSilent; };	//#IR +
	bool IsHelp()const { return m_data.m_bHelp; };	//#IR:100303 +

	bool GetNoRun()const;
	bool GetConfig()const;
	bool GetDebug()const;
	std::string GetDebugPath();
	bool GetAutoExitAfterLastViewer()const;
	bool GetAutoExitIfNoViewer()const;
	bool GetGeneratePassword();
	bool GetEnableDualMonitors();
	bool GetAutoRun();
	bool IsHelperTray();
	bool IsSubService();

	CUserData m_data;

	static bool ExtractParameter(std::string& rSrcCmdLine, std::string& rParameter );

	// static
	bool CheckParameter( const std::string strData, const char* szPrefix, std::string& rStrForSave );

protected:
	int m_argc;

	const char** m_argv;

	
	//std::string m_szAddress;
	//std::string m_szUsername;
	//std::string m_szPassword;

	bool m_bNoRun;
	bool m_bConfig;
	bool m_bDebug;
	bool m_bAutoExitAfterLastViewer;
	bool m_bAutoExitIfNoViewer;
	bool m_bGeneratePassword;
	bool m_bEnableDualMonitors;
	bool m_bAutoRun;
	bool m_bUseMiniGui;
	bool m_bIsHelperTray;
	bool m_bIsSubService;
	std::string m_sDebugPath;

	bool CheckNoRun(const char* szNext);
	bool CheckConfig(const char* szNext);
	bool CheckDebug(const char* szNext);
	bool CParseCmdLine::CheckAutoRun(const char* szNext);
	bool CheckHelperTray(const char *szNext);
	bool CheckSubService(const char *szNext);
	bool CheckAutoExitAfterLastViewer(const char* szNext);
	bool CheckAutoExitIfNoViewer(const char* szNext);
	bool CheckGeneratePassword(const char* szNext);
	bool CheckEnableDualMonitors(const char* szNext);
	bool CheckUseMiniGui(const char* szNext);

	bool CheckSilent(const char* szNext); //#IR +
	bool CheckHelp(const char* szNext); //#IR:100303 +

	bool CheckLockSettings(const char* szNext);

	bool CheckString( const char* szNext );
	bool CheckSubString( const std::string strNext );
	bool m_bCheckString;

	bool m_bStartTest;

};

#endif // !defined(AFX_PARSECMDLINE_H__5D83B50F_812C_4AC7_AEA7_0E311D64D455__INCLUDED_)
