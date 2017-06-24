//  Copyright (C) 2005-2006 Lev Kazarkin. All Rights Reserved.
//
//  TightVNC is free software; you can redistribute it and/or modify
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
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/

#ifndef _WINVNC_VIDEODRIVER
#define _WINVNC_VIDEODRIVER

#include "stdhdrs.h"
#include "vncRegion.h"
#include <windows.h> 
#include <winioctl.h>


#define ESC_QVI		1026

#define MAP1		1030
#define UNMAP1		1031
#define TESTMAPPED	1051

#define MAXCHANGES_BUF 20000


//-----------------------------------------------
#define	IGNORE					 0
#define	FROM_SCREEN				 1
#define	FROM_DIB				 2
#define	TO_SCREEN				 3

#define	SCREEN_SCREEN			 11
#define	BLIT					 12
#define	SOLIDFILL				 13
#define	BLEND					 14
#define	TRANS					 15
#define	PLG						 17
#define	TEXTOUT					 18

//-------------------------------------------------

typedef enum
{
	dmf_dfo_Ptr_Engage	= 48,	// point is used with this record
	dmf_dfo_Ptr_Avert	= 49,
	
	// 1.0.9.0
	// mode-assert notifications to manifest PDEV limbo status
	dmf_dfn_assert_on	= 64,	// DrvAssert(TRUE): PDEV reenabled
	dmf_dfn_assert_off	= 65,	// DrvAssert(FALSE): PDEV disabled

} dmf_UpdEvent;


#define CDS_UPDATEREGISTRY  0x00000001
#define CDS_TEST            0x00000002
#define CDS_FULLSCREEN      0x00000004
#define CDS_GLOBAL          0x00000008
#define CDS_SET_PRIMARY     0x00000010
#define CDS_RESET           0x40000000
#define CDS_SETRECT         0x20000000
#define CDS_NORESET         0x10000000


#define SIOCTL_TYPE 40000
#define IOCTL_SIOCTL_METHOD_IN_DIRECT \
    CTL_CODE( SIOCTL_TYPE, 0x900, METHOD_IN_DIRECT, FILE_ANY_ACCESS  )
#define IOCTL_SIOCTL_METHOD_OUT_DIRECT \
    CTL_CODE( SIOCTL_TYPE, 0x901, METHOD_OUT_DIRECT , FILE_ANY_ACCESS  )
#define IOCTL_SIOCTL_METHOD_BUFFERED \
    CTL_CODE( SIOCTL_TYPE, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS  )
#define IOCTL_SIOCTL_METHOD_NEITHER \
    CTL_CODE( SIOCTL_TYPE, 0x903, METHOD_NEITHER , FILE_ANY_ACCESS  )

typedef BOOL (WINAPI* pEnumDisplayDevices)(PVOID,DWORD,PVOID,DWORD);
typedef LONG (WINAPI* pChangeDisplaySettingsEx)(LPCTSTR, LPDEVMODE, HWND, DWORD, LPVOID);

//*********************************************************************

typedef struct _CHANGES_RECORD
{
	ULONG type;  //screen_to_screen, blit, newcache,oldcache
	RECT rect;
	RECT origrect;
	POINT point;
	ULONG color; //number used in cache array
	ULONG refcolor; //slot used to pase btimap data
}CHANGES_RECORD;

typedef CHANGES_RECORD *PCHANGES_RECORD;

typedef struct _CHANGES_BUF
{
	 ULONG counter;
	 CHANGES_RECORD pointrect[MAXCHANGES_BUF];
} CHANGES_BUF;

typedef CHANGES_BUF *PCHANGES_BUF;

typedef struct _GETCHANGESBUF
{
	 PCHANGES_BUF buffer;
	 PVOID Userbuffer;
} GETCHANGESBUF;

typedef GETCHANGESBUF *PGETCHANGESBUF;

#define	DMF_VERSION_DEFINE(_ver_0,_ver_1,_ver_2,_ver_3)	((_ver_0<<24) | (_ver_1<<16) | (_ver_2<<8) | _ver_3)

#define	DMF_PROTO_VER_CURRENT	DMF_VERSION_DEFINE(1,2,0,0)
#define	DMF_PROTO_VER_MINCOMPAT	DMF_VERSION_DEFINE(0,9,0,1)

struct	Esc_dmf_Qvi_IN
{
	ULONG	cbSize;

	ULONG	app_actual_version;
	ULONG	display_minreq_version;

	ULONG	connect_options;		// reserved. must be 0.
};

enum
{
	esc_qvi_prod_name_max	= 16,
};

#define	ESC_QVI_PROD_MIRAGE	"MIRAGE"

struct	Esc_dmf_Qvi_OUT
{
	ULONG	cbSize;

	ULONG	display_actual_version;
	ULONG	miniport_actual_version;
	ULONG	app_minreq_version;
	ULONG	display_buildno;
	ULONG	miniport_buildno;

	char	prod_name[esc_qvi_prod_name_max];
};

class vncDesktop;

class vncVideoDriver
{

// Fields
public:

// Methods
public:
	vncVideoDriver();
	~vncVideoDriver();

	BOOL Activate(BOOL fForDirectAccess,int x,int y, int w,int h);
	void DeActivate();

	BOOL Activate_NT50(BOOL fForDirectAccess,int x,int y, int w,int h);
	void Deactivate_NT50();
	BOOL Activate_NT46(BOOL fForDirectAccess);
	void Deactivate_NT46();

	void ResetCounter() { oldCounter = bufdata.buffer->counter; }
	BYTE *GetScreenView(void) {	return (BYTE*)bufdata.Userbuffer; }

	BOOL TestMapped();

	BOOL MapSharedbuffers(BOOL fForDirectScreenAccess);
	void UnMapSharedbuffers();

	static BOOL GetDllProductVersion(char* dllName, char *vBuffer, int size);

	BOOL IsActive(void) { return m_fIsActive; }
	BOOL IsDirectAccessInEffect(void) {	return m_fDirectAccessInEffect; }
	BOOL IsHandlingScreen2ScreenBlt(void) { return m_fHandleScreen2ScreenBlt; }
	static BOOL IsMirageDriverActive();

	static bool ExistDriver();
	static bool CheckVideoDriver(bool Box);

	BOOL HardwareCursor();
	BOOL NoHardwareCursor();

	ULONG oldCounter;

	BOOL Tempres();
	BOOL Temp_Resolution;
	BOOL blocked;

	GETCHANGESBUF bufdata;

protected:

	static BOOL	LookupVideoDeviceAlt(LPCTSTR szDevStr, LPCTSTR szDevStrAlt,	INT &devNum, DISPLAY_DEVICE *pDd);
	static HKEY	CreateDeviceKey(LPCTSTR szMpName);

	char	m_devname[32];
	ULONG	m_drv_ver_mj;
	ULONG	m_drv_ver_mn;

	bool m_fIsActive;
	bool m_fDirectAccessInEffect;
	bool m_fHandleScreen2ScreenBlt;

	static char	vncVideoDriver::szDriverString[];
	static char	vncVideoDriver::szDriverStringAlt[];
	static char	vncVideoDriver::szMiniportName[];
};

#endif