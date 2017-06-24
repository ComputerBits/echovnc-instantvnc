// Logging.h: interface for the CLogging class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGGING_H__2523464D_935C_4B3C_8816_AC7530296A0A__INCLUDED_)
#define AFX_LOGGING_H__2523464D_935C_4B3C_8816_AC7530296A0A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef void (*LOGFAILEDEventFn)(char *machine);
typedef void (*LOGLOGONEventFn)(char *machine);
typedef void (*LOGEXITEventFn)(char *machine);
typedef void (*LOGFAILEDUSEREventFn)(char *machine, char *user);
typedef void (*LOGLOGONUSEREventFn)(char *machine, char *user);

class CLogging  
{
public:
	CLogging();
	virtual ~CLogging();

	void LOGFAILED(char *machine);
	void LOGLOGON(char *machine);
	void LOGEXIT(char *machine);
	void LOGFAILEDUSER(char *machine, char *user);
	void LOGLOGONUSER(char *machine, char *user);

private:
	HMODULE m_hLogging;

	LOGFAILEDEventFn m_fnLOGFAILED;
	LOGLOGONEventFn m_fnLOGLOGON;
	LOGEXITEventFn m_fnLOGEXIT;
	LOGFAILEDUSEREventFn m_fnLOGFAILEDUSER;
	LOGLOGONUSEREventFn m_fnLOGLOGONUSER;
};

#endif // !defined(AFX_LOGGING_H__2523464D_935C_4B3C_8816_AC7530296A0A__INCLUDED_)
