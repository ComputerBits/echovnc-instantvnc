#ifndef _inc_common_base_h
#define _inc_common_base_h

#include <string>

namespace NS_Common
{

	bool WriteConfigFileName( const BYTE* pBuf, DWORD dwSize);

	bool ReadConfigFileNameSize(DWORD& dwSize);

	bool ReadConfigFileName( BYTE* pBuf, DWORD dwSize);

	// Returns full name of the file for storing command swithes
	std::string GetConfigFileName();

	// Returns full name of the miniWinVnc file
	std::string GetminiWinVncFileName();

	std::string GetAppCaption();

	void GetErrorString( std::string& strErr, int iErrorCode = 1);// -1 = GetLastError calling

		// Удалить файл, сняв read-only атрибут если необходимо
	bool SafeDeleteFile(const std::string& rStrPath);

	// Возвратить в *rbReadOnly* атрибут read-only
	bool GetReadOnly(const std::string& rStrPath, bool& rbReadOnly);

	// Убрать атрибут read-only
	bool RemoveReadOnly(const std::string& rStrPath);

	// Установить атрибут read-only
	bool SetReadOnly(const std::string& rStrPath);

	bool IsDrive( const char *cDriveLetter );

	bool IsDirectory( const char *szPath );

	bool IsFile( const char *szFilename );

	bool StartProcess( 
		const char* szPathProcess, 
		const char* szParam, 
		int& riLastError );

	void CalcRectCtlDialog( HWND hDialog, HWND hCtl, RECT& rcCtl );

	HWND GetConsole();

	int Message_Box( const char* szMsg, int iBtn = MB_OK, HWND hParent = 0);

	void ShowHideWindow(HWND hWnd, bool bShow);

}

#endif _inc_common_base_h