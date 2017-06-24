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
//  SerializeData.cpp: implementation of the CSerializeData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "SerializeData.h"
#include "CommonBase.h"
#include "Serialize.h"


/*
** Some microsoft compilers lack this definition.
*/
#ifndef INVALID_SET_FILE_POINTER
# define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

#define DEBUG_LOG(s)	MessageBox(NULL, s, "Ask debug", MB_OK);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//#define ERROR_GETFILESIZE    0xFFFFFFFF

const char CSerializeData::m_szLabel[] = "773DD5BE-6C0C-4a91-8997-AE25DA7A2B6A";

CSerializeData::CSerializeData()
{
}


bool CSerializeData::ReadFromFile(const char* szFullPath, bool bDefaultSettings)
{
	if (SimpleReadFromFile(szFullPath))
	{
		if (( m_data.m_szAddress.length() == 0 ) && (bDefaultSettings) )
		{
			m_data.m_szAddress = "demo.echovnc.com";
		}

		if (( m_data.m_szPassword.length() == 0 ) && (bDefaultSettings) )
		{
			m_data.m_szPassword = "demo2010";
		}

		if (( m_data.m_szUsername.length() == 0 ) && (bDefaultSettings) )
		{
			DWORD len = MAX_COMPUTERNAME_LENGTH + 1;
			char szHostName[MAX_COMPUTERNAME_LENGTH + 1 + 1];
			memset(szHostName, 0, len + 1);

			if (!GetComputerName(szHostName, &len))
			{
				sprintf_s(szHostName, len + 1, "PC-%d", GetLastError());
			}

			m_data.m_szUsername = szHostName;
		}

		return true;
	}
	return false;
}


bool CSerializeData::SimpleReadFromFile(const char* szFullPath)
{
	bool bOK = false;
	HANDLE hFile = ::CreateFile( szFullPath, GENERIC_READ, FILE_SHARE_READ, NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		bOK = ReadFromFile(hFile);
		::CloseHandle(hFile);
	}

	return bOK;
}


bool CSerializeData::ReadFromFile(HANDLE hFile)
{
	bool bOK = false;
	UINT uiDataSize;
	if (CheckHeader( hFile, uiDataSize))
	{
		DWORD dwFileSize;
		if (NS_Common::GetFileSize( hFile, dwFileSize))
		{
			UINT uiOffset = uiDataSize + GetHeaderLength();
			if ( dwFileSize > uiOffset )
			{
				long lDistanceToMove = dwFileSize - uiOffset;
				if ( ::SetFilePointer( hFile, lDistanceToMove, 0, FILE_BEGIN ) != INVALID_SET_FILE_POINTER )
				{
					BYTE* pBuf = new byte[uiDataSize];
					DWORD dwReal;
					if (::ReadFile( hFile, pBuf, uiDataSize, &dwReal, 0) && uiDataSize == dwReal )
					{
						bOK = ReadFromMemory(pBuf, uiDataSize);
					}
					delete [] pBuf;
				}
			}
		}
	}
	return bOK;
}


UINT CSerializeData::GetHeaderLength()//static
{
	return (sizeof(m_szLabel) + sizeof(UINT));
}


bool CSerializeData::CheckHeader(HANDLE hFile, UINT& rUiDataSize)const
{
	bool bOK = false;
	DWORD dwFileSize;
	if (NS_Common::GetFileSize( hFile, dwFileSize))
	{
		DWORD dwHeaderLength = GetHeaderLength();
		if ( dwFileSize > dwHeaderLength )
		{
			long lDistanceToMove = dwFileSize - dwHeaderLength;
			if ( ::SetFilePointer( hFile, lDistanceToMove, 0, FILE_BEGIN ) != INVALID_SET_FILE_POINTER )
			{
				char* pBuf = new char[dwHeaderLength];
				DWORD dwReal;
				if (::ReadFile( hFile, pBuf, dwHeaderLength, &dwReal, 0) && dwHeaderLength == dwReal )
				{
					if ( memcmp( m_szLabel, pBuf, sizeof(m_szLabel) ) == 0 )
					{
						memcpy( &rUiDataSize, pBuf + sizeof(m_szLabel), sizeof(UINT));
						bOK = true;
					}
				}
				delete [] pBuf;
			}
		}
	}
	return bOK;
}


int CSerializeData::GetLength()const
{
	int iLen = 0;
	iLen += CSerialize::GetSize(m_data.m_szAddress);
	iLen += CSerialize::GetSize(m_data.m_szUsername);
	iLen += CSerialize::GetSize(m_data.m_szPassword);
	iLen += CSerialize::GetSize(m_data.m_szDebug);

	iLen += CSerialize::GetSize(m_data.m_szLockAddress);
	iLen += CSerialize::GetSize(m_data.m_szLockUsername);
	iLen += CSerialize::GetSize(m_data.m_szLockPassword);
	iLen += CSerialize::GetSize(m_data.m_szLockAdvProps);

	// Proxy data
	iLen += CSerialize::GetSize(m_data.m_ProxyData.m_strHost);
	iLen += CSerialize::GetSize(m_data.m_ProxyData.m_strUsername);
	iLen += CSerialize::GetSize(m_data.m_ProxyData.m_strPassword);

	iLen += CSerialize::GetSize(m_data.m_szImagePath);
	iLen += CSerialize::GetSizeBinaryData(m_data.m_dwSizeBinImage);
	
	iLen += CSerialize::GetSize(m_data.m_szAutoExitAfterLastViewer);
	iLen += CSerialize::GetSize(m_data.m_szAutoExitIfNoViewer);
	iLen += CSerialize::GetSize(m_data.m_szCreatePasswords);
	iLen += CSerialize::GetSize(m_data.m_szEnableDualMonitors);
	iLen += CSerialize::GetSize(m_data.m_szUseMiniGui);
	iLen += CSerialize::GetSize(m_data.m_szDisplayStartupHelp);
	int iTextLen  = m_data.m_objData->GetLength();
	iLen += iTextLen;
	return iLen;
}

bool CSerializeData::ReadFromMemory( BYTE* pBuf, UINT uiSize )
{
	CSerialize ser( pBuf, uiSize);

	if (!ser.LoadFromBuffer( m_data.m_szAddress))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szUsername))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szPassword))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szDebug))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szLockAddress))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szLockUsername))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szLockPassword))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szLockAdvProps))
		return false;

	// Proxy data
	if (!ser.LoadFromBuffer( m_data.m_ProxyData.m_strHost))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_ProxyData.m_strUsername))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_ProxyData.m_strPassword))
		return false;

	// Image
	if (!ser.LoadFromBuffer( m_data.m_szImagePath))
		return false;

	if (!ser.LoadFromBuffer( &m_data.m_pBinImage, m_data.m_dwSizeBinImage ))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szAutoExitAfterLastViewer))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szAutoExitIfNoViewer))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szCreatePasswords))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szEnableDualMonitors))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szUseMiniGui))
		return false;

	if (!ser.LoadFromBuffer( m_data.m_szDisplayStartupHelp))
		return false;

	//Texts
	int nSize = 0;
	if (!ser.LoadFromBuffer(nSize))
		return false;
	if (nSize > 0)
	{
		if (!m_data.m_objData->ReadFromMemory(pBuf + ser.m_iOffset, nSize - sizeof(int)))
			return false;
		ser.m_iOffset += nSize;
	}

	return true;
}


bool CSerializeData::WriteDataToBuf( BYTE* pBuf, UINT uiSize )
{
	CSerialize ser( pBuf, uiSize);
	if (!ser.SaveToBuffer(m_data.m_szAddress))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szUsername))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szPassword))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szDebug))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szLockAddress))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szLockUsername))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szLockPassword))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szLockAdvProps))
		return false;

	// Proxy data
	if (!ser.SaveToBuffer(m_data.m_ProxyData.m_strHost))
		return false;

	if (!ser.SaveToBuffer(m_data.m_ProxyData.m_strUsername))
		return false;

	if (!ser.SaveToBuffer(m_data.m_ProxyData.m_strPassword))
		return false;

	// Image
	if (!ser.SaveToBuffer(m_data.m_szImagePath))
		return false;

	bool bImage = false;
	if (m_data.m_szImagePath.length() != 0 && NS_Common::IsFile( m_data.m_szImagePath.c_str() ))
	{
		HANDLE hFile = ::CreateFile( m_data.m_szImagePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwFileSize;
			if (NS_Common::GetFileSize( hFile, dwFileSize))
			{
				m_data.m_pBinImage = new BYTE[dwFileSize];
				DWORD dwReal;
				if (::ReadFile( hFile, m_data.m_pBinImage, dwFileSize, &dwReal, 0) && dwFileSize == dwReal )
				{
					m_data.m_dwSizeBinImage = dwFileSize;
					bImage = true;
				}

			}
			::CloseHandle(hFile);
		}
	}

	if (!bImage)
	{
		m_data.m_dwSizeBinImage = 0;
		if (m_data.m_pBinImage)
		{
			delete [] m_data.m_pBinImage;
		}
	}

	if (!ser.SaveToBuffer(m_data.m_pBinImage, m_data.m_dwSizeBinImage))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szAutoExitAfterLastViewer))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szAutoExitIfNoViewer))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szCreatePasswords))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szEnableDualMonitors))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szUseMiniGui))
		return false;

	if (!ser.SaveToBuffer(m_data.m_szDisplayStartupHelp))
		return false;

	//Text
	int nSize = m_data.m_objData->GetLength();
	if (!ser.SaveToBuffer(nSize))
		return false;
	if (!m_data.m_objData->WriteDataToBuf(ser))
		return false; 
	return true;
}


bool CSerializeData::GetSizeImageFromDrive( DWORD& rDwImageSize)const
{
	bool bOK = false;
	if (m_data.m_szImagePath.length() != 0 && NS_Common::IsFile( m_data.m_szImagePath.c_str() ))
	{
		HANDLE hFile = ::CreateFile( m_data.m_szImagePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			bOK = NS_Common::GetFileSize( hFile, rDwImageSize);
			::CloseHandle(hFile);
		}
	}

	return bOK;
}

//////////////////////////////////////////////////////


bool CSerializeData::IsData(const char* szFullPath, UINT* pDataSize)const
{
	bool bIsData = false;
	HANDLE hFile = ::CreateFile( szFullPath, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		UINT uiDataSize;
		bIsData = CheckHeader( hFile, uiDataSize);
		if (pDataSize)
		{
			*pDataSize = uiDataSize;
		}
		::CloseHandle(hFile);
	}

	return bIsData;
}


bool CSerializeData::WriteIntoFile(const char* szFullPath)
{
	bool bWasReadOnly;
	if (!NS_Common::RemoveReadOnly( szFullPath, bWasReadOnly))
	{
		return false;
	}

	UINT uiOldDataSize;
	if (IsData(szFullPath, &uiOldDataSize))
	{
		if (!RemoveData( szFullPath, uiOldDataSize))
		{
			return false;
		}
	}
	if (NS_Common::IsFile(m_data.m_szIcon.c_str()))
	{
		BYTE *pBuffer = NULL;
		DWORD dwBufferSize = 0;

		ReadIconToBuffer(m_data.m_szIcon.c_str(), pBuffer, dwBufferSize);
		ChangeAppIcon(szFullPath, pBuffer, dwBufferSize);

		if (pBuffer)
			free(pBuffer);
	}

	bool bOK = false;
	DWORD dwImageSize;
	if (GetSizeImageFromDrive(dwImageSize))
	{
		m_data.m_dwSizeBinImage = dwImageSize;
	}

	DWORD dwDataSize = GetLength();

	BYTE* pBuf = new BYTE[dwDataSize];
	if (WriteDataToBuf( pBuf, dwDataSize ))
	{
//		DEBUG_LOG("WriteDataToBuf ok")
		HANDLE hFile = ::CreateFile( szFullPath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
						OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, (HANDLE)0);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			if ( ::SetFilePointer( hFile, 0, 0, FILE_END ) != INVALID_SET_FILE_POINTER )
			{
				DWORD dwReal;
				if (::WriteFile( hFile, pBuf, dwDataSize, &dwReal, 0) && dwDataSize == dwReal )
				{
					if (::WriteFile( hFile, m_szLabel, sizeof(m_szLabel), &dwReal, 0) && sizeof(m_szLabel) == dwReal )
					{
						if (::WriteFile( hFile, &dwDataSize, sizeof(dwDataSize), &dwReal, 0) && sizeof(dwDataSize) == dwReal )
						{
//							DEBUG_LOG("WriteFile ok")
							bOK = true;
						}
					}

				}
			}
			::CloseHandle(hFile);
		}
	}

	if (bWasReadOnly)
	{
		NS_Common::SetReadOnly(szFullPath);
	}

	return bOK;
}


bool CSerializeData::RemoveData(const char* szFullPath, UINT uiOldDataSize)const
{
	uiOldDataSize += GetHeaderLength();
	bool bOK = false;
	HANDLE hFile = ::CreateFile( szFullPath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
			OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, (HANDLE)0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwFileSize;
		if (NS_Common::GetFileSize( hFile, dwFileSize))
		{
			if ( dwFileSize > uiOldDataSize )
			{
				long lDistanceToMove = dwFileSize - uiOldDataSize;
				if ( ::SetFilePointer( hFile, lDistanceToMove, 0, FILE_BEGIN ) != INVALID_SET_FILE_POINTER )
				{
					bOK = SetEndOfFile( hFile ) ? true:false;
				}
			}
		}

		::CloseHandle(hFile);
	}

	return bOK;
}

BOOL CSerializeData::ChangeAppIcon(const char* szFullPath, BYTE *pBuffer, DWORD dwBufferSize)
{
	BOOL res = FALSE;
	if (szFullPath && pBuffer && dwBufferSize > 0)
	{
		HANDLE hUpdateRes = BeginUpdateResource(szFullPath, FALSE);
		if (hUpdateRes)
		{
			int d = 22;// read from 22 byte from the beginning
			res = UpdateResource(hUpdateRes, RT_ICON, MAKEINTRESOURCE(1),
				MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPVOID)(pBuffer + d), 
				dwBufferSize - d);
			if (res)
				res = EndUpdateResource(hUpdateRes, FALSE);
		}
	}
	return res;
}

BOOL CSerializeData::ReadIconToBuffer(const char* szFullPath, BYTE *&pBuffer, DWORD &dwBufferSize)
{
	if (!szFullPath) return FALSE;

	BOOL bRes = FALSE;
	HANDLE hFile = CreateFile(szFullPath, GENERIC_READ, FILE_SHARE_READ, 
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwBufferSize = GetFileSize(hFile, NULL);
		if (dwBufferSize != INVALID_FILE_SIZE)
		{
			if (pBuffer)
				pBuffer = (BYTE*)realloc(pBuffer, dwBufferSize);
			else
				pBuffer = (BYTE*)malloc(dwBufferSize);

			if (pBuffer)
			{
				ZeroMemory(pBuffer, dwBufferSize);
				DWORD dwReaded;
				bRes = ReadFile(hFile, pBuffer, dwBufferSize, &dwReaded, NULL);
			}

		}
		CloseHandle(hFile);
	}
	return bRes;
}
