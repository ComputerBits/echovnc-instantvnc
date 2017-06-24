//Copyright (c) 2008 Echogent Systems, Inc., a California  corporation. All rights reserved.

// memory format:
// int: totallength
// int: itemscount
// [iterator for each item:]
// int: item string size
// int item index
// char* string

#pragma once
#include <map>
#include <string>
#include "windows.h"
#include "Serialize.h"



typedef	std::map<int, std::string> MapString; 
class CTextData  
{
public:
	CTextData();
	virtual ~CTextData();
	int GetLength();
	bool ReadFromMemory( BYTE* pBuf, UINT uiSize );
	bool WriteDataToBuf( BYTE* pBuf, UINT uiSize );
	bool WriteDataToBuf(CSerialize &ser);
	MapString Data;
	static int ReplaceString(std::string &str, char* find, char* replace);
private:
	int nLengthAdd;

};
