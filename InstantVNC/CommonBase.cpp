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

#include <windows.h>
#include <VersionHelpers.h>
#include "CommonBase.h"

#define ERROR_GETFILESIZE    0xFFFFFFFF

#define _BUFF_SIZE		512

bool NS_Common::RunModule( const char* szModulePath, const std::vector<std::string>& rVectParameter,
	bool fWaitFor, char szParameterSeparator ) // '\n'
{
	char szSeparator[2] = {0, 0};
	szSeparator[0] = szParameterSeparator;

	char szCmdLine[_BUFF_SIZE];
	memset(szCmdLine, 0, _BUFF_SIZE);
	strcpy_s( szCmdLine, _BUFF_SIZE, szModulePath);
	for( UINT ui = 0; ui < rVectParameter.size(); ui++)
	{
		strcat_s( szCmdLine, _BUFF_SIZE, szSeparator );
		strcat_s( szCmdLine, _BUFF_SIZE, "\"");
		std::string strNext = rVectParameter[ui];
		strcat_s( szCmdLine, _BUFF_SIZE, strNext.c_str() );
		strcat_s( szCmdLine, _BUFF_SIZE, "\"");
	}
	STARTUPINFO stInfo;
	memset( &stInfo, 0, sizeof(stInfo));
	stInfo.cb = sizeof( stInfo );

	PROCESS_INFORMATION stPrInfo;
	memset( &stPrInfo, 0, sizeof(stPrInfo));
	if (!::CreateProcess( 0, szCmdLine, 0, 0, FALSE, CREATE_DEFAULT_ERROR_MODE,	0, 0, 
			&stInfo, &stPrInfo ))
	{
#ifdef _DEBUG
	::MessageBox(0, szCmdLine, "NS_Common::RunModule - FAILED", MB_OK);
#endif
		return false;
	}
	else
	{
		if (fWaitFor)
			WaitForSingleObject(stPrInfo.hThread, INFINITE);
		CloseHandle( stPrInfo.hThread );
		CloseHandle( stPrInfo.hProcess );

#ifdef _DEBUG
		::MessageBox(0, szCmdLine, "NS_Common::RunModule - debug", MB_OK);
#endif

		return true;
	}
}

bool NS_Common::RunElevated( char* szModulePath, const std::vector<std::string>& rVectParameter, bool fWaitFor, char szParameterSeparator ) // '\n'
{
	char szExecutable[_BUFF_SIZE];
	memset(szExecutable, 0, _BUFF_SIZE);
	strcpy_s(szExecutable,_BUFF_SIZE,szModulePath);

	
	char szSeparator[2] = {0, 0};
	szSeparator[0] = szParameterSeparator;

	char szCmdLine[_BUFF_SIZE];
	memset(szCmdLine, 0, _BUFF_SIZE);
	for( UINT ui = 0; ui < rVectParameter.size(); ui++)
	{
		strcat_s( szCmdLine, _BUFF_SIZE, szSeparator );
		strcat_s( szCmdLine, _BUFF_SIZE, "\"");
		std::string strNext = rVectParameter[ui];
		strcat_s( szCmdLine, _BUFF_SIZE, strNext.c_str() );
		strcat_s( szCmdLine, _BUFF_SIZE, "\"");
	}

	SHELLEXECUTEINFO shex;

	memset( &shex, 0, sizeof( shex) );

	char *p = strrchr(szModulePath, '\\');
	if (!p)
		p = strrchr(szModulePath, '/');
	TCHAR szFile[_MAX_PATH];
	memset(szFile, 0, _MAX_PATH);
	if (p)
	{
		strcpy_s(szFile, _MAX_PATH, p + 1);
		*(p) = 0;
	}
//DEMS
	shex.cbSize			= sizeof( SHELLEXECUTEINFO ); 
	shex.fMask			= SEE_MASK_NOCLOSEPROCESS; 
	shex.hwnd			= NULL;
	shex.lpVerb			= NULL;
	shex.lpFile			= szFile; 
	shex.lpParameters	= szCmdLine; 
	shex.lpDirectory	= szModulePath; 
	shex.nShow			= SW_NORMAL; 
//DEMS 
/*	if (::ShellExecuteEx( &shex ))
	{
		if (fWaitFor)
			WaitForSingleObject(shex.hProcess, INFINITE);
		CloseHandle(shex.hProcess);
		return true;
	}
	return false;
*///DEMS
	


	
		size_t iMyCounter = 0, iReturnVal = 0, iPos = 0;
		DWORD dwExitCode = 0;



		/* CreateProcess API initialization */
		STARTUPINFO siStartupInfo;
		PROCESS_INFORMATION piProcessInfo;
		memset(&siStartupInfo, 0, sizeof(siStartupInfo));
		memset(&piProcessInfo, 0, sizeof(piProcessInfo));
		siStartupInfo.cb = sizeof(siStartupInfo);
//		char* szExecutable = strcat(szModulePath,"\\");
//		szExecutable = strcat(szExecutable,szFile);
		if (CreateProcess(szExecutable,//const_cast<LPCWSTR>(FullPathToExe.c_str()),
								szCmdLine, 0, 0, false,
								CREATE_DEFAULT_ERROR_MODE, 0, 0,
								&siStartupInfo, &piProcessInfo) != false)
		{
			 /* Watch the process. */
			dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, INFINITE);
		}
		else
		{
			/* CreateProcess failed */
			return true;
		}

		/* Release handles */
		CloseHandle(piProcessInfo.hProcess);
		CloseHandle(piProcessInfo.hThread);

	return false;
	
}

bool NS_Common::WriteFileFromResource( UINT uiIdRes, const char* szPathFile )//, int& rLastError, CString* pStrComment )
{
//	rLastError = 0;
	bool bOK = false;
	HRSRC hRes = ::FindResource(NULL, MAKEINTRESOURCE(uiIdRes), RT_RCDATA);
	if (hRes)
	{
		// Load the resource and save its total size.
		DWORD dwSize = SizeofResource(NULL , hRes);
		HGLOBAL MemoryHandle = ::LoadResource(NULL, hRes);
		if (MemoryHandle != NULL)
		{
			// LockResource returns a BYTE pointer to the raw data in the resource
			BYTE *pMemPtr = (BYTE *)::LockResource(MemoryHandle);
			if (pMemPtr)
			{
				HANDLE hFile = ::CreateFile( szPathFile, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
					CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, (HANDLE)0);

				if (hFile != INVALID_HANDLE_VALUE)
				{
					DWORD dwReal;
					if (::WriteFile( hFile, pMemPtr, dwSize, &dwReal, 0) && dwSize == dwReal )
					{
						bOK = true;
					}
					::CloseHandle(hFile);
				}

			}
			else
			{
//				bOK = false;
//				rLastError = ::GetLastError();
//				if (pStrComment)
//					pStrComment->Format("Error <LockResource>");
			}
		}
		else
		{
//			rLastError = ::GetLastError();
//			if (pStrComment)
//				pStrComment->Format("Error <LoadResource>");
		}

		FreeResource((HANDLE)hRes);
	}
	else
	{
//		if (pStrComment)
//			pStrComment->Format("Error <FindResource>");
//
//		rLastError = ::GetLastError();
	}

	return bOK;
}



bool NS_Common::CreateMultiDirecrory( const char* szPath)
{
	UINT uiLen = strlen(szPath);
	for( UINT n = 3; n < uiLen; n++)
	{
		if ( szPath[n] == '\\' )
		{
			char sDir[_MAX_PATH];
			strcpy_s( sDir, _MAX_PATH, szPath );
			sDir[n] = 0;

			if (!IsDirectory(sDir))
			{
				if (!CreateDirectory( sDir, NULL))
					return false;
			}
		}
	}

	if (!IsDirectory(szPath) && !CreateDirectory( szPath, NULL))
		return false;
	else
		return true;
}


// Наличие логического диска
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


// Наличие директории. Допускается завершающий слеш
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


// Наличие файла
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


bool NS_Common::DeleteFolder(const char* szPath)
{
	bool bRes = false;
	SHFILEOPSTRUCT shFileOp;
	memset(&shFileOp, 0, sizeof(shFileOp));

	// Close source folder path by 2 zero
	int iLen = strlen( szPath ) + 1 + 1;
	char* pFrom = (char*)malloc(iLen);
	if (pFrom)
	{
		memset(pFrom, 0, iLen);
		strcpy_s(pFrom, iLen, szPath);
		if (pFrom[strlen(pFrom) - 1] == '\\')
			pFrom[strlen(pFrom) - 1] = 0;

		shFileOp.pFrom = pFrom;

		shFileOp.wFunc = FO_DELETE;
		shFileOp.fFlags |= FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;

		bRes = SHFileOperation(&shFileOp) == 0;
		free(pFrom);
	}

	return bRes;
}


bool NS_Common::SetFolderHidden( const char* szPath, bool bHidden )
{
	bool bOK = false;
	DWORD dw = ::GetFileAttributes( szPath );
	if ( dw != -1)
	{
		DWORD dwAttr;
		if (bHidden)
			dwAttr = dw | FILE_ATTRIBUTE_HIDDEN;
		else
			dwAttr = dw & ~FILE_ATTRIBUTE_HIDDEN;

		bOK = SetFileAttributes( szPath, dwAttr ) != 0 ? true:false;
	}

	return bOK;
}


bool NS_Common::SetFileHidden( const char* szPath, bool bHidden )
{
	bool bOK = false;
	DWORD dw = ::GetFileAttributes( szPath );
	if ( dw != -1)
	{
		DWORD dwAttr;
		if (bHidden)
			dwAttr = dw | FILE_ATTRIBUTE_HIDDEN;
		else
			dwAttr = dw & ~FILE_ATTRIBUTE_HIDDEN;

		bOK = SetFileAttributes( szPath, dwAttr ) != 0 ? true:false;
	}

	return bOK;
}


void NS_Common::GetErrorString(int iErrorCode, std::string &strErr)
{
	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0,
		iErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf, 0, 0 );

	strErr = std::string((const char*)lpMsgBuf);
	LocalFree( lpMsgBuf );
}


void NS_Common::DeleteInstance()
{
	char szAppPath[_MAX_PATH];
	::GetModuleFileName( NULL, szAppPath, sizeof(szAppPath));

	SetFileHidden( szAppPath, false );

	char szBatPath[_MAX_PATH], szDrive[_MAX_DRIVE], szDirect[ _MAX_DIR ]; 
	_splitpath_s( szAppPath, szDrive, _MAX_DRIVE, szDirect, _MAX_DIR, 0, 0, 0, 0 );
	_makepath_s( szBatPath, _MAX_PATH, szDrive, szDirect, "del", ".bat");

	char szCmdDelApp[_MAX_PATH];
	sprintf_s( szCmdDelApp, _MAX_PATH, "del \"%s\"", szAppPath );

	char szCmdDelBat[_MAX_PATH];
	sprintf_s( szCmdDelBat, _MAX_PATH, "del \"%s\"", szBatPath );
	
	FILE* pFile = NULL;

	if ( fopen_s(&pFile, szBatPath, "w+t") == 0 )
	{
		fprintf( pFile, "%s\n", szCmdDelApp );
		fprintf( pFile, "%s\n", szCmdDelBat );
		fclose(pFile);
	}

	::ShellExecute(NULL, "open", szBatPath, NULL, NULL, SW_SHOW);
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


bool NS_Common::GetFileSize( HANDLE hFile, DWORD& rDwFileSize)
{
	rDwFileSize = ::GetFileSize( hFile, NULL );
	return ( rDwFileSize != ERROR_GETFILESIZE );
}


bool NS_Common::RemoveReadOnly(const char* szPath, bool& bWasReadOnly)
{
	bool bOK = false;
	DWORD dw = ::GetFileAttributes( szPath );
	bWasReadOnly = ((dw & FILE_ATTRIBUTE_READONLY) != 0 ) ? true:false;
	if ( dw != -1)
	{
		bOK = SetFileAttributes( szPath, dw & ~FILE_ATTRIBUTE_READONLY ) != 0 ? true:false;
	}

	return bOK;
}


bool NS_Common::SetReadOnly(const char* szPath)
{
	bool bOK = false;
	DWORD dw = ::GetFileAttributes( szPath );
	if ( dw != -1)
	{
		bOK = SetFileAttributes( szPath, dw | FILE_ATTRIBUTE_READONLY ) != 0 ? true:false;
	}

	return bOK;
}

bool NS_Common::IsVista()
{
	OSVERSIONINFO osver;

	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	
	if (IsWindowsVistaOrGreater() && !IsWindows7OrGreater())
		return true;

	return false;
}
