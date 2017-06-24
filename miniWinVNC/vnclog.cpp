//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
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
// which you received this file, check http://www.realvnc.com/ or contact
// the authors on info@realvnc.com for information on obtaining it.

// Log.cpp: implementation of the VNCLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdhdrs.h"
#include <io.h>
#include "VNCLog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const int VNCLog::ToDebug   =  1;
const int VNCLog::ToFile    =  2;
const int VNCLog::ToConsole =  4;

const static int LINE_BUFFER_SIZE = 1024;

VNCLog::VNCLog()
{
	m_lastLogTime = 0;
	m_filename = NULL;
	m_append = false;
    hlogfile = NULL;
    m_todebug = false;
    m_toconsole = false;
    m_tofile = false;
}

void VNCLog::SetMode(int mode)
{
	m_mode = mode;
    if (mode & ToDebug)
        m_todebug = true;
    else
        m_todebug = false;

    if (mode & ToFile)  {
        m_tofile = true;
	} else {
		CloseFile();
        m_tofile = false;
    }

    if (mode & ToConsole) {
        if (!m_toconsole) {
            AllocConsole();
            fclose(stdout);
            fclose(stderr);
#ifdef _MSC_VER
            int fh = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), 0);
            _dup2(fh, 1);
            _dup2(fh, 2);
            _fdopen(1, "wt");
            _fdopen(2, "wt");
            printf("fh is %d\n",fh);
            fflush(stdout);
#endif
        }

        m_toconsole = true;

    } else {
        m_toconsole = false;
    }
}


void VNCLog::SetLevel(int level) {
    m_level = level;
}

void VNCLog::SetFile(const char* filename, bool append) 
{
	if (m_filename)
	{
		free(m_filename);
		m_filename = NULL;
	}

	m_filename = mystrdup(filename);
	m_append = append;
	if (!m_tofile || !m_filename)
		return;
    // if a log file is open, close it now.
    CloseFile();

    m_tofile  = true;

    // If filename is NULL or invalid we should throw an exception here

    hlogfile = CreateFile(
        filename, GENERIC_WRITE, FILE_SHARE_READ, NULL,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hlogfile == INVALID_HANDLE_VALUE) {
        // We should throw an exception here
        m_todebug = true;
        m_tofile = false;
        Print(0, ("Error opening log file %s\n"), filename);
    }
    if (append) {
        SetFilePointer( hlogfile, 0, NULL, FILE_END );
    } else {
        SetEndOfFile( hlogfile );
    }
}

// if a log file is open, close it now.
void VNCLog::CloseFile()
{
    if (hlogfile != NULL)
	{
        CloseHandle(hlogfile);
        hlogfile = NULL;
    }
}

void VNCLog::DeleteLog()
{
	DeleteFile(m_filename);
}

inline void VNCLog::ReallyPrintLine(const char* line) 
{
    if (m_todebug) OutputDebugString(line);
    if (m_toconsole) {
        DWORD byteswritten;
        WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), line, strlen(line), &byteswritten, NULL); 
    };
    if (m_tofile && (hlogfile != NULL)) {
        DWORD byteswritten;
        WriteFile(hlogfile, line, strlen(line), &byteswritten, NULL); 
    }
}

void VNCLog::ReallyPrint(const char* format, va_list ap) 
{
	TCHAR line[(LINE_BUFFER_SIZE * 2) + 1]; // sf@2006 - Prevents buffer overflow
    _vsnprintf_s(line, (LINE_BUFFER_SIZE * 2) + 1, LINE_BUFFER_SIZE, format, ap);

	SYSTEMTIME CurrSysTime;
	TCHAR timestr[(LINE_BUFFER_SIZE * 2) + 1];
	GetLocalTime((LPSYSTEMTIME)&CurrSysTime);
	sprintf_s(timestr, (LINE_BUFFER_SIZE * 2) + 1, "%02d/%02d/%04d %02d:%02d:%02d.%03d %s\r\n",
						CurrSysTime.wDay,
						CurrSysTime.wMonth,
						CurrSysTime.wYear,					
						CurrSysTime.wHour,
						CurrSysTime.wMinute,
						CurrSysTime.wSecond,
						CurrSysTime.wMilliseconds,
						line);

	ReallyPrintLine(timestr);
}

VNCLog::~VNCLog()
{
    CloseFile();
	if (m_filename != NULL)
	{
		free(m_filename);
		m_filename = NULL;
	}
}

void VNCLog::GetLastErrorMsg(LPSTR szErrorMsg) {

   DWORD  dwErrorCode = GetLastError();

   // Format the error message.
   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER 
         | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &szErrorMsg,
         0, NULL);
}
