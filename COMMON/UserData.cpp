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
//  UserData.cpp: implementation of the CUserData class.
//
//////////////////////////////////////////////////////////////////////

#include "Windows.h"
#include "UserData.h"
//#include "serialize.h"
//#include "CommonBase.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUserData::CUserData()
:	m_szAddress(""),
	m_szUsername(""),
	m_szPassword(""),
	m_szImagePath(""),
	m_szPort(""),
	m_vncPassword(""),
	m_bLockAddress(false),
	m_bLockUsername(false),
	m_bLockPassword(false),
	m_bLockAdvProps(false),
	m_bSilent(false), //#IR +
	m_bHelp(false), //#IR:100303 +
	m_proxy(),
	m_bClose(FALSE),
	m_szIniPath("")
{
}


int CUserData::ReplaceString(std::string &str, char* find, char* replace)
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