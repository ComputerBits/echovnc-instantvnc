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
//  WndProc.cpp

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#pragma once

#include <windows.h>
#include <stdlib.h>
#include <shellapi.h>

#include "AskDlg.h"
#include "..\InstantVNC\SerializeData.h"
#include "..\InstantVNC\CommonBase.h"
#include <TCHAR.h>

TCHAR *szComputerName = NULL;

CTextData objTextData;

//HINSTANCE g_hInstance = 0;

// Windows Entry Point
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iShow)
{
	DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
	szComputerName = (TCHAR*)malloc(dwSize + 1);
	if (szComputerName)
	{
		ZeroMemory(szComputerName, dwSize + 1);
		if (!GetComputerName(szComputerName, &dwSize))
			sprintf_s(szComputerName, dwSize + 1, "PC-%d", GetLastError());
	}

	TCHAR **args = NULL;
	int i = 0;
	//g_hInstance = hInstance;
	int cmdlinelen = _tcslen(lpCmdLine);
	TCHAR *cmd = new TCHAR[cmdlinelen + 1];
	_tcscpy_s(cmd, cmdlinelen + 1, lpCmdLine);
	if (cmdlinelen == 0)
	{
		char szPath[ _MAX_PATH ];
		::GetModuleFileName( 0, szPath, sizeof szPath);
		char szDrive[_MAX_DRIVE], szDirect[ _MAX_DIR ];
		_splitpath_s( szPath, szDrive, _MAX_DRIVE, szDirect, _MAX_DIR, 0, 0, 0, 0 );
		_makepath_s( szPath, _MAX_PATH, szDrive, szDirect, "InstantVNC", ".exe");
		args = new LPTSTR[1];
		args[i] = new char[_MAX_PATH];
		strcpy_s(args[i], _MAX_PATH, szPath);
	}
	else
	{

		// Count the number of spaces
		// This may be more than the number of arguments, but that doesn't matter.
		int nspaces = 0;
		TCHAR *p = cmd;
		TCHAR *pos = cmd;
		while ( ( pos = _tcschr(p, ' ') ) != NULL ) 
		{
			nspaces ++;
			p = pos + 1;
		}

		// Create the array to hold pointers to each bit of string
		args = new LPTSTR[nspaces + 1];

		// replace spaces with nulls and
		// create an array of TCHAR*'s which points to start of each bit.
		pos = cmd;
		args[i] = cmd;
		bool inquote=false;
		for (pos = cmd; *pos != 0; pos++) 
		{
			// Arguments are normally separated by spaces, unless there's quoting
			if ((*pos == ' ') && !inquote) 
			{
				*pos = '\0';
				p = pos + 1;
				args[++i] = p;
			}

			if (*pos == '"') 
			{  
				if (!inquote) 
				{      
					// Are we starting a quoted argument?
					args[i] = ++pos; // It starts just after the quote
				} 
				else 
				{
					*pos = '\0';     // Finish a quoted argument?
				}
				inquote = !inquote;
			}
		}
	}

	char szModulePath[_MAX_PATH];
	::GetModuleFileName(0, szModulePath, sizeof(szModulePath));

	char szDrive[_MAX_DRIVE], szDirect[ _MAX_DIR ];
	_splitpath_s( szModulePath, szDrive, _MAX_DRIVE, szDirect, _MAX_DIR, 0, 0, 0, 0 );

	char szImagePath[_MAX_PATH] = {0};
	bool bIsImage = false;
	CSerializeData data;

	data.ReadFromFile(args[i], false);

	if (data.m_data.m_dwSizeBinImage > 0 )
	{
		if (data.m_data.m_szImagePath.length())
		{
			char szExt[_MAX_EXT];
			_splitpath_s( data.m_data.m_szImagePath.c_str(), 0, 0, 0, 0, 0, 0, szExt, _MAX_EXT );
			_makepath_s( szImagePath, _MAX_PATH, szDrive, szDirect, "banner", szExt );
			if (NS_Common::IsFile(szImagePath))
			{
				::DeleteFile(szImagePath);
			}

			HANDLE hFile = ::CreateFile( szImagePath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
					CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, (HANDLE)0);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwReal;
				if (::WriteFile( hFile, data.m_data.m_pBinImage, data.m_data.m_dwSizeBinImage, &dwReal, 0) && 
					data.m_data.m_dwSizeBinImage == dwReal )
				{
					data.m_data.m_szImagePath = szImagePath;
					bIsImage = true;
				}
				::CloseHandle(hFile);

				NS_Common::SetFileHidden( szImagePath, true );
			}
			
		}
	}

	if (!bIsImage)
	{
		data.m_data.m_szImagePath.empty();
	}
	objTextData.Data.clear();
	if (data.m_data.m_objData->Data.size() > 0)
	{
		objTextData.Data.swap(data.m_data.m_objData->Data);
	}

	if (CConfigDlg::AskRun( &data, hInstance ))
	{
		int n = 0;
		if (objTextData.Data.size() > 0)
		{
			data.m_data.m_objData->Data.swap(objTextData.Data);
		}
		while (n < 25)
		{
			if (data.WriteIntoFile(args[i]))
				break;
			n++;
			Sleep(500);
		}
	}

	if (NS_Common::IsFile(szImagePath))
	{
		::DeleteFile(szImagePath);
	}

	if (cmd)
		delete [] cmd;
	if (args)
		delete [] args;

	if (szComputerName)
	{
		free(szComputerName);
		szComputerName = NULL;
	}

	return 0;
}