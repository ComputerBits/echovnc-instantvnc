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
//  CPicLib.cpp: implementation of the CDawPic class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PicLib.h"
#include "Picture.h"


//////////////////////////////////////////////////////////////////////
CPicLib::CPicLib()
: m_hInstanceMod(NULL)	//#IR:100301 +
{
	m_pPicture = new CPicture();
}


CPicLib::~CPicLib()
{
	delete m_pPicture;
}

//(#IR:100301 +
void CPicLib::SetInstanceMod(HINSTANCE v) { 
	m_hInstanceMod = v; 
	if (NULL != m_pPicture) {
		m_pPicture->SetInstanceMod(v);
	}
}
//)

bool CPicLib::LoadFromRes( UINT uiID )
{
	return m_pPicture->LoadBinary(uiID);
}


bool CPicLib::LoadFromFile( const char* szPath )
{
	return m_pPicture->Load(szPath);
}


bool CPicLib::IsLoad()const
{
	return m_pPicture->IsLoad();
}


void CPicLib::GetSize( HDC hDC, UINT& rWidth, UINT& rHeight)
{
	CDC* pDC = CDC::FromHandle(hDC);
	CSize sz = m_pPicture->GetImageSize(pDC);
	rWidth = sz.cx;
	rHeight = sz.cy;
}


bool CPicLib::Draw( HDC hDC, RECT rc)
{
	CDC* pDC = CDC::FromHandle(hDC);
	CRect rect(rc);
	return m_pPicture->Draw( pDC, rect );
}
