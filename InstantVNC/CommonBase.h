#ifndef _inc_Common_h
#define _inc_Common_h

#pragma warning (disable : 4786)
#include <vector>

namespace NS_Common
{
	bool RunModule( const char* szModulePath, const std::vector<std::string>& rVectParameter, bool fWaitFor = true,
		char szParameterSeparator = ' ');

	bool RunElevated( char* szModulePath, const std::vector<std::string>& rVectParameter, bool fWaitFor = true,
		char szParameterSeparator = ' ');

	bool IsVista();

	bool WriteFileFromResource( UINT uiIdRes, const char* szPathFile );

	bool CreateMultiDirecrory( const char* szPath);

	// Наличие логического диска
	bool IsDrive( const char *cDriveLetter );

	// Наличие директории. Допускается завершающий слеш
	bool IsDirectory( const char *szPath );

	// Наличие файла
	bool IsFile( const char *szFilename );

	// Удалить папку со всем ее содержимым (включая поддиректории) без запроса о подтверждении.
	bool DeleteFolder(const char* szPath);

	bool SetFolderHidden( const char* szPath, bool bHidden = true );

	bool SetFileHidden( const char* szPath, bool bHidden = true );

	void GetErrorString(int iErrorCode, std::string &strErr);

	void DeleteInstance();

	void CalcRectCtlDialog( HWND hDialog, HWND hCtl, RECT& rcCtl );

	bool GetFileSize( HANDLE hFile, DWORD& rDwFileSize);

	bool RemoveReadOnly(const char* szPath, bool& bWasReadOnly);

	bool SetReadOnly(const char* szPath);
}

#endif _inc_Common_h