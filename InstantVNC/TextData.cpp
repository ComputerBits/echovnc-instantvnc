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
//  TextData.cpp: implementation of the CTextData class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning( disable : 4786 )
#include "TextData.h"
#include "Serialize.h"

CTextData::CTextData()
{
}
//-------------------------------------------------------------------------------

CTextData::~CTextData()
{

}
//-------------------------------------------------------------------------------

int CTextData::GetLength()
{
	int iLen = 0;
	int nIntLen = sizeof(iLen);
	iLen += 2 * CSerialize::GetSize(nIntLen);
	MapString::iterator itr;
	std::string str;
	for (itr = Data.begin(); itr != Data.end(); itr++)
	{
		iLen += CSerialize::GetSize(itr->first);
		iLen += CSerialize::GetSize((std::string)itr->second);
		str = itr->second;
		iLen += ReplaceString(str, "\n", "#N");
		iLen += ReplaceString(str, "\r", "#R");
	}
	return iLen;
}
//-------------------------------------------------------------------------------

bool CTextData::ReadFromMemory( BYTE* pBuf, UINT uiSize )
{
	CSerialize ser( pBuf, uiSize);
	int nCount;

	if (!ser.LoadFromBuffer( nCount))
		return false;
	int nIndex = 0;
	std::string nData;
	for (int i = 0; i < nCount; i++)
	{
		if (!ser.LoadFromBuffer(nIndex))
			return false;
		nData.erase(nData.begin(), nData.end());
		if (!ser.LoadFromBuffer(nData))
			return false;
		ReplaceString(nData, "#N", "\n");
		ReplaceString(nData, "#R", "\r");
		Data[nIndex] = nData;
	}
	return true;
}
//-------------------------------------------------------------------------------

bool CTextData::WriteDataToBuf( BYTE* pBuf, UINT uiSize )
{
	CSerialize ser( pBuf, uiSize);
	return WriteDataToBuf(ser);
}
//-------------------------------------------------------------------------------

bool CTextData::WriteDataToBuf(CSerialize &ser)
{
	if (!ser.SaveToBuffer(Data.size()))
		return false;

	int nLen = CSerialize::GetSize(Data.size());
//	char ss[300];
//	sprintf(ss, "text len = %d,", nLen);
//	MessageBox(NULL, ss, "len", MB_OK); 
	MapString::iterator itr;
	for (itr = Data.begin(); itr != Data.end(); itr++)
	{
		if (!ser.SaveToBuffer(itr->first))
		{
			MessageBox(NULL, itr->second.c_str(), "Save item data fails", MB_OK);
			return false;
		}
//		nLen += CSerialize::GetSize(itr->first);
//		char ss[300];
//		sprintf(ss, "key len = %d", nLen);
//		MessageBox(NULL, ss, "len", MB_OK); 
		std::string str = itr->second;
		ReplaceString(str, "\n", "#N");
		ReplaceString(str, "\r", "#R");
//		MessageBox(NULL, str.c_str(), "Save item data", MB_OK);
		if (!ser.SaveToBuffer(str))
		{
//			MessageBox(NULL, str.c_str(), "Save item data", MB_OK);
			return false;
		}
//		nLen += CSerialize::GetSize(itr->second);
//		sprintf(ss, "data len = %d", nLen);
//		MessageBox(NULL, ss, "len", MB_OK); 
	}
	return true;
}
//-------------------------------------------------------------------------------

int CTextData::ReplaceString(std::string &str, char* find, char* replace)
{
	int nRes = 0;
	int nIndex = str.find(find);
	while (nIndex != std::string::npos)
	{
		str.erase(nIndex, strlen(find));
		str.insert(nIndex, replace);
		nIndex = str.find(find);
		nRes += strlen(replace) - strlen(find);
	}
	return nRes;
}