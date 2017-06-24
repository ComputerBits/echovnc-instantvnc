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
//  CommonBase.cpp

#include "Windows.h"
#include "CommonBase.h"
#include <stdio.h>


bool NS_Common::WriteConfigFileName( const BYTE* pBuf, DWORD dwSize)
{
	std::string s = GetConfigFileName();
	const char* szPath = s.c_str();
	if (IsFile(szPath))
	{
		if (!SafeDeleteFile(szPath))
		{
			return false;
		}
	}

	bool bOK = false;
	HANDLE hFile = 
		::CreateFile( szPath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
				CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, (HANDLE)0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwReal;
		if (::WriteFile( hFile, pBuf, dwSize, &dwReal, 0) && dwSize == dwReal )
		{
			bOK = true;
		}
		::CloseHandle(hFile);
	}

	if (bOK)
	{
		SetReadOnly(szPath);
	}
	else
	{
		SafeDeleteFile(szPath);
	}

	return bOK;
}


bool NS_Common::ReadConfigFileName( BYTE* pBuf, DWORD dwSize)
{
	std::string s = GetConfigFileName();
	const char* szPath = s.c_str();
	bool bOK = false;
	HANDLE hFile = ::CreateFile( szPath, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwReal;
		if (::ReadFile( hFile, pBuf, dwSize, &dwReal, 0) && dwSize == dwReal )
		{
			bOK = true;
		}
		::CloseHandle(hFile);
	}

	return bOK;
}


bool NS_Common::ReadConfigFileNameSize(DWORD& dwSize)
{
	std::string s = GetConfigFileName();
	const char* szPath = s.c_str();
	if (!IsFile(szPath))
	{
		return false;
	}

	bool bOK = false;
	HANDLE hFile = ::CreateFile( szPath, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwSize = GetFileSize( hFile, 0 );
		bOK = ( dwSize != INVALID_FILE_SIZE ) ? true:false;
		::CloseHandle(hFile);
	}
	return bOK;
}

extern const char	szAppName[];

std::string NS_Common::GetAppCaption()
{
// 	char szApp[_MAX_PATH];
// 	::GetModuleFileName( 0, szApp, sizeof(szApp));
// 	char szPath[_MAX_PATH]; 
// 	_splitpath_s( szApp, 0, 0, 0, 0, szPath, _MAX_PATH, 0, 0 );
// 	return std::string(szPath);

	return std::string(szAppName);
}

// Returns full name of the file for storing command switches
std::string NS_Common::GetConfigFileName()
{
	char szApp[_MAX_PATH];
	::GetModuleFileName( 0, szApp, sizeof(szApp));
	char szDrive[_MAX_DRIVE], szDirect[ _MAX_DIR ]; 
	_splitpath_s( szApp, szDrive, _MAX_DRIVE, szDirect, _MAX_DIR, 0, 0, 0, 0 );
	_makepath_s( szApp, _MAX_PATH, szDrive, szDirect, "echoWinVNC", ".dat");

	return std::string(szApp);
}


// Returns full name of the miniWinVnc file
std::string NS_Common::GetminiWinVncFileName()
{
	char szApp[_MAX_PATH];
	::GetModuleFileName( 0, szApp, sizeof(szApp));
	char szDrive[_MAX_DRIVE], szDirect[ _MAX_DIR ]; 
	_splitpath_s( szApp, szDrive, _MAX_DRIVE, szDirect, _MAX_DIR, 0, 0, 0, 0 );
	_makepath_s( szApp, _MAX_PATH, szDrive, szDirect, "miniWinVnc", ".exe");

	return std::string(szApp);
}


void NS_Common::GetErrorString( std::string& strErr, int iErrorCode )// -1 = GetLastError calling
{
	if ( iErrorCode == -1 )
	{
		iErrorCode = GetLastError();
	}

	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0,
		iErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf, 0, 0 );

	strErr = std::string((const char*)lpMsgBuf);
	LocalFree( lpMsgBuf );
}


// Удалить файл, сняв read-only атрибут если необходимо
bool NS_Common::SafeDeleteFile(const std::string& rStrPath)
{
	if (!IsFile( rStrPath.c_str() ))
		return true;

	bool bReadOnly;
	if (!GetReadOnly( rStrPath.c_str(), bReadOnly))
	{
		return false;
	}

	if (bReadOnly)
	{
		if (!RemoveReadOnly( rStrPath.c_str() ))
		{
			return false;
		}
	}

	return ::DeleteFile( rStrPath.c_str() ) != 0 ? true:false;
}


// Возвратить в *rbReadOnly* атрибут read-only
bool NS_Common::GetReadOnly(const std::string& rStrPath, bool& rbReadOnly)
{
	DWORD dw = ::GetFileAttributes( rStrPath.c_str() );
	if ( dw != -1)
	{
		rbReadOnly = ( dw & FILE_ATTRIBUTE_READONLY ) != 0 ? true:false;
		return true;
	}

	return false;
}


// Убрать атрибут read-only
bool NS_Common::RemoveReadOnly(const std::string& rStrPath)
{
	bool bOK = false;
	DWORD dw = ::GetFileAttributes( rStrPath.c_str() );
	if ( dw != -1)
	{
		bOK = SetFileAttributes( rStrPath.c_str(), dw & ~FILE_ATTRIBUTE_READONLY ) != 0 ? true:false;
	}

	return bOK;
}


// Установить атрибут read-only
bool NS_Common::SetReadOnly(const std::string& rStrPath)
{
	bool bOK = false;
	DWORD dw = ::GetFileAttributes( rStrPath.c_str() );
	if ( dw != -1)
	{
		bOK = SetFileAttributes( rStrPath.c_str(), dw | FILE_ATTRIBUTE_READONLY ) != 0 ? true:false;

	}

	return bOK;
}


bool NS_Common::IsDrive( const char *cDriveLetter )
{ 
	char* Sim = new char[2];
	memcpy( &Sim[0], cDriveLetter, 1 );
	Sim[1]  = 0;
	Sim = CharUpper( (LPTSTR)&Sim[0] );
	Sim[0] -= 'A';
	
	DWORD seek = 0;
	memcpy( &seek, Sim, 1 );
	delete Sim;

	DWORD me = 1;
	me <<= seek;
	if ( GetLogicalDrives() & me )
		return true;
	else
		return false;
}


bool NS_Common::IsDirectory( const char *szPath )
{ 
	if ( szPath == 0 ) 
		return false;

	if (!IsDrive( szPath ) || *( szPath + 1 ) != ':' ) 
		return false;

	if (lstrlen( szPath ) == 3 &&
		( szPath[ lstrlen( szPath ) - 1 ] == '\\' ||
		szPath[ lstrlen( szPath ) - 1 ] != '/' )   ) 
		return true;

	WIN32_FIND_DATA fData;

	char szDir[ 260 ];

	lstrcpy( szDir, szPath );

	if (szDir[ lstrlen( szDir ) - 1 ] == '\\' || 
		szDir[ lstrlen( szDir ) - 1 ] == '/' )
		szDir[ lstrlen( szDir ) - 1 ] = 0;

	HANDLE hFindFile = FindFirstFile( szDir, &fData );

	if ( hFindFile == INVALID_HANDLE_VALUE ) 
		return false;

	FindClose( hFindFile );

	if ( (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )
		return true;
	else  
		return false;
}


bool NS_Common::IsFile( const char *szFilename )
{ 
	if ( szFilename == 0 ) 
		return false;

	if (!IsDrive( szFilename ) ) 
		return false;

	WIN32_FIND_DATA fData;

	HANDLE hFindFile = FindFirstFile( szFilename, &fData );

	if ( hFindFile == INVALID_HANDLE_VALUE ) 
		return false;

	FindClose( hFindFile );

	if ( (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
		return true;
	else  
		return false;
}


bool NS_Common::StartProcess( 
	const char* szPathProcess, 
	const char* szParam, 
	int& riLastError )
{
	char szCmdLine[512];

	STARTUPINFO stInfo;
	memset( &stInfo, 0, sizeof(stInfo));
	stInfo.cb = sizeof( stInfo );

	PROCESS_INFORMATION innerPrInfo;

	sprintf_s( szCmdLine, 512, "\"%s\"", szPathProcess );
	if (szParam)
	{
		strcat_s( szCmdLine, 512, " " );
		strcat_s( szCmdLine, 512, szParam );
	}

	bool bRes;
	int flags = CREATE_DEFAULT_ERROR_MODE;

	if (!::CreateProcess( 0, szCmdLine, 0, 0, FALSE, CREATE_DEFAULT_ERROR_MODE,	0, 0, 
			&stInfo, &innerPrInfo ))
	{
		riLastError = ::GetLastError();
		bRes = false;
	}
	else
	{
		CloseHandle( innerPrInfo.hThread );
		bRes = true;
	}

	return bRes;
}


void NS_Common::CalcRectCtlDialog( HWND hDialog, HWND hCtl, RECT& rcCtl )
{ 
	RECT rc;
	int  top, left;

	::GetWindowRect( hDialog, &rc );
	top  = rc.top;
	top += GetSystemMetrics( SM_CYCAPTION );
	top += GetSystemMetrics( SM_CXDLGFRAME );

	left  = rc.left;
	left += GetSystemMetrics( SM_CYDLGFRAME );

	::GetWindowRect( hCtl, &rc );
	top  = rc.top  - top;
	left = rc.left - left;

	GetClientRect( hCtl, &rcCtl );

	rcCtl.left   += left;
	rcCtl.right  += left;
	rcCtl.top    += top;
	rcCtl.bottom += top;      
}


HWND NS_Common::GetConsole()
{

typedef HWND (*LPFN_GetConsoleWindow)();

	HWND hWnd = 0;
	HMODULE hMod = LoadLibrary("kernel32.dll");
	if (hMod)
	{
		LPFN_GetConsoleWindow pConsole =
			(LPFN_GetConsoleWindow)GetProcAddress(hMod,"GetConsoleWindow");
		if (pConsole)
		{
			hWnd = pConsole();
		}
		FreeLibrary(hMod);
	}

	return hWnd;
}


int NS_Common::Message_Box( const char* szMsg, int iBtn, HWND hParent)
{
	std::string sCaption = GetAppCaption();
	return ::MessageBox( hParent, szMsg, sCaption.c_str(), iBtn);
}


void NS_Common::ShowHideWindow(HWND hWnd, bool bShow)
{
	if (hWnd)
	{
		ShowWindow( hWnd, bShow ? SW_SHOW:SW_HIDE);
	}
}