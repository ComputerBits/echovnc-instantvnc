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


// vncMenu

// This class handles creation of a system-tray icon & menu

#define SHARED_PIPE_SIZE 1024
#define IPC_TRAY_MAX_DATA 256
#define SHARED_IPC_PIPE_SERVICE_TO_SUBSERVICE "\\\\.\\pipe\\VNC_IPC_PIPE_SERVER"
#define SHARED_IPC_PIPE_SERVICE_TO_HELPER  "\\\\.\\pipe\\VNC_IPC_PIPE_HELPER"
#define INDEX_PIPE_SERVICE 0
#define INDEX_PIPE_HELPER 1

extern bool g_bIsHelperTray;
extern bool g_bIsSubService;

const char *getStringError(LONG err = -1);

#if (!defined(_WINVNC_VNCMENU))
#define _WINVNC_VNCMENU

#include "stdhdrs.h"
#include <lmcons.h>
#include "vncServer.h"
#include "vncProperties.h"
#include "vncAbout.h"
void composeTrayTip(vncServer *server, int nTipLength, char *szTip);
void composeTrayTip(const char *sAddr, const char *sUserName, const char *sPass, int nTipLength, char *szTip);

// Constants
extern const UINT MENU_PROPERTIES_SHOW;
extern const UINT MENU_DEFAULT_PROPERTIES_SHOW;
extern const UINT MENU_ABOUTBOX_SHOW;
extern const UINT MENU_SERVICEHELPER_MSG;
extern const UINT MENU_ADD_CLIENT_MSG;
extern const char *MENU_CLASS_NAME;

extern const UINT FileTransferSendPacketMessage;

const UINT TIMER_ID_FLASH_TRAY_ICON = 1;
const UINT TIMER_INTERVAL_FLASH_TRAY_ICON = 5000;

const UINT TIMER_ID_AUTO_EXIT_AFTER_LAST_VIEWER	= 1000;
const UINT TIMER_INTERVAL_AUTO_EXIT_AFTER_LAST_VIEWER = 30000;

const UINT TIMER_ID_AUTO_EXIT_IF_NO_VIEWER	= 2000;
const UINT TIMER_INTERVAL_AUTO_EXIT_IF_NO_VIEWER = 300000;


const UINT TIMER_ID_TRAY_POLL_SERVICE = 2001;
const UINT TIMER_INTERVAL_TRAY_POLL_SERVICE = 2000;

typedef struct
{
	char szAddress[64];
	char szUser[32];
	char szPasswd[16];
} ConnInfo;

enum 
{
	VNC_RUN_MODE_REGULAR_APP = 0,
	VNC_RUN_MODE_SERVICE = 1,
	VNC_RUN_MODE_HELPER_TRAY = 2,
	VNC_RUN_MODE_SUB_SERVICE = 3,
	VNC_RUN_MODE_INVALID = 4
};

class CPipe
{
private:
	int pipeNameIndex;

public:
	typedef enum {
		IPC_MESSAGE_HELLO = 1,
		IPC_MESSAGE_SET_TRAY_HWND = 2,
		IPC_MESSAGE_GET_TRAY_INFO = 3,
		IPC_MESSAGE_CLIENT_ASK_CONNECT = 4,
		IPC_MESSAGE_CLIENT_ASK_DISCONNECT = 5,
        IPC_MESSAGE_SERVICE_ASK_CONNECT = 6,
        IPC_MESSAGE_SERVICE_ASK_DISCONNECT = 7,
        IPC_MESSAGE_SERVICE_ASK_SUBSERVICE_STOP = 8,
        IPC_MESSAGE_SUB_SERVICE_ALIVE = 9,
		IPC_MESSAGE_SUB_SERVICE_ASK_CAD = 10
	} EnumIPCMessages;

	typedef struct
	{
		int trans_id;
		DWORD pid;
		DWORD sessionId;
		int msg_type;
		int length;
		char data[IPC_TRAY_MAX_DATA];
	} CIpcMessage;

	CPipe(int index) : pipeNameIndex(index) { _ASSERT((index >= INDEX_PIPE_SERVICE) && (index <= INDEX_PIPE_HELPER)); }
	void buildIpcMessage(EnumIPCMessages msg_type, int length, const void *data, CIpcMessage &msg);
	bool TalkToPeer(CIpcMessage *msg, int peerIndex);
};

// The tray menu class itself
class vncMenu
{
public:
	int PropsInit();
	void AddTrayIcon();
	void DelTrayIcon();
	vncMenu(vncServer *server, int runMode);
	void startServerPolling();
	void performServerPolling();
	void performServerDisConnect(bool bConect);
	~vncMenu();
protected:
	// Tray icon handling
	void FlashTrayIcon(BOOL flash);
	void SendTrayMsg(DWORD msg, BOOL flash);

	// Message handler for the tray window
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

	// Fields
protected:
	vncServer		*m_server;
	// Properties object for this server
	vncProperties	m_properties;

	// About dialog for this server
	vncAbout		m_about;

	// List of viewers
	HWND			m_hwnd;
	HMENU			m_hmenu;

	NOTIFYICONDATA	m_nid;
	bool			m_bServiceInstalled;

	char			m_username[UNLEN+1];

	// The icon handles
	HICON			m_winvnc_icon;
	HICON			m_flash_icon;

	int			m_runMode;
	ConnInfo		m_connInfo; //only used in case of helper tray

	BOOL StartTimerAutoExitAfterLastViewer(HWND hwnd);
	BOOL StopTimerAutoExitAfterLastViewer(HWND hwnd);

	BOOL StartTimerAutoExitIfNoViewer(HWND hwnd);
	BOOL StopTimerAutoExitIfNoViewer(HWND hwnd);

	BOOL StartTimerFlashTrayIcon(HWND hwnd);
	BOOL StopTimerFlashTrayIcon(HWND hwnd);
};

#endif
