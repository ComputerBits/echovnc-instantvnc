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
// EchowareDll.cpp: implementation of the CEchowareDll class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "EchowareDll.h"
#include <stdio.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEchowareDll::CEchowareDll()
:	m_hModEchowareProxyDll(0),
	m_pGetDllVersion(0)
{
}


CEchowareDll::~CEchowareDll()
{
	if (m_hModEchowareProxyDll)
	{
		::FreeLibrary(m_hModEchowareProxyDll);
	}
}


bool CEchowareDll::Init()
{
	m_hModEchowareProxyDll = LoadLibrary("Echoware.dll");
	if (!m_hModEchowareProxyDll)
	{
#ifdef _DEBUG
		printf("Failed to LoadLibrary.\n");
#endif
		return false;
	}

	m_pGetDllVersion = (LPFN_PROXYDLL_GET_DLLVERSION)
		GetProcAddress(m_hModEchowareProxyDll,"GetDllVersion");
	if (m_pGetDllVersion == NULL)
	{
#ifdef _DEBUG
		printf("Error - Function Pointer: GetDllVersion is NULL");
#endif
		return false;
	}

	return true;
}

const char* CEchowareDll::GetDllVersion()const
{
	return m_pGetDllVersion();
}
