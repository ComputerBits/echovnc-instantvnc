/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2004 Martin Scharpf, B. Braun Melsungen AG. All Rights Reserved.
//  Copyright (C) 2002 Ultr@VNC Team Members. All Rights Reserved.
//
//  This program is free software; you can redistribute it and/or modify
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
// If the source code for the program is not available from the place from
// which you received this file, check 
// http://ultravnc.sourceforge.net/

/* versioninfo.h: Defines 
	STR_COMPANYNAME, 
    STR_FILEVERSION,
	STR_PRODUCTVERSION,
	STR_COPYRIGHT,
	STR_SPECIALBUILD,
	INT_FILEVERSION,
	INT_PRODUCTVERSION */

// some helper macros (see http://http://gcc.gnu.org/onlinedocs/cpp/Stringification.html)
#define XSTR(x) STR(x)
#define STR(x) #x

// Concatenate tokens
#define CAT2(A,B) A.B
#define CAT4(A,B,C,D) A.B.C.D

// Define Version here
#define MAJ_VER 1
#define MIN_VER 0
#define SUB_VER 0
#define RC_VER 20
#define RC_SUB 5 // Used as ReleaseCandidate minor version in SpecialBuildDescription
// Set to 1 if ReleaseCandidate (toggles visibility of SpecialBuildDescription)
#define isRC 1

#define INT_VERSION MAJ_VER,MIN_VER,SUB_VER,RC_VER
#define STR_VERSION CAT4(MAJ_VER,MIN_VER,SUB_VER,RC_VER)
#define STR_RC_VERSION CAT2(RC_VER,RC_SUB)

#define INT_FILEVERSION INT_VERSION
#define INT_PRODUCTVERSION INT_VERSION

#define STR_FILEVERSION XSTR(STR_VERSION) "\0"
#define STR_PRODUCTVERSION XSTR(STR_VERSION) "\0"
#define STR_COPYRIGHT "Copyright © 2002-2005 UltraVNC team members\0"
#define STR_SPECIALBUILD "v1.0 ReleaseCandidate " XSTR(STR_RC_VERSION) "\0"
#define STR_COMPANYNAME "UltraVNC\0"
