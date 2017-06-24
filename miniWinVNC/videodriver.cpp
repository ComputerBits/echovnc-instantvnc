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
// TightVNC distribution homepage on the Web: http://www.tightvnc.com
//------------------------------------------------------------------------
#include "VideoDriver.h"
#include "vncDesktop.h"
#include "vncBuffer.h"  
#include "vncService.h"
#include "vncOSVersion.h"

char	vncVideoDriver::szDriverString[] = "Mirage Driver";
char	vncVideoDriver::szDriverStringAlt[] = "DemoForge Mirage Driver";
char	vncVideoDriver::szMiniportName[] = "dfmirage";

#define CURSOREN		1060
#define CURSORDIS		1061
#define MINIPORT_REGISTRY_PATH	"SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current\\System\\CurrentControlSet\\Services"
#define DM_POSITION         0x00000020L

//BOOL IsWinNT();
BOOL IsNtVer(ULONG mj, ULONG mn);
BOOL IsWinVerOrHigher(ULONG mj, ULONG mn);

#define	BYTE0(x)	((x) & 0xFF)
#define	BYTE1(x)	(((x) >> 8) & 0xFF)
#define	BYTE2(x)	(((x) >> 16) & 0xFF)
#define	BYTE3(x)	(((x) >> 24) & 0xFF)

vncVideoDriver::vncVideoDriver()
{
	bufdata.buffer = NULL;
	bufdata.Userbuffer = NULL;

	m_fIsActive = false;
	m_fDirectAccessInEffect = false;
	m_fHandleScreen2ScreenBlt = false;
	*m_devname = 0;
	m_drv_ver_mj = 0;
	m_drv_ver_mn = 0;
	oldCounter = 0;
	// no video driver
	//Temp_Resolution=false;

	blocked = FALSE;
}

vncVideoDriver::~vncVideoDriver()
{
}

template <class TpFn>
HINSTANCE  LoadNImport(LPCTSTR szDllName, LPCTSTR szFName, TpFn &pfn)
{
	HINSTANCE hDll = LoadLibrary(szDllName);
	if (hDll)
	{
		pfn = (TpFn)GetProcAddress(hDll, szFName);
		if (pfn) return hDll;
		FreeLibrary(hDll);
	}
	vnclog.Print(LL_INTERR,	VNCLOG("Can not import '%s' from '%s'."),	szFName, szDllName);
	return NULL;
}

BOOL vncVideoDriver::Activate(BOOL fForDirectAccess,int x,int y, int w,int h)
{ 
	_ASSERTE(vncService::IsWinNT());

	if (IsWinVerOrHigher(5, 0))
	{		
		return Activate_NT50(fForDirectAccess, x, y, w, h);
	}
	else
	{
		return Activate_NT46(fForDirectAccess);
	}
}

void vncVideoDriver::DeActivate()
{
	_ASSERTE(vncService::IsWinNT());

	if (IsWinVerOrHigher(5, 0))
	{
		Deactivate_NT50();
	}
	else
	{
		Deactivate_NT46();
	}
}

BOOL vncVideoDriver::Activate_NT50(BOOL fForDirectAccess, int x, int y, int w, int h)
{
	HDESK   hdeskInput;
    HDESK   hdeskCurrent;
 
	DISPLAY_DEVICE dd;
	INT devNum = 0;
	if (!LookupVideoDeviceAlt(szDriverString, szDriverStringAlt, devNum, &dd))
	{
		vnclog.Print(LL_INTERR, VNCLOG("No '%s' or '%s' found."), szDriverString, szDriverStringAlt);
		return FALSE;
	}

	DEVMODE devmode;
	FillMemory(&devmode, sizeof(DEVMODE), 0);
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	BOOL change = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (x && y && w && h)
	{
// we always have to set position or
// a stale position info in registry would come into effect.
		devmode.dmFields |= DM_POSITION;
		devmode.dmPosition.x = x;
		devmode.dmPosition.y = y;

		devmode.dmPelsWidth = w;
		devmode.dmPelsHeight = h;
	}

	devmode.dmDeviceName[0] = '\0';

    vnclog.Print(LL_INTINFO, VNCLOG("DevNum:%d\nName:%s\nString:%s"), devNum, &dd.DeviceName[0], &dd.DeviceString[0]);
	vnclog.Print(LL_INTINFO, VNCLOG("Screen Top-Left Position: (%i, %i)"), devmode.dmPosition.x, devmode.dmPosition.y);
	vnclog.Print(LL_INTINFO, VNCLOG("Screen Dimensions: (%i, %i)"), devmode.dmPelsWidth, devmode.dmPelsHeight);
	vnclog.Print(LL_INTINFO, VNCLOG("Screen Color depth: %i"), devmode.dmBitsPerPel);

	HKEY hKeyDevice = CreateDeviceKey(szMiniportName);
	if (hKeyDevice == NULL)
		return FALSE;

// TightVNC does not use these features
	RegDeleteValue(hKeyDevice, ("Screen.ForcedBpp"));
	RegDeleteValue(hKeyDevice, ("Pointer.Enabled"));

	DWORD dwVal = fForDirectAccess ? 3 : 0;
// NOTE that old driver ignores it and mapping is always ON with it
	if (RegSetValueEx(hKeyDevice, ("Cap.DfbBackingMode"), 0, REG_DWORD,	(unsigned char *)&dwVal, 4) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't set \"Cap.DfbBackingMode\" to %d"), dwVal);
		return FALSE;
	}

	dwVal = 1;
	if (RegSetValueEx(hKeyDevice, ("Order.BltCopyBits.Enabled"), 0,	REG_DWORD, (unsigned char *)&dwVal,	4) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't set Order.BltCopyBits.Enabled to %d"), dwVal);
		return FALSE;
	}

	dwVal = 1;
	if (RegSetValueEx(hKeyDevice, ("Attach.ToDesktop"),	0, REG_DWORD, (unsigned char *)&dwVal, 4) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't set Attach.ToDesktop to %d"), dwVal);
		return FALSE;
	}

	pChangeDisplaySettingsEx pCDS = NULL;
	HINSTANCE  hInstUser32 = LoadNImport("User32.DLL", "ChangeDisplaySettingsExA", pCDS);
	if (!hInstUser32) return FALSE;

	// Save the current desktop
	hdeskCurrent = GetThreadDesktop(GetCurrentThreadId());
	if (hdeskCurrent != NULL)
	{
		hdeskInput = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
		if (hdeskInput != NULL) 
			SetThreadDesktop(hdeskInput);
	}
// 24 bpp screen mode is MUNGED to 32 bpp.
// the underlying buffer format must be 32 bpp.
// see vncDesktop::ThunkBitmapInfo()
	if (devmode.dmBitsPerPel==24) devmode.dmBitsPerPel = 32;
	LONG cr = (*pCDS)((TCHAR *)dd.DeviceName, &devmode,	NULL, CDS_UPDATEREGISTRY,NULL);
	if (cr != DISP_CHANGE_SUCCESSFUL)
	{
		vnclog.Print(LL_INTERR,
			VNCLOG("ChangeDisplaySettingsEx failed on device \"%s\" with status: 0x%x"),
			dd.DeviceName,
			cr);
	}

	strcpy_s(m_devname, 32, (const char *)dd.DeviceName);

	// Reset desktop
	SetThreadDesktop(hdeskCurrent);
	// Close the input desktop
	CloseDesktop(hdeskInput);
	RegCloseKey(hKeyDevice);
	FreeLibrary(hInstUser32);

	return TRUE;
}

BOOL vncVideoDriver::Activate_NT46(BOOL fForDirectAccess)
{
	HKEY hKeyDevice = CreateDeviceKey(szMiniportName);
	if (hKeyDevice == NULL)
		return FALSE;

	// TightVNC does not use these features
	RegDeleteValue(hKeyDevice, ("Screen.ForcedBpp"));
	RegDeleteValue(hKeyDevice, ("Pointer.Enabled"));

	DWORD dwVal = fForDirectAccess ? 3 : 0;
	// NOTE that old driver ignores it and mapping is always ON with it
	if (RegSetValueEx(hKeyDevice, ("Cap.DfbBackingMode"), 0, REG_DWORD,	(unsigned char *)&dwVal, 4) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't set \"Cap.DfbBackingMode\" to %d"), dwVal);
		return FALSE;
	}

	dwVal = 1;
	if (RegSetValueEx(hKeyDevice, ("Order.BltCopyBits.Enabled"), 0, REG_DWORD, (unsigned char *)&dwVal,	4) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't set Order.BltCopyBits.Enabled to %d"), dwVal);
		return FALSE;
	}

// NOTE: we cannot truly load the driver
// but ChangeDisplaySettings makes PDEV to reload
// and thus new settings come into effect

// TODO

	strcpy_s(m_devname, 32, "DISPLAY");

	RegCloseKey(hKeyDevice);
	return TRUE;
}

void vncVideoDriver::Deactivate_NT50()
{
	HDESK   hdeskInput;
	HDESK   hdeskCurrent;
 
	*m_devname = 0;

	DISPLAY_DEVICE dd;
	INT devNum = 0;
	if (!LookupVideoDeviceAlt(szDriverString, szDriverStringAlt, devNum, &dd))
	{
		vnclog.Print(LL_INTERR, VNCLOG("No '%s' or '%s' found."), szDriverString, szDriverStringAlt);
		return;
	}

	DEVMODE devmode;
	FillMemory(&devmode, sizeof(DEVMODE), 0);
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	BOOL change = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	devmode.dmDeviceName[0] = '\0';

	HKEY hKeyDevice = CreateDeviceKey(szMiniportName);
	if (hKeyDevice == NULL)
		return;

    DWORD one = 0;
	if (RegSetValueEx(hKeyDevice,("Attach.ToDesktop"), 0, REG_DWORD, (unsigned char *)&one,4) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't set Attach.ToDesktop to 0x1"));
	}

// reverting to default behavior
	RegDeleteValue(hKeyDevice, ("Cap.DfbBackingMode"));
	RegDeleteValue(hKeyDevice, ("Order.BltCopyBits.Enabled"));

	pChangeDisplaySettingsEx pCDS = NULL;
	HINSTANCE  hInstUser32 = LoadNImport("User32.DLL", "ChangeDisplaySettingsExA", pCDS);
	if (!hInstUser32) return;

	// Save the current desktop
	hdeskCurrent = GetThreadDesktop(GetCurrentThreadId());
	if (hdeskCurrent != NULL)
	{
		hdeskInput = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
		if (hdeskInput != NULL)
			SetThreadDesktop(hdeskInput);
	}
// 24 bpp screen mode is MUNGED to 32 bpp. see vncDesktop::ThunkBitmapInfo()
	if (devmode.dmBitsPerPel==24) devmode.dmBitsPerPel = 32;

	// Add 'Default.*' settings to the registry under above hKeyProfile\mirage\device
	(*pCDS)((TCHAR *)dd.DeviceName, &devmode, NULL, CDS_UPDATEREGISTRY, NULL);

	// Reset desktop
	SetThreadDesktop(hdeskCurrent);
	// Close the input desktop
	CloseDesktop(hdeskInput);
	RegCloseKey(hKeyDevice);
	FreeLibrary(hInstUser32);
}

void vncVideoDriver::Deactivate_NT46()
{
	*m_devname = 0;

	HKEY hKeyDevice = CreateDeviceKey(szMiniportName);
	if (hKeyDevice == NULL)
		return;

// reverting to default behavior
	RegDeleteValue(hKeyDevice, ("Cap.DfbBackingMode"));

//	RegDeleteValue(hKeyDevice, ("Order.BltCopyBits.Enabled"));
// TODO: remove "Order.BltCopyBits.Enabled"
// now we don't touch this important option
// because we dont apply the changed values

	RegCloseKey(hKeyDevice);
}


BOOL vncVideoDriver::LookupVideoDeviceAlt(
		LPCTSTR szDevStr,
		LPCTSTR szDevStrAlt,
		INT &devNum,
		DISPLAY_DEVICE *pDd)
{
	_ASSERTE(IsWinVerOrHigher(5, 0));

	pEnumDisplayDevices pd = NULL;
	HINSTANCE  hInstUser32 = LoadNImport("User32.DLL", "EnumDisplayDevicesA", pd);
	if (!hInstUser32) return FALSE;

	ZeroMemory(pDd, sizeof(DISPLAY_DEVICE));
	pDd->cb = sizeof(DISPLAY_DEVICE);
	BOOL result;
	while (result = (*pd)(NULL,devNum, pDd, 0))
	{
		if (strcmp((const char *)pDd->DeviceString, szDevStr) == 0 ||
			szDevStrAlt && strcmp((const char *)pDd->DeviceString, szDevStrAlt) == 0)
			break;
		devNum++;
	}

	FreeLibrary(hInstUser32);
	return result;
}

HKEY vncVideoDriver::CreateDeviceKey(LPCTSTR szMpName)
{
	HKEY hKeyProfileMirage = (HKEY)0;
	if (RegCreateKey(HKEY_LOCAL_MACHINE, (MINIPORT_REGISTRY_PATH), &hKeyProfileMirage) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't access registry."));
		return FALSE;
	}
	HKEY hKeyProfileMp = (HKEY)0;
	LONG cr = RegCreateKey(hKeyProfileMirage, szMpName, &hKeyProfileMp);
	RegCloseKey(hKeyProfileMirage);
	if (cr != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR,	VNCLOG("Can't access \"%s\" hardware profiles key."),	szMpName);
		return FALSE;
	}
	HKEY hKeyDevice = (HKEY)0;
	if (RegCreateKey(hKeyProfileMp,	("DEVICE0"), &hKeyDevice) != ERROR_SUCCESS)
	{
		vnclog.Print(LL_INTERR, VNCLOG("Can't access DEVICE0 hardware profiles key."));
	}
	RegCloseKey(hKeyProfileMp);
	return hKeyDevice;
}

BOOL vncVideoDriver::IsMirageDriverActive()
{
	BOOL result = FALSE;
	DISPLAY_DEVICE dd;
	INT devNum = 0;
	if (LookupVideoDeviceAlt(szDriverString, szDriverStringAlt, devNum, &dd))
		result = (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP);
	return result;
}

BOOL vncVideoDriver:: GetDllProductVersion(char* dllName, char *vBuffer, int size)
{
   char *versionInfo;
   char fileName[_MAX_PATH + 1];
   void *lpBuffer;
   unsigned int cBytes;
   DWORD rBuffer;

   if (!dllName || !vBuffer)
      return FALSE;

   DWORD sName = GetModuleFileName(NULL, fileName, sizeof(fileName));
   DWORD sVersion = GetFileVersionInfoSize(dllName, &rBuffer);
   if (sVersion==0) return FALSE;
   versionInfo = new char[sVersion];

   BOOL resultVersion = GetFileVersionInfo(dllName, NULL, sVersion, versionInfo);

   BOOL resultValue = VerQueryValue(versionInfo, TEXT("\\StringFileInfo\\040904b0\\ProductVersion"), &lpBuffer, &cBytes);
   if (!resultValue)
	   resultValue = VerQueryValue(versionInfo,TEXT("\\StringFileInfo\\000004b0\\ProductVersion"), &lpBuffer, &cBytes);

   if (resultValue)
   {
      strncpy_s(vBuffer, size, (char *)lpBuffer, size);
      delete versionInfo;
      return TRUE;
   }
   else
   {
      *vBuffer = '\0';
      delete versionInfo;
      return FALSE;
   }
}

BOOL vncVideoDriver::MapSharedbuffers(BOOL fForDirectScreenAccess)
{
	_ASSERTE(!m_fIsActive);
	_ASSERTE(!m_fDirectAccessInEffect);
	_ASSERTE(vncService::IsWinNT());

	HDC m_gdc = ::CreateDC(m_devname, NULL, NULL, NULL);
	if (!m_gdc)
	{
		vnclog.Print(LL_INTERR,
			VNCLOG("vncVideoDriver::MapSharedbuffers: can't create DC on \"%s\""),
			m_devname);
		return FALSE;
	}

	oldCounter = 0;
	int drvCr = ExtEscape(m_gdc, MAP1, 0, NULL,	sizeof(GETCHANGESBUF), (LPSTR) &bufdata);
	DeleteDC(m_gdc);

	if (drvCr <= 0)
	{
		vnclog.Print(LL_INTERR,
			VNCLOG("vncVideoDriver::MapSharedbuffers: MAP1 call returned 0x%x"),
			drvCr);
		return FALSE;
	}
	m_fIsActive = true;
	if (fForDirectScreenAccess)
	{
		if (!bufdata.Userbuffer)
		{
			vnclog.Print(LL_INTERR,
				VNCLOG("vncVideoDriver::MapSharedbuffers: mirror screen view is NULL"));
			return FALSE;
		}
		m_fDirectAccessInEffect = true;
	}
	else
	{
		if (bufdata.Userbuffer)
		{
			vnclog.Print(LL_INTINFO,
				VNCLOG("vncVideoDriver::MapSharedbuffers: mirror screen view is mapped but direct access mode is OFF"));
		}
	}

// Screen2Screen support added in Mirage ver 1.2
	m_fHandleScreen2ScreenBlt = (m_drv_ver_mj > 1) || (m_drv_ver_mj == 1 && m_drv_ver_mn >= 2);

	return TRUE;	
}
void vncVideoDriver::UnMapSharedbuffers()
{
	_ASSERTE(vncService::IsWinNT());

	int DrvCr = 0;
	if (m_devname[0])
	{
		_ASSERTE(m_fIsActive);
		_ASSERTE(bufdata.buffer);
		_ASSERTE(!m_fDirectAccessInEffect || bufdata.Userbuffer);

		HDC m_gdc= ::CreateDC(m_devname, NULL, NULL, NULL);
		if (!m_gdc)
		{
			vnclog.Print(LL_INTERR,
				VNCLOG("vncVideoDriver::UnMapSharedbuffers: can't create DC on \"%s\""),
				m_devname);
		}
		else
		{
			DrvCr = ExtEscape(m_gdc, UNMAP1, sizeof(GETCHANGESBUF), (LPSTR) &bufdata, 0, NULL);
			DeleteDC(m_gdc);

			_ASSERTE(DrvCr > 0);
		}
	}
// 0 return value is unlikely for Mirage because its DC is independent
// from the reference device;
// this happens with Quasar if its mode was changed externally.
// nothing is particularly bad with it.

	if (DrvCr <= 0)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("vncVideoDriver::UnMapSharedbuffers failed. unmapping manually"));
		if (bufdata.buffer)
		{
			BOOL br = UnmapViewOfFile(bufdata.buffer);
			vnclog.Print(LL_INTINFO,
				VNCLOG("vncVideoDriver::UnMapSharedbuffers: UnmapViewOfFile(bufdata.buffer) returned %d"),
				br);
		}
		if (bufdata.Userbuffer)
		{
			BOOL br = UnmapViewOfFile(bufdata.Userbuffer);
			vnclog.Print(LL_INTINFO,
				VNCLOG("vncVideoDriver::UnMapSharedbuffers: UnmapViewOfFile(bufdata.Userbuffer) returned %d"),
				br);
		}
	}
	m_fIsActive = false;
	m_fDirectAccessInEffect = false;
	m_fHandleScreen2ScreenBlt = false;
}

BOOL vncVideoDriver::TestMapped()
{
	_ASSERTE(vncService::IsWinNT());

	TCHAR *pDevName;
	if (IsWinVerOrHigher(5, 0))
	{
		DISPLAY_DEVICE dd;
		INT devNum = 0;
		if (!LookupVideoDeviceAlt(szDriverString, szDriverStringAlt, devNum, &dd))
			return FALSE;
		pDevName = (TCHAR *)dd.DeviceName;
	}
	else
	{
		pDevName = "DISPLAY";
	}

	HDC	l_ddc = ::CreateDC(pDevName, NULL, NULL, NULL);
	if (l_ddc)
	{
		BOOL b = ExtEscape(l_ddc, TESTMAPPED, 0, NULL, 0, NULL);	
		DeleteDC(l_ddc);
		return b;
	}
	return FALSE;
}

bool vncVideoDriver::ExistDriver()
{
	DISPLAY_DEVICE dd;
	INT devNum = 0;
	return LookupVideoDeviceAlt(szDriverString, szDriverStringAlt, devNum, &dd) != FALSE;
}

bool vncVideoDriver::CheckVideoDriver(bool Box)
{
	if (OSVersion() != 1) return false;
	char DriverFileName1[_MAX_PATH];
	char DriverFileName2[_MAX_PATH];

	char text[_MAX_PATH*3];
	char buffer1[256];
	char buffer2[256];

	GetSystemDirectory(DriverFileName1, _MAX_PATH);
	GetSystemDirectory(DriverFileName2, _MAX_PATH);

	strcat_s(DriverFileName1, _MAX_PATH, "\\dfmirage.dll");
	strcat_s(DriverFileName2, _MAX_PATH, "\\Drivers\\dfmirage.sys");

	BOOL mycheck1 = GetDllProductVersion(DriverFileName1, buffer1, 254);
	BOOL mycheck2 = GetDllProductVersion(DriverFileName2, buffer2, 254);

	bool exist = ExistDriver();

	vncVideoDriver videoDriver;
	bool active = exist && videoDriver.IsMirageDriverActive();

	if (Box)
	{
		if (mycheck1)
		{
			strcpy_s(text,_MAX_PATH*3,DriverFileName1);
			strcat_s(text,_MAX_PATH*3,"     Version: ");
			strcat_s(text,_MAX_PATH*3,buffer1);
			strcat_s(text,_MAX_PATH*3,".\n");
		}
		else
			strcpy_s(text,_MAX_PATH*3,"dfmirage.dll not found.\n");

		if (mycheck2)
		{
			strcat_s(text,_MAX_PATH*3,DriverFileName2);
			strcat_s(text,_MAX_PATH*3,"     Version: ");
			strcat_s(text,_MAX_PATH*3,buffer2);
			strcat_s(text,_MAX_PATH*3,".\n");
		}
		else
			strcat_s(text,_MAX_PATH*3,"dfmirage.sys not found.\n");

		strcat_s(text,_MAX_PATH*3,"\n");

		if (exist)
			strcat_s(text,_MAX_PATH*3,"The driver is present.\n");
		else
			strcat_s(text,_MAX_PATH*3,"The driver is not present.\n");

		if (active)
			strcat_s(text,_MAX_PATH*3,"The driver is currently active.\n");
		else
		{
			strcat_s(text,_MAX_PATH*3,"The driver is not active.\n");

			DEVMODE devmode;
			FillMemory(&devmode, sizeof(DEVMODE), 0);
			devmode.dmSize = sizeof(DEVMODE);
			devmode.dmDriverExtra = 0;
			EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS, &devmode);
			if (!(devmode.dmBitsPerPel==8 || devmode.dmBitsPerPel==16 || devmode.dmBitsPerPel==32))
			{
				strcat_s(text,_MAX_PATH*3,"Color depth not supported, have to be 8/16/32.\n");
			}
		}

		MessageBox(NULL, text, "Driver Test", 0);
	}

	return exist;
}

BOOL vncVideoDriver::HardwareCursor()
{
	HDC gdc = GetDC(NULL);
	int returnvalue = ExtEscape(gdc, CURSOREN, 0, NULL, NULL, NULL);
	ReleaseDC(NULL, gdc);
	return returnvalue > 0;
}

BOOL vncVideoDriver::NoHardwareCursor()
{
	HDC gdc = GetDC(NULL);
	int returnvalue = ExtEscape(gdc, CURSORDIS, 0, NULL, NULL, NULL);
	ReleaseDC(NULL, gdc);
	return returnvalue > 0;
}

BOOL vncVideoDriver::Tempres()
{
	HMODULE hUser32 = LoadLibrary("USER32");
	pEnumDisplayDevices pd = (pEnumDisplayDevices)GetProcAddress(hUser32, "EnumDisplayDevicesA");

	DWORD i;
	int curw, curh, curp;
	int regw, regh, regp;
	DISPLAY_DEVICE dd;
	DEVMODE dm;
	LPSTR deviceName = NULL;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	DEVMODE devmode;
	FillMemory(&devmode, sizeof(DEVMODE), 0);
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;

	for (i = 0; (*pd)(NULL, i, &dd, 0); i++)
	{
		if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
		{
			ZeroMemory(&dm, sizeof(dm));
			dm.dmSize = sizeof(dm);
			EnumDisplaySettings((char*)(dd.DeviceName), ENUM_CURRENT_SETTINGS, &dm);
			break;
		}
	}

	deviceName = (LPSTR)&dd.DeviceName[0];
	EnumDisplaySettings(deviceName, ENUM_CURRENT_SETTINGS, &devmode);
	curw = devmode.dmPelsWidth;
	curh = devmode.dmPelsHeight;
	curp = devmode.dmBitsPerPel;
	EnumDisplaySettings(deviceName, ENUM_REGISTRY_SETTINGS, &devmode);
	regw = devmode.dmPelsWidth;
	regh = devmode.dmPelsHeight;
	regp = devmode.dmBitsPerPel;

	return (curw == regw && curh == regh && curp == regp);
}
