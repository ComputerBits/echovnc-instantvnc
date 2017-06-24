///////////////////////////////////////////////////////////////////////////////////////////////////
// irUtility.h: declarations of various common utilities
//
// @history:
//	Created/Added by Igor Rumyantsev, 2010-03-03 (100303)
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(irUtility_H__INCLUDED_)
#define irUtility_H__INCLUDED_

#pragma once


#include <windows.h>

namespace Utility
{

void CenterWindow(HWND hWnd, bool bRedraw=false, int iXOffset=0, int iYOffset=0);

};


#endif // !defined(irUtility_H__INCLUDED_)
