// Logging.cpp: implementation of the CLogging class.
//
//////////////////////////////////////////////////////////////////////

#include "stdhdrs.h"
#include "LoggingDLL.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const TCHAR szLoggingDLL[] = "logging.dll";

CLogging logging;

CLogging::CLogging()
{
	m_hLogging = NULL;
	m_fnLOGFAILED = NULL;
	m_fnLOGLOGON = NULL;
	m_fnLOGEXIT = NULL;
	m_fnLOGFAILEDUSER = NULL;
	m_fnLOGLOGONUSER = NULL;

	char szCurrentDir[_MAX_PATH];
	memset(szCurrentDir, 0, _MAX_PATH);

	if (GetModuleFileName(NULL, szCurrentDir, _MAX_PATH))
	{
		char* p = strrchr(szCurrentDir, '\\');
		if (!p) p = strrchr(szCurrentDir, '/');
		*(++p) = 0;
	}
	strcat_s(szCurrentDir, _MAX_PATH, szLoggingDLL);

	m_hLogging = LoadLibrary(szCurrentDir);
	if (m_hLogging)
	{
		m_fnLOGFAILED = (LOGFAILEDEventFn)GetProcAddress(m_hLogging, "LOGFAILED");
		m_fnLOGLOGON = (LOGLOGONEventFn)GetProcAddress(m_hLogging, "LOGLOGON");
		m_fnLOGEXIT = (LOGEXITEventFn)GetProcAddress(m_hLogging, "LOGEXIT");
		m_fnLOGFAILEDUSER = (LOGFAILEDUSEREventFn)GetProcAddress(m_hLogging, "LOGFAILEDUSER");
		m_fnLOGLOGONUSER = (LOGLOGONUSEREventFn)GetProcAddress(m_hLogging, "LOGLOGONUSER");
	}
}

CLogging::~CLogging()
{
	m_fnLOGFAILED = NULL;
	m_fnLOGLOGON = NULL;
	m_fnLOGEXIT = NULL;
	m_fnLOGFAILEDUSER = NULL;
	m_fnLOGLOGONUSER = NULL;

	if (m_hLogging)
	{
		FreeLibrary(m_hLogging);
		m_hLogging = NULL;
	}
}

void CLogging::LOGFAILED(char *machine)
{
	if (m_fnLOGFAILED)
		m_fnLOGFAILED(machine);
}

void CLogging::LOGLOGON(char *machine)
{
	if (m_fnLOGLOGON)
		m_fnLOGLOGON(machine);
}

void CLogging::LOGEXIT(char *machine)
{
	if (m_fnLOGEXIT)
		m_fnLOGEXIT(machine);
}

void CLogging::LOGFAILEDUSER(char *machine, char *user)
{
	if (m_fnLOGFAILEDUSER)
		m_fnLOGFAILEDUSER(machine, user);
}

void CLogging::LOGLOGONUSER(char *machine, char *user)
{
	if (m_fnLOGLOGONUSER)
		m_fnLOGLOGONUSER(machine, user);
}
