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
#pragma warning (disable : 4786)

#include <windows.h>
#include "resource.h"
#include "CommonBase.h"
#include "unzip.h"
#include "SerializeData.h"
#include <shellapi.h>
#include "StartDlg.h"

//(#updated:IR:100614: +
//v.1
//#include <Userenv.h> // to retrieve user profile directories
//#pragma comment(lib, "Userenv.lib")
////Requires Userenv.dll
//v.2
#include <sys/stat.h>
//Requires shfolder.dll
#include <shlobj.h> // for SHGetFolderPath
#pragma comment(lib, "shell32.lib")
//)


HINSTANCE g_hInstance = 0;
bool m_fDebug = false;

TCHAR *szComputerName = NULL;

int MsgBox(const char* szText, int iBtn = MB_OK, HWND hParent = 0)
{
	return MessageBox(hParent, szText, "SetupMiniWinVNC", iBtn);
}

bool WriteTmpBanner(CSerializeData& ser, const char* szPathFolder)
{
	bool bOK = false;
	if (ser.m_data.m_szImagePath.length() && ser.m_data.m_dwSizeBinImage > 0)
	{
		char szExt[_MAX_EXT];
		_splitpath_s( ser.m_data.m_szImagePath.c_str(), 0, 0, 0, 0, 0, 0, szExt, _MAX_EXT );
		char szImagePath[_MAX_PATH];
		strcpy_s( szImagePath, _MAX_PATH, szPathFolder );
		if ( szImagePath[ strlen( szImagePath ) - 1 ] != '\\' )
		{
			strcat_s( szImagePath, _MAX_PATH, "\\" );
		}
		strcat_s( szImagePath, _MAX_PATH, "banner" );
		strcat_s( szImagePath, _MAX_PATH, szExt );

		if (NS_Common::IsFile(szImagePath))
		{
			::DeleteFile(szImagePath);
		}

		HANDLE hFile = ::CreateFile( szImagePath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
				CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, (HANDLE)0);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwReal;
			if (::WriteFile( hFile, ser.m_data.m_pBinImage, ser.m_data.m_dwSizeBinImage, &dwReal, 0) && 
				ser.m_data.m_dwSizeBinImage == dwReal )
			{
				ser.m_data.m_szImagePath = szImagePath;
				bOK = true;
			}
			::CloseHandle(hFile);
		}

	}
	return bOK;
}

bool WriteTmpIni(CSerializeData& ser, const char* szPathFolder)
{
	bool bOK = false;
	if (ser.m_data.m_objData->Data.size())
	{
		char szImagePath[_MAX_PATH];
		strcpy_s( szImagePath, _MAX_PATH, szPathFolder );
		strcat_s( szImagePath, _MAX_PATH, "data.ini" );

		if (NS_Common::IsFile(szImagePath))
		{
			::DeleteFile(szImagePath);
		}

		HANDLE hFile = ::CreateFile( szImagePath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
				CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, (HANDLE)0);
		
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hFile);
			MapString::iterator itr;
			char buff[_MAX_PATH];
			WritePrivateProfileSection("Text", "", szImagePath);
			for (itr = ser.m_data.m_objData->Data.begin(); itr != ser.m_data.m_objData->Data.end(); itr++)
			{
				CTextData::ReplaceString(itr->second, "\n", "#N");
				CTextData::ReplaceString(itr->second, "\r", "#R");
				sprintf_s(buff, _MAX_PATH, "%d", itr->first);
				WritePrivateProfileString("Text", buff, itr->second.c_str(), szImagePath);
			}
			bOK = true; 
		}
	}
	return bOK;
}

bool GetTemporaryFolder(char* szPathFolder, int nPathFolderLen)
{
	SYSTEMTIME st;
	char st_buffer[32];
	
	GetSystemTime(&st);
	_snprintf(st_buffer, sizeof(st_buffer), "_%d-%d-%d_%d-%d-%d-%d",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond,
		st.wMilliseconds);

	bool ok = false;
	if (szPathFolder)
	{
		char szPath[_MAX_PATH];
		memset(szPath, 0, _MAX_PATH);

		char *ptempvar;
		size_t len;

		if(_dupenv_s(&ptempvar, &len, "TEMP") == 0)
			strcpy_s(szPath, _MAX_PATH, ptempvar);
		else
			strcpy_s(szPath, _MAX_PATH, "");
		if (szPath[strlen(szPath) - 1] != '\\')
			strcat_s(szPath, _MAX_PATH, "\\");
		strcat_s(szPath, _MAX_PATH, "InstantVNC");
		strcat_s(szPath, _MAX_PATH, st_buffer);
		strcat_s(szPath, _MAX_PATH, "\\");

		NS_Common::DeleteFolder(szPath);
		NS_Common::CreateMultiDirecrory(szPath);

/*
		GetModuleFileName(NULL, szPath, _MAX_PATH);
		char szDrive[_MAX_DRIVE], szDirect[ _MAX_DIR ];
		_splitpath( szPath, szDrive, szDirect, 0, 0 );
		_makepath(szPath, szDrive, szDirect, NULL, NULL);
*/
		int n = 0;
		while (true)
		{
			if (n == 0)
				sprintf_s(szPathFolder, nPathFolderLen, "%sTMP\\", szPath);
			else
				sprintf_s(szPathFolder, nPathFolderLen, "%sTMP%d\\", szPath, n);

			if (!NS_Common::IsDirectory(szPathFolder))
				break;
			else
				NS_Common::DeleteFolder(szPathFolder);
			n++;
		}
		if (!NS_Common::CreateMultiDirecrory(szPathFolder))
		{
			char szMsg[_MAX_PATH + _MAX_PATH];
			sprintf_s(szMsg, _MAX_PATH + _MAX_PATH, "Failed to create folder\n%s", szPathFolder);
			MsgBox(szMsg, MB_OK, 0);
		}
		else
		{
			ok = true;
			NS_Common::SetFolderHidden(szPathFolder, true);
		}
	}
	return ok;
}

void DeleteTemporaryFolder(char* szPathFolder)
{
	if (szPathFolder)
	{
		while (NS_Common::IsDirectory(szPathFolder))
		{
			if (NS_Common::DeleteFolder(szPathFolder))
			{
				break;
			}
			Sleep(500);
		}
	}
}

bool Extract(const char *szFolder, UINT uiRes, char** &files)
{
	bool bOK = true;
	char szTmpBin[_MAX_PATH];
	sprintf_s(szTmpBin, _MAX_PATH, "%stmp.zip", szFolder);
	if (!NS_Common::WriteFileFromResource(uiRes, szTmpBin))
	{
		return false;
	}
	HZIP hz = OpenZip(szTmpBin, 0);
	if (hz != 0)
	{
		ZIPENTRY ze;
		// -1 gives overall information about the zipfile
		GetZipItem(hz, -1, &ze);
		int numitems = ze.index;
		files = (char**)calloc(numitems, _MAX_PATH);
		for (int zi = 0; zi < numitems; zi++)
		{
			ZIPENTRY ze; 
			GetZipItem(hz, zi, &ze); // fetch individual details

			char szFname[_MAX_FNAME], szExt[_MAX_EXT], sPath[_MAX_PATH];
			_splitpath_s(ze.name, 0, 0, 0, 0, szFname, _MAX_FNAME, szExt, _MAX_EXT);
			strcpy_s(sPath, _MAX_PATH, szFolder);
			strcat_s(sPath, _MAX_PATH, szFname);
			strcat_s(sPath, _MAX_PATH, szExt);

			files[zi] = (char*)malloc(_MAX_PATH);
			strcpy_s(files[zi], _MAX_PATH, sPath);

			if (UnzipItem(hz, zi, sPath) != 0)
			{
				bOK = false;
				break;
			}
		}

		CloseZip(hz);
		DeleteFile(szTmpBin);
	}
	else
	{
		bOK = false;
	}
	return bOK;
}

bool ExtractMain(const char *szFolder, char** &files)
{
	return Extract(szFolder, IDR_BIN_MAIN, files);
}

bool ExtractDll(const char *szFolder, char** &files)
{
	return Extract(szFolder, IDR_BIN_DLL, files);
}

bool ExtractAsk(const char *szFolder, char** &files)
{
	return Extract(szFolder, IDR_ASK, files);
}

static int getStartupHelpStatus(const char* szPath)
{
	char **files = NULL;
	if (!ExtractDll(szPath, files))
	{
		MsgBox("Failed extracting binary - Is Remote Support Allready Running?", MB_OK, 0);
		if (files)
			free(files);
		return -1;
	}
	if (files)
	{
		free(files);
		files = NULL;
	}
	if (!ExtractMain(szPath, files))
	{
		MsgBox("Failed extracting binary - Is Remote Support Allready Running?", MB_OK, 0);
		if (files)
			free(files);
		return -1;
	}

	char szModulePath[_MAX_PATH];
	::GetModuleFileName(0, szModulePath, sizeof(szModulePath));
	CSerializeData ser;
	if (ser.ReadFromFile(szModulePath, true))
	{
		if (strlen(ser.m_data.m_szDisplayStartupHelp.c_str()) != 0)
		{
			char szParameter[_MAX_PATH];
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szDisplayStartupHelp.c_str());
			return 1;
		}
	}
	return 0;
}

void Install_and_Run_MainFiles(const char* szPath)
{
	char **files = NULL;
	if (!ExtractDll(szPath, files))
	{
		MsgBox("Failed extracting binary - Is Remote Support Allready Running?", MB_OK, 0);
		if (files)
			free(files);
		return;
	}
	if (files)
	{
		free(files);
		files = NULL;
	}
	if (!ExtractMain(szPath, files))
	{
		MsgBox("Failed extracting binary - Is Remote Support Allready Running?", MB_OK, 0);
		if (files)
			free(files);
		return;
	}

	std::vector<std::string> vectParameter;

	char szModulePath[_MAX_PATH];
	::GetModuleFileName(0, szModulePath, sizeof(szModulePath));
	CSerializeData ser;
	if (ser.ReadFromFile(szModulePath, true))
	{
		char szParameter[_MAX_PATH];

		sprintf_s(szParameter, _MAX_PATH, "Address=%s", ser.m_data.m_szAddress.c_str());
		vectParameter.push_back(szParameter);

		sprintf_s(szParameter, _MAX_PATH, "Username=%s", ser.m_data.m_szUsername.c_str());
		vectParameter.push_back(szParameter);

		sprintf_s( szParameter, _MAX_PATH, "Password=%s", ser.m_data.m_szPassword.c_str());
		vectParameter.push_back(szParameter);

		if (!WriteTmpBanner(ser, szPath))
		{
			ser.m_data.m_szImagePath.empty();
		}
		sprintf_s( szParameter, _MAX_PATH, "Image=%s", ser.m_data.m_szImagePath.c_str());
		vectParameter.push_back(szParameter);

		if (WriteTmpIni(ser, szPath))
		{
			sprintf_s(szParameter, _MAX_PATH, "UseIni=%s\\data.ini", szPath);
			vectParameter.push_back(szParameter);
		}

		// Proxy
		sprintf_s(szParameter, _MAX_PATH, "Haddr=%s", ser.m_data.m_ProxyData.m_strHost.c_str());
		vectParameter.push_back(szParameter);

		sprintf_s(szParameter, _MAX_PATH, "Huser=%s", ser.m_data.m_ProxyData.m_strUsername.c_str());
		vectParameter.push_back(szParameter);

		sprintf_s(szParameter, _MAX_PATH, "Hpwd=%s", ser.m_data.m_ProxyData.m_strPassword.c_str());
		vectParameter.push_back(szParameter);

		if (strlen(ser.m_data.m_szLockAddress.c_str()) != 0)
		{
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szLockAddress.c_str());
			vectParameter.push_back(szParameter);
		}

		if (strlen(ser.m_data.m_szLockUsername.c_str()) != 0)
		{
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szLockUsername.c_str());
			vectParameter.push_back(szParameter);
		}

		if (strlen(ser.m_data.m_szLockPassword.c_str()) != 0)
		{
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szLockPassword.c_str());
			vectParameter.push_back(szParameter);
		}

		if (strlen(ser.m_data.m_szLockAdvProps.c_str()) != 0)
		{
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szLockAdvProps.c_str());
			vectParameter.push_back(szParameter);
		}

		if (strlen(ser.m_data.m_szDebug.c_str()) != 0)
		{
			TCHAR path[_MAX_PATH];
			GetModuleFileName(NULL, path, _MAX_PATH);
			TCHAR *p = strrchr(path, '\\');
			if (!p) p = strrchr(path, '/');
			if (p) *(++p) = 0;

			m_fDebug = true;
//			sprintf_s(szParameter, _MAX_PATH, "%s=%s", ser.m_data.m_szDebug.c_str(), szPath);
			sprintf_s(szParameter, _MAX_PATH, "%s=%s", ser.m_data.m_szDebug.c_str(), path);
			vectParameter.push_back(szParameter);
		}

		if (strlen(ser.m_data.m_szAutoExitAfterLastViewer.c_str()) != 0)
		{
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szAutoExitAfterLastViewer.c_str());
			vectParameter.push_back(szParameter);
		}

		if (strlen(ser.m_data.m_szAutoExitIfNoViewer.c_str()) != 0)
		{
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szAutoExitIfNoViewer.c_str());
			vectParameter.push_back(szParameter);
		}

		if (strlen(ser.m_data.m_szCreatePasswords.c_str()) != 0)
		{
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szCreatePasswords.c_str());
			vectParameter.push_back(szParameter);
		}

		if (strlen(ser.m_data.m_szEnableDualMonitors.c_str()) != 0)
		{
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szEnableDualMonitors.c_str());
			vectParameter.push_back(szParameter);
		}

		if (strlen(ser.m_data.m_szUseMiniGui.c_str()) != 0)
		{
			sprintf_s(szParameter, _MAX_PATH, "%s", ser.m_data.m_szUseMiniGui.c_str());
			vectParameter.push_back(szParameter);
		}
	}
	if (NS_Common::IsVista())
		NS_Common::RunElevated(files[0], vectParameter);
	else
		NS_Common::RunModule(files[0], vectParameter);

	if (files)
		free(files);
}

void Install_and_Run_AskParameter(const char* szPath)
{
	char **files = NULL;
	ExtractDll(szPath, files);
	if (files)
	{
		free(files);
		files = NULL;
	}
	if (!ExtractAsk(szPath, files))
	{
		MsgBox("Failed extracting binary - Is Remote Support Allready Running?", MB_OK, 0);
		if (files)
			free(files);
		return;
	}

	char szModulePath[_MAX_PATH];
	::GetModuleFileName(0, szModulePath, sizeof(szModulePath));
	std::vector<std::string> vectParameter;
	vectParameter.push_back(szModulePath);

 	NS_Common::RunModule(files[0], vectParameter, false);

	if (files)
		free(files);
}

void ChangeIcon( const HICON hNewIcon )
{
	// Load kernel 32 library
	HMODULE hMod = LoadLibrary( ( "Kernel32.dll" ));

	if (hMod)
	{
		// Load console icon changing procedure
		typedef DWORD ( __stdcall *SCI )( HICON );
		SCI pfnSetConsoleIcon = reinterpret_cast<SCI>( GetProcAddress( hMod, "SetConsoleIcon" ));

		if (pfnSetConsoleIcon)
			// Call function to change icon
			pfnSetConsoleIcon( hNewIcon ); 

		FreeLibrary( hMod );
	}
}// End ChangeIcon 

// Windows Entry Point
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iShow)
{
	DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 2;
	szComputerName = (TCHAR*)malloc(dwSize);
	if (szComputerName)
	{
		ZeroMemory(szComputerName, dwSize);
		GetComputerName(szComputerName, &dwSize);
	}

	g_hInstance = hInstance;
	bool bShift = (::GetKeyState( VK_SHIFT ) & 0x8000) != 0 ? true : false;
#ifdef _DEBUG
//	bShift = true;
#endif

	char temp[_MAX_PATH];
	memset(temp, 0, _MAX_PATH);

//#updated:IR:100614: -
//	bool ok = GetTemporaryFolder(temp, _MAX_PATH);

//(#updated:IR:100614: + : new approach (see description below)
//If "c:\users\<user name>" exists, it extracts to a temp folder in there;
//if that path doesn't exist, it should keep doing what it's doing now.
	bool ok = false;
	//Retrieve the path to the root directory of the specified user's profile
	//Possible location:
	//CSIDL_APPDATA
	//		Is for file system directory that serves as a common repository for application-specific data. 
	//		A typical path is C:\Documents and Settings\username\Application Data. 
	//CSIDL_LOCAL_APPDATA
	//		Is for file system directory that serves as a data repository for local (nonroaming) applications. 
	//		A typical path is C:\Documents and Settings\username\Local Settings\Application Data.
	//CSIDL_INTERNET_CACHE
	//		Is for file system directory that serves as a common repository for temporary Internet files. 
	//		A typical path is C:\Documents and Settings\username\Local Settings\Temporary Internet Files.
	bool bUserTempFolder = false;
	//(#updated:IR:100618: * : new approach with c:\users\{username} instead of SHGetFolderPath()
//	HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_DEFAULT, temp); //note: returns path without ending backslash
	HRESULT hr = E_FAIL;
	strcpy(temp, "c:\\users\\");
	char szUser[_MAX_PATH];
	DWORD dwInSize = sizeof(szUser)/sizeof(char);
	if (FALSE != ::GetUserName(szUser, &dwInSize)) {
		strcat(temp, szUser);
		hr = S_OK;
	}
	//)
	if (SUCCEEDED(hr)) {
		// attempt to get the file/directory attributes 
		struct stat stFileInfo; 
		int intStat = stat(temp, &stFileInfo); //note: file/dir must be withot ending backslash
		if (0 == intStat) { 
			// We were able to get the file/dir attributes so it exists

			SYSTEMTIME st;
			char st_buffer[32];
			
			GetSystemTime(&st);
			_snprintf(st_buffer, sizeof(st_buffer), "_%d-%d-%d_%d-%d-%d-%d",
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond,
				st.wMilliseconds);

			strcat_s(temp, _MAX_PATH, "\\AppData\\Local\\Temp\\InstantVNC");
			strcat_s(temp, _MAX_PATH, st_buffer);

			intStat = stat(temp, &stFileInfo);
			ok = true;
			bUserTempFolder = true;
			strcat(temp, "\\"); 
			if (0 != intStat) { // not exists, try to create
				if (FALSE == ::CreateDirectory(temp, NULL)) {
					ok = false;
					bUserTempFolder = false;
				}
			}
		}
		else {
			ok = false;
		}
	}
	if (!ok) {
		ok = GetTemporaryFolder(temp, _MAX_PATH); //note: returns path with ending backslash
	}
//)

	if (ok)
	{
		if (bShift)
		{
			Install_and_Run_AskParameter(temp);
		}
		else
		{
			int result;
			result = getStartupHelpStatus(temp);
			if (result == 0)
			{
				result = IDC_START_RUN;
			}
			else if (result == 1)
			{
				result = CStartDlg::ShowStartup(hInstance);
			}
			switch (result)
			{
				case IDC_START_RUN:
					Install_and_Run_MainFiles(temp);
/*
					if (!m_fDebug)
					{
						DeleteTemporaryFolder(temp);
					}
					else
					{
						NS_Common::SetFolderHidden(temp, false);
					}
*/
//#updated:IR:100614: -
					//DeleteTemporaryFolder(temp);
//(#updated:IR:100614: +
					if (!bUserTempFolder) {
						DeleteTemporaryFolder(temp);
					}
					else {
						if ('\\' == temp[strlen(temp)-1]) {
							temp[strlen(temp)-1] = 0;
						}
						// Fully qualified name of the directory being deleted, without trailing backslash 
						SHFILEOPSTRUCT file_op = { 
							NULL, 
							FO_DELETE, 
							temp, 
							"", 
							FOF_NOCONFIRMATION | 
							FOF_NOERRORUI | 
							FOF_SILENT, 
							false, 
							0, 
							"" }; 
						SHFileOperation(&file_op); 
					}
					break;

				case IDC_START_ASK:
					Install_and_Run_AskParameter(temp);
					break;

				default:
					return 0;
			}
		}
	}

	if (szComputerName)
	{
		free(szComputerName);
		szComputerName = NULL;
	}

	return 0;
}
