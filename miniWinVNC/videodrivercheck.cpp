#include <windows.h>
#include <stdlib.h>
#include "vncOSVersion.h"

#define MAP1		1030

typedef BOOL (WINAPI* pEnumDisplayDevices)(PVOID,DWORD,PVOID,DWORD);

/*bool CheckDriver2(void)
{
	SC_HANDLE       schMaster;
	SC_HANDLE       schSlave;

	schMaster = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS); 

    schSlave = OpenService (schMaster, "vnccom", SERVICE_ALL_ACCESS);

    if (schSlave == NULL) 
    {
     CloseServiceHandle(schMaster);
	 return false;
    }
	else
	{
		CloseServiceHandle (schSlave);
		CloseServiceHandle(schMaster);
		return true;
	}
}
*/
BOOL
IsDriverActive()
{
	char driverName[] = "Mirage Driver";

	BOOL result = FALSE;
	HMODULE hUser32 = LoadLibrary("USER32");
	pEnumDisplayDevices pd = (pEnumDisplayDevices)GetProcAddress(hUser32, "EnumDisplayDevicesA");
    if (pd)
    {
        DISPLAY_DEVICE dd;
        ZeroMemory(&dd, sizeof(dd));
        dd.cb = sizeof(dd);
        ULONG devNum = 0;

        while (result = (*pd)(NULL, devNum, &dd, 0))
        {
			if (_strcmpi((const char *)dd.DeviceString, driverName) == 0)
				break;
			devNum++;
        }

		HDC	l_gdc = CreateDC((char*)dd.DeviceName, NULL, NULL, NULL);
		int err = GetLastError();
		BOOL m_bFlag1 = (l_gdc != NULL);

		int drvCr = ExtEscape(l_gdc, MAP1, 0, NULL, sizeof(pd), (char*)&pd);
		BOOL m_bFlag2 = (drvCr > 0);

		result = (m_bFlag1 && m_bFlag2);

		if (l_gdc) DeleteDC(l_gdc);
	}

	if (hUser32) FreeLibrary(hUser32);
	return result;

}

bool ExistDriver()
{
	//LPSTR driverName = "Winvnc video hook driver";
	LPSTR driverName = "Mirage Driver";
    DISPLAY_DEVICE dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);

	HMODULE hUser32 = LoadLibrary("USER32");
	pEnumDisplayDevices pd = (pEnumDisplayDevices)GetProcAddress( hUser32, "EnumDisplayDevicesA");
    INT devNum = 0;
	int result;
    while (result = (*pd)(NULL,devNum, &dd,0))
        {
          if (strcmp((const char *)&dd.DeviceString[0], driverName) == 0)
              break;
           devNum++;
        }

	if (hUser32) FreeLibrary(hUser32);
	return result != FALSE;
}
///////////////////////////////////////////////////////////////////
BOOL GetDllProductVersion(char* dllName, char *vBuffer, int size)
{
   char *versionInfo;
   void *lpBuffer;
   unsigned int cBytes;
   DWORD rBuffer;

   if( !dllName || !vBuffer )
      return(FALSE);

//   DWORD sName = GetModuleFileName(NULL, fileName, sizeof(fileName));
// FYI only
   DWORD sVersion = GetFileVersionInfoSize(dllName, &rBuffer);
   if (sVersion==0) return (FALSE);
   versionInfo = new char[sVersion];

   BOOL resultVersion = GetFileVersionInfo(dllName,
                                           NULL,
                                           sVersion,
                                           versionInfo);

   BOOL resultValue = VerQueryValue(versionInfo,  

TEXT("\\StringFileInfo\\040904b0\\ProductVersion"), 
                                    &lpBuffer, 
                                    &cBytes);

   if( !resultValue )
   {
	   resultValue = VerQueryValue(versionInfo,TEXT("\\StringFileInfo\\000004b0\\ProductVersion"), 
                                    &lpBuffer, 
                                    &cBytes);

   }

   if( resultValue )
   {
      strncpy_s(vBuffer, size, (char *) lpBuffer, size);
      delete versionInfo;
      return(TRUE);
   }
   else
   {
      *vBuffer = '\0';
      delete versionInfo;
      return(FALSE);
   }
}

///////////////////////////////////////////////////////////////////

bool
CheckVideoDriver(bool Box)
{
	if(OSVersion()!=1) return false;
	char DriverFileName1[_MAX_PATH];
	char DriverFileName2[_MAX_PATH];
	//char DriverFileName3[_MAX_PATH];
	char text [_MAX_PATH*3];
	char buffer1[256];
	char buffer2[256];
	//char buffer3[256];
	//bool b1,b2,b3,b4,active,active2;
	bool b4, active;
	BOOL mycheck1,mycheck2;
	
	//b1=false;
	//b2=false;
	//b3=false;
	b4=false;
	active=false;
	//active2=false;

	GetSystemDirectory(DriverFileName1, _MAX_PATH);
	GetSystemDirectory(DriverFileName2, _MAX_PATH);
	//GetSystemDirectory(DriverFileName3, _MAX_PATH);
	strcat_s(DriverFileName1,_MAX_PATH,"\\dfmirage.dll");
	strcat_s(DriverFileName2,_MAX_PATH,"\\Drivers\\dfmirage.sys");
	//strcat(DriverFileName3,"\\Drivers\\vncdrv.SYS");

	mycheck1=GetDllProductVersion(DriverFileName1,buffer1,254);
	mycheck2=GetDllProductVersion(DriverFileName2,buffer2,254);
	//GetDllProductVersion(DriverFileName3,buffer3,254);
	//if (strcmp(buffer1,"1.00.18")==0) b1=true;
	//if (strcmp(buffer1,"1.00.19")==0) b1=true;
	//if (strcmp(buffer2,"1.0.0.17")==0) b2=true;
//	if (strcmp(buffer3,"1.00.17")==NULL) b3=true;

	if (ExistDriver()) b4=true;
	//if (/*b1 && b2*/ /*&& b3*/ && b4)
	if (b4)
	{
		if (IsDriverActive()) active=true;
		//if (CheckDriver2()) active2=true;
	}

	if (Box)
	{
		if (mycheck1)
		{
			strcpy_s(text,_MAX_PATH*3,DriverFileName1);
			strcat_s(text,_MAX_PATH*3,"     Version: ");
			strcat_s(text,_MAX_PATH*3,buffer1);
			strcat_s(text,_MAX_PATH*3, ". ");
			strcat_s(text,_MAX_PATH*3,"\n");
		}
		else
		{
			//strcpy_s(text,_MAX_PATH*3,DriverFileName1);
			//strcat_s(text,_MAX_PATH*3,"  No found");
			strcpy_s(text,_MAX_PATH*3, "dfmirage.dll NOT found. ");
			strcat_s(text,_MAX_PATH*3,"\n");
		}

		if (mycheck2)
		{
			strcat_s(text,_MAX_PATH*3,DriverFileName2);
			strcat_s(text,_MAX_PATH*3,"     Version: ");
			strcat_s(text,_MAX_PATH*3,buffer2);
			strcat_s(text,_MAX_PATH*3, ". ");
			strcat_s(text,_MAX_PATH*3,"\n");
		}
		else
		{
			//strcat_s(text,_MAX_PATH*3,DriverFileName2);
			//strcat_s(text,_MAX_PATH*3,"  No found");
			strcat_s(text,_MAX_PATH*3,"dfmirage.sys NOT found. ");
			strcat_s(text,_MAX_PATH*3,"\n");
		}
//		strcat_s(text,_MAX_PATH*3,DriverFileName3);
//		strcat_s(text,_MAX_PATH*3," Version:  ");
//		strcat_s(text,_MAX_PATH*3,buffer3);
//		strcat_s(text,_MAX_PATH*3,"\n");
		strcat_s(text,_MAX_PATH*3,"\n");

		if (b4) 
		{
			strcat_s(text,_MAX_PATH*3,"The driver is present. ");
			strcat_s(text,_MAX_PATH*3,"\n");
		}
		else
		{
			strcat_s(text,_MAX_PATH*3,"The driver is NOT present. ");
			strcat_s(text,_MAX_PATH*3,"\n");
		}

		/*if (active2) 
		{
			strcat(text,"The communication service is present ");
			strcat(text,"\n");
		}
		else
		{
			strcat(text,"The communication service is NOT present ");
			strcat(text,"\n");
		}
		*/
		if (active) 
		{
			strcat_s(text,_MAX_PATH*3,"The driver is currently ACTIVE. ");
			strcat_s(text,_MAX_PATH*3,"\n");
		}
		else
		{
			strcat_s(text,_MAX_PATH*3,"The driver is NOT active. ");
			DEVMODE devmode;
			FillMemory(&devmode, sizeof(DEVMODE), 0);
			devmode.dmSize = sizeof(DEVMODE);
			devmode.dmDriverExtra = 0;
			BOOL change = EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&devmode);
			if (devmode.dmBitsPerPel==8 || devmode.dmBitsPerPel==16 || devmode.dmBitsPerPel==32) 
			{
				strcat_s(text,_MAX_PATH*3,"\n");
			}
			else
			{
				strcat_s(text,_MAX_PATH*3,"Color depth NOT supported, have to be 8/16/32");
				strcat_s(text,_MAX_PATH*3,"\n");
			}
		}

		MessageBox(NULL,text,"Driver Test",0);
	}

	//if (b1 && b2 /*&& b3*/ && b4) return true;
	if (b4) return true;
	else return false;
}
