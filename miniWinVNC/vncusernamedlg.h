//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncUserNameDlg

// Object implementing the enter user name prompt dialog for InstantVNC.

class vncUserNameDlg;

#if (!defined(_WINVNC_VNCUSERNAMEDLG))
#define _WINVNC_VNCUSERNAMEDLG

// Includes
#include "stdhdrs.h"

// The vncUserNameDlg class itself
class vncUserNameDlg
{
public:
	// Constructor/destructor
	vncUserNameDlg();
	~vncUserNameDlg();

	// General
	int Show(BOOL show);

	int GetEnteredData(char* pName, int len);

	// set the dialog type
	// nType = 0 for address
	// nType = 1 for username
	// nType = 2 for password
	void setType(int nType);

protected:
	// Initialisation
	BOOL Init(HWND hDlg);

	// The dialog box window proc
	static BOOL CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool CheckData( HWND hDlg );
	int m_nType; // 0 - address
				//  1 - username
				//  2 - password
	char m_szUsername[71];
};

#endif // _WINVNC_VNCUSERNAMEDLG
