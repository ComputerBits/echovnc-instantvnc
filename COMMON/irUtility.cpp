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
///////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation of various common utilities
//
// @history:
//	Created/Added by Igor Rumyantsev, 2010-03-03 (100303)
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "irUtility.h"

namespace Utility
{

void CenterWindow(HWND hWnd, bool bRedraw, int iXOffset, int iYOffset)
//Default:                 , false       , 0           , 0
{
	if (NULL == hWnd) {
		return;
	}
	int x0 = ::GetSystemMetrics(SM_CXSCREEN) >> 1;
	int y0 = ::GetSystemMetrics(SM_CYSCREEN) >> 1;
	RECT rect;
	::GetWindowRect(hWnd, &rect);
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	rect.left = x0 - (w>>1) + iXOffset;
	rect.right = rect.left + w;
	rect.top = y0 - (h>>1) + iYOffset;
	rect.bottom = rect.top + h;
	::MoveWindow(hWnd, rect.left, rect.top, w, h, bRedraw); //TRUE; // TRUE means 'redraw'
}

}