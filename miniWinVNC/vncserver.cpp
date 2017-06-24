//  Copyright (C) 2002 Ultr@Vnc Team Members. All Rights Reserved.
//  Copyright (C) 2000-2002 Const Kaplinsky. All Rights Reserved.
//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
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


// vncServer.cpp

// vncServer class implementation

// Includes
#include "stdhdrs.h"
#include <omnithread.h>
#include <string.h>
#include <lmcons.h>

// Custom
#include "WinVNC.h"
#include "vncServer.h"
#include "vncSockConnect.h"
#include "vncClient.h"
#include "vncService.h"
#include "vncTimedMsgBox.h"
#include "mmSystem.h" // sf@2002
#include "../Common/InterfaceDllProxyInfo.h"

#include "localization.h" // ACT : Add localization on messages
bool g_Server_running;
extern bool g_Desktop_running;
void*	vncServer::pThis;

// vncServer::UpdateTracker routines

void
vncServer::ServerUpdateTracker::add_changed(const rfb::Region2D &rgn) {
	vncClientList::iterator i;
	
	omni_mutex_lock l(m_server->m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_server->m_authClients.begin(); i != m_server->m_authClients.end(); i++)
	{
		// Post the update
		// REalVNC 336 change
		// m_server->GetClient(*i)->GetUpdateTracker().add_changed(rgn);
		vncClient* client = m_server->GetClient(*i);
		omni_mutex_lock l(client->GetUpdateLock());
		client->GetUpdateTracker().add_changed(rgn);

	}
}

void
vncServer::ServerUpdateTracker::add_cached(const rfb::Region2D &rgn) {
	vncClientList::iterator i;
	
	omni_mutex_lock l(m_server->m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_server->m_authClients.begin(); i != m_server->m_authClients.end(); i++)
	{
		// Post the update
		// RealVNC 336 change
		// m_server->GetClient(*i)->GetUpdateTracker().add_cached(rgn);
		vncClient* client = m_server->GetClient(*i);
		omni_mutex_lock l(client->GetUpdateLock());
		client->GetUpdateTracker().add_cached(rgn);
	}
}





void
vncServer::ServerUpdateTracker::add_copied(const rfb::Region2D &dest, const rfb::Point &delta) {
	vncClientList::iterator i;
	
	omni_mutex_lock l(m_server->m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_server->m_authClients.begin(); i != m_server->m_authClients.end(); i++)
	{
		// Post the update
		// RealVNC 336 change
		//m_server->GetClient(*i)->GetUpdateTracker().add_copied(dest, delta);
		vncClient* client = m_server->GetClient(*i);
		omni_mutex_lock l(client->GetUpdateLock());
		client->GetUpdateTracker().add_copied(dest, delta);

	}
}



// Constructor/destructor
//vncServer::vncServer()
//#IR:100303: * : modified constructor of the vncServer
vncServer::vncServer(UINT portRFB, UINT portHTTP)
//Default:           0,            0
{
	// used for our retry timer proc;
	pThis = this;
	// Initialise some important stuffs...
	g_Server_running=true;
	m_socketConn = NULL;
//	m_httpConn = NULL;
	m_enableHttpConn = false;
	m_enableXdmcpConn = false;
	m_desktop = NULL;
	m_name = NULL;
	
	//(#IR:100303: * : modified code block to accept port numbers
	//m_port = DISPLAY_TO_PORT(0);
	//m_port_http = DISPLAY_TO_HPORT(0); // TightVNC 1.2.7
	(0 == portRFB) ? (m_port = DISPLAY_TO_PORT(0)) : (m_port = portRFB);
	(0 == portHTTP) ? (m_port_http = DISPLAY_TO_HPORT(0)) : (m_port_http = portHTTP);
	//)
	
	m_autoportselect = TRUE;
	m_passwd_required = TRUE;
	m_auth_hosts = 0;
	m_blacklist = 0;
	{
	    vncPasswd::FromClear clearPWD;
	    memcpy(m_password, clearPWD, MAXPWLEN);
	}
	m_querysetting = 2;
	m_queryaccept = 0;
	m_querytimeout = 10;

	// Autolock settings
	m_lock_on_exit = 0;

	// Set the polling mode options
	m_poll_fullscreen = TRUE;
	m_poll_foreground = FALSE;
	m_poll_undercursor = TRUE;

	m_poll_oneventonly = FALSE;
	m_poll_consoleonly = FALSE;

	m_driver = FALSE;
	m_hook = FALSE;
	m_virtual = FALSE;
	sethook=false;
	
	// General options
	m_loopbackOnly = FALSE;
	m_loopback_allowed = FALSE;
	m_lock_on_exit = 0;
	m_connect_pri = 0;
	m_disableTrayIcon = FALSE;
	m_AllowEditClients = FALSE;

	// Set the input options
	m_enable_remote_inputs = TRUE;
	m_disable_local_inputs = FALSE;

	// Clear the client mapping table
	for (int x=0; x<MAX_CLIENTS; x++)
		m_clientmap[x] = NULL;
	m_nextid = 0;

	// Initialise the update tracker
	m_update_tracker.init(this);

	// Signal set when a client quits
	m_clientquitsig = new omni_condition(&m_clientsLock);

	// Modif sf@2002
	m_SingleWindow = FALSE;
	strcpy_s(m_szWindowName, 32, "");

	// Modif sf@2002
	m_TurboMode = false;
	// m_fCursorMoved = false;

	// sf@2002 - v1.1.2
	// m_fQueuingEnabled = false;
	m_fFileTransferEnabled = true;
	m_nDefaultScale = 1;

	// sf@2002 - Data Stream Modification Plugin handling
//	m_pDSMPlugin = new CDSMPlugin();

//	m_fDSMPluginEnabled = false;
//	strcpy(m_szDSMPlugin, "");

	m_fMSLogonRequired = false;

	m_fXRichCursor = false;

	// sf@2003 - Autoreconnect
	m_fAutoReconnect = false;
	m_AutoReconnectPort = 0;
	strcpy_s(m_szAutoReconnectAdr, 64, "");
	strcpy_s(m_szAutoReconnectId, _MAX_PATH, "");

	// sf@2005 - No FT User Impersonnation
	m_fFTUserImpersonation = true;
	m_fCaptureAlphaBlending = false;
	m_fBlackAlphaBlending = false;

	m_impersonationtoken=NULL; //byteboon

	m_xdmcpConn=NULL;

	m_impersonationtoken=NULL; // Modif Jeremy C. 
}

vncServer::~vncServer()
{
	vnclog.Print(LL_STATE, VNCLOG("shutting down server object"));

	// We don't want to retry when we are shutting down...
	m_fAutoReconnect = FALSE;

	// if we are in the middle of retrying our autoreconnect - kill the timer
	if ( m_retry_timeout > 0 )
	{
		KillTimer( NULL, m_retry_timeout );
		m_retry_timeout = 0;
	}
	// If there is a socket_conn object then delete it
	if (m_socketConn != NULL)
	{
		delete m_socketConn;
		m_socketConn = NULL;
	}

//	if (m_httpConn != NULL)
//	{
//		delete m_httpConn;
//		m_httpConn = NULL;
//	}

	// Modif Jeremy C. 
	if(m_impersonationtoken) 
		CloseHandle(m_impersonationtoken);

	// Remove any active clients!
	KillAuthClients(TRUE);
	KillUnauthClients(TRUE);

	// Wait for all the clients to die
	WaitUntilAuthEmpty();
	WaitUntilUnauthEmpty();

	// Don't free the desktop until no KillClient is likely to free it
	{	omni_mutex_lock l(m_desktopLock);

		if (m_desktop != NULL)
		{
			delete m_desktop;
			m_desktop = NULL;
		}
	}
	while (g_Desktop_running)
	{
		Sleep(100);
		vnclog.Print(LL_STATE, VNCLOG("Waiting for desktop to shutdown"));
	}

	// Don't free the authhosts string until no more connections are possible
	if (m_auth_hosts != 0)
	{
		free(m_auth_hosts);
		m_auth_hosts = 0;
	}

	if (m_clientquitsig != NULL)
	{
		delete m_clientquitsig;
		m_clientquitsig = NULL;
	}

	// Modif sf@2002 - DSMPlugin handling
//	if (m_pDSMPlugin != NULL)
//		delete(m_pDSMPlugin);

	// Free the host blacklist
	while (m_blacklist) {
		vncServer::BlacklistEntry *current = m_blacklist;
		m_blacklist=m_blacklist->_next;

		free (current->_machineName);
		delete current;
	}
	//We need to give the client thread to give some time to close
	// bad hack
	Sleep(500);

	g_Server_running=false;

	if (m_name != NULL)
	{
		free(m_name);
		m_name = NULL;
	}
}

// Client handling functions
vncClientId
vncServer::AddClient(VSocket *socket, BOOL auth, BOOL shared)
{
	return AddClient(socket, auth, shared, /*FALSE,*/ 0, /*TRUE, TRUE,*/NULL); 
}

vncClientId
vncServer::AddClient(VSocket *socket, BOOL auth, BOOL shared,rfbProtocolVersionMsg *protocolMsg)
{
	return AddClient(socket, auth, shared, /*FALSE,*/ 0, /*TRUE, TRUE,*/protocolMsg); 
}

vncClientId
vncServer::AddClient(VSocket *socket, BOOL auth, BOOL shared,
					  int capability,
					 /*BOOL keysenabled, BOOL ptrenabled,*/rfbProtocolVersionMsg *protocolMsg)
{
	vncClient *client;

	omni_mutex_lock l(m_clientsLock);

	// Try to allocate a client id...
	vncClientId clientid = m_nextid;
	do
	{
		clientid = (clientid+1) % MAX_CLIENTS;
		if (clientid == m_nextid)
		{
			delete socket;
			return -1;
		}
	}
	while (m_clientmap[clientid] != NULL);

	// Create a new client and add it to the relevant client list
	client = new vncClient();
	if (client == NULL) {
		delete socket;
		return -1;
	}

	// Set the client's settings
	client->SetProtocolVersion(protocolMsg);
	client->SetCapability(capability);
	client->EnableKeyboard(/*keysenabled &&*/ m_enable_remote_inputs);
	client->EnablePointer(/*ptrenabled &&*/ m_enable_remote_inputs);

	// Start the client
	if (!client->Init(this, socket, auth, shared, clientid))
	{
		// The client will delete the socket for us...
		vnclog.Print(LL_CONNERR, VNCLOG("failed to initialise client object"));
		delete client;
		return -1;
	}

	m_clientmap[clientid] = client;

	// Add the client to unauth the client list
	m_unauthClients.push_back(clientid);

	// Notify anyone interested about this event
	DoNotify(WM_SRV_CLIENT_CONNECT, 0, 0);

	vnclog.Print(LL_INTINFO, VNCLOG("AddClient() done"));

	return clientid;
}

BOOL
vncServer::Authenticated(vncClientId clientid)
{
	vncClientList::iterator i;
	BOOL authok = TRUE;
//	vnclog.Print(LL_INTINFO, VNCLOG("Lock2\n"));
	omni_mutex_lock l1(m_desktopLock);
//	vnclog.Print(LL_INTINFO, VNCLOG("Lock3\n"));
	omni_mutex_lock l2(m_clientsLock);

	// Search the unauthenticated client list
	for (i = m_unauthClients.begin(); i != m_unauthClients.end(); i++)
	{
		// Is this the right client?
		if ((*i) == clientid)
		{
			vncClient *client = GetClient(clientid);
			

			// Yes, so remove the client and add it to the auth list
			m_unauthClients.erase(i);

			// Create the screen handler if necessary
			if (m_desktop == NULL)
			{
				m_desktop = new vncDesktop();
				if (m_desktop == NULL)
				{
					client->Kill(TRUE);
					authok = FALSE;
					break;
				}
				if (!m_desktop->Init(this))
				{
					vnclog.Print(LL_INTINFO, VNCLOG("Desktop init failed, unlock in application mode?"));
					client->Kill(TRUE);
					authok = FALSE;
					delete m_desktop;
					m_desktop = NULL;

					break;
				}
			}

			// Tell the client about this new buffer
			client->SetBuffer(&(m_desktop->m_buffer));

			// Add the client to the auth list
			m_authClients.push_back(clientid);

			break;
		}
	}

	// Notify anyone interested of this event
	DoNotify(WM_SRV_CLIENT_AUTHENTICATED, 0, 0);

	vnclog.Print(LL_INTINFO, VNCLOG("Authenticated() done"));

	return authok;
}

void
vncServer::KillClient(vncClientId clientid, BOOL bSendCloseMsg)
{
	vncClientList::iterator i;
	BOOL done = FALSE;

	omni_mutex_lock l(m_clientsLock);

	// Find the client in one of the two lists
	for (i = m_unauthClients.begin(); i != m_unauthClients.end(); i++)
	{
		// Is this the right client?
		if ((*i) == clientid)
		{
			vnclog.Print(LL_INTINFO, VNCLOG("killing unauth client"));

			// Ask the client to die
			vncClient *client = GetClient(clientid);
			client->Kill(bSendCloseMsg);

			done = TRUE;
			break;
		}
	}
	if (!done)
	{
		for (i = m_authClients.begin(); i != m_authClients.end(); i++)
		{
			// Is this the right client?
			if ((*i) == clientid)
			{
				vnclog.Print(LL_INTINFO, VNCLOG("killing auth client"));

				// Yes, so kill it
				vncClient *client = GetClient(clientid);
				client->Kill(bSendCloseMsg);

				done = TRUE;
				break;
			}
		}
	}

	vnclog.Print(LL_INTINFO, VNCLOG("KillClient() done"));
}

//
// sf@2002 - Kill the client which has the passed name
//
void vncServer::KillClient(LPSTR szClientName, BOOL bSendCloseMsg)
{
	vncClientList::iterator i;
	omni_mutex_lock l(m_clientsLock);
	vncClient *pClient = NULL;

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		pClient = GetClient(*i);
		if (!_stricmp(pClient->GetClientName(), szClientName))
		{
			vnclog.Print(LL_INTINFO, VNCLOG("Killing client named: %s"), szClientName);
			pClient->Kill(bSendCloseMsg);
		}
	}
	vnclog.Print(LL_INTINFO, VNCLOG("KillClient() from name done"));
}


void vncServer::ShutDown()
{
	// *** JNW - Trying to fix up a lock-up when the listening socket closes
	KillAuthClients(TRUE);
	KillUnauthClients(TRUE);
	WaitUntilAuthEmpty();
	WaitUntilUnauthEmpty();

	// Is there a listening socket?
	if (m_socketConn != NULL)
	{
		m_socketConn->m_socket.Close();
		delete m_socketConn;
		m_socketConn = 0;
	}
}


//
// sf@2002 - Open a textchat window with the named client
//
void vncServer::TextChatClient(LPSTR szClientName)
{
	vncClientList::iterator i;
	omni_mutex_lock l(m_clientsLock);
	vncClient *pClient = NULL;

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		pClient = GetClient(*i);
		if (!_stricmp(pClient->GetClientName(), szClientName))
		{
			if (!pClient->IsUltraViewer())
			{
				vnclog.Print(LL_INTINFO, VNCLOG("Client %s is not Ultra. Doesn't know TextChat"), szClientName);
				vncTimedMsgBox::Do(
									sz_ID_ULTRAVNC_TEXTCHAT,
									sz_ID_WINVNC_WARNIN,
									MB_ICONINFORMATION | MB_OK
									);
				break;
			}
			vnclog.Print(LL_INTINFO, VNCLOG("TextChat with client named: %s"), szClientName);
			pClient->GetTextChatPointer()->OrderTextChat();
			break;
		}
	}
	vnclog.Print(LL_INTINFO, VNCLOG("KillClient() from name done"));
}


void
vncServer::KillAuthClients(BOOL bSendCloseMsg)
{
	vncClientList::iterator i;
	omni_mutex_lock l(m_clientsLock);

	// Tell all the authorised clients to die!
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("killing auth client"));

		// Kill the client
		GetClient(*i)->Kill(bSendCloseMsg);
	}

	vnclog.Print(LL_INTINFO, VNCLOG("KillAuthClients() done"));
}

//
// sf@2002 - Fill the passed ListBox with clients names
//
void vncServer::ListAuthClients(HWND hListBox)
{
	vncClientList::iterator i;
	omni_mutex_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		SendMessage(hListBox, 
					LB_ADDSTRING,
					0,
					(LPARAM) GetClient(*i)->GetClientName()
					);
	}
}

/*
//
// sf@2003 - Returns the first client name (IP)
//
void vncServer::FirstClientName()
{
	omni_mutex_lock l(m_clientsLock);
	return GetClient(*(m_authClients.begin()))->GetClientName();
}
*/


//
// sf@2002 - test if there's a slow client connected
// The criteria is a client that has been using a slow
// encoding for more than 10 seconds after its connection
//
bool vncServer::IsThereASlowClient()
{
	vncClientList::iterator i;
	bool fFound = false;
	omni_mutex_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		if (GetClient(*i)->IsSlowEncoding())
		{
			if (timeGetTime() - GetClient(*i)->GetConnectTime() > 10000) 
			{
				fFound = true;
				break;
			}	
			else
				continue;
		}
		else
			continue;
	}
	return fFound;
}

bool vncServer::IsThereAUltraEncodingClient()
{
	vncClientList::iterator i;
	bool fFound = false;
	omni_mutex_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		if (GetClient(*i)->IsUltraEncoding())
		{
			return true;
		}
	}
	return false;
}


void
vncServer::KillUnauthClients(BOOL bSendCloseMsg)
{
	vncClientList::iterator i;
	omni_mutex_lock l(m_clientsLock);

	// Tell all the authorised clients to die!
	for (i = m_unauthClients.begin(); i != m_unauthClients.end(); i++)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("killing unauth client"));

		// Kill the client
		GetClient(*i)->Kill(bSendCloseMsg);
	}

	vnclog.Print(LL_INTINFO, VNCLOG("KillUnauthClients() done"));
}


UINT
vncServer::AuthClientCount()
{
	omni_mutex_lock l(m_clientsLock);

	return m_authClients.size();
}

UINT
vncServer::UnauthClientCount()
{
	omni_mutex_lock l(m_clientsLock);

	return m_unauthClients.size();
}

BOOL
vncServer::UpdateWanted()
{
	vncClientList::iterator i;
	omni_mutex_lock l(m_clientsLock);

	// Iterate over the authorised clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		if (GetClient(*i)->UpdateWanted())
			return TRUE;
	}
	return FALSE;
}

BOOL
vncServer::RemoteEventReceived()
{
	vncClientList::iterator i;
	BOOL result = FALSE;
	omni_mutex_lock l(m_clientsLock);

	// Iterate over the authorised clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		result = result || GetClient(*i)->RemoteEventReceived();
	}
	return result;
}

void
vncServer::WaitUntilAuthEmpty()
{
	omni_mutex_lock l(m_clientsLock);

	// Wait for all the clients to exit
	while (!m_authClients.empty())
	{
		// Wait for a client to quit
		m_clientquitsig->wait();
	}
}

void
vncServer::WaitUntilUnauthEmpty()
{
	omni_mutex_lock l(m_clientsLock);

	// Wait for all the clients to exit
	while (!m_unauthClients.empty())
	{
		// Wait for a client to quit
		m_clientquitsig->wait();
	}
}

// Client info retrieval/setup
vncClient*
vncServer::GetClient(vncClientId clientid)
{
	if ((clientid >= 0) && (clientid < MAX_CLIENTS))
		return m_clientmap[clientid];
	return NULL;
}

vncClientList
vncServer::ClientList()
{
	vncClientList clients;

	omni_mutex_lock l(m_clientsLock);

	clients = m_authClients;

	return clients;
}

void
vncServer::SetCapability(vncClientId clientid, int capability)
{
	omni_mutex_lock l(m_clientsLock);

	vncClient *client = GetClient(clientid);
	if (client != NULL)
		client->SetCapability(capability);
}

void
vncServer::SetKeyboardEnabled(vncClientId clientid, BOOL enabled)
{
	omni_mutex_lock l(m_clientsLock);

	vncClient *client = GetClient(clientid);
	if (client != NULL)
		client->EnableKeyboard(enabled);
}

void
vncServer::SetPointerEnabled(vncClientId clientid, BOOL enabled)
{
	omni_mutex_lock l(m_clientsLock);

	vncClient *client = GetClient(clientid);
	if (client != NULL)
		client->EnablePointer(enabled);
}

int
vncServer::GetCapability(vncClientId clientid)
{
	omni_mutex_lock l(m_clientsLock);

	vncClient *client = GetClient(clientid);
	if (client != NULL)
		return client->GetCapability();
	return 0;
}

const char*
vncServer::GetClientName(vncClientId clientid)
{
	omni_mutex_lock l(m_clientsLock);

	vncClient *client = GetClient(clientid);
	if (client != NULL)
		return client->GetClientName();
	return NULL;
}

// RemoveClient should ONLY EVER be used by the client to remove itself.
void
vncServer::RemoveClient(vncClientId clientid)
{
	vncClientList::iterator i;
	BOOL done = FALSE;
//	vnclog.Print(LL_INTINFO, VNCLOG("Lock1\n"));
	omni_mutex_lock l1(m_desktopLock);
//	vnclog.Print(LL_INTINFO, VNCLOG("Lock3\n"));
	{	omni_mutex_lock l2(m_clientsLock);

		// Find the client in one of the two lists
		for (i = m_unauthClients.begin(); i != m_unauthClients.end(); i++)
		{
			// Is this the right client?
			if ((*i) == clientid)
			{
				vnclog.Print(LL_INTINFO, VNCLOG("removing unauthorised client"));

				// Yes, so remove the client and kill it
				m_unauthClients.erase(i);
				m_clientmap[clientid] = NULL;
				done = TRUE;
				break;
			}
		}
		if (!done)
		{
			for (i = m_authClients.begin(); i != m_authClients.end(); i++)
			{
				// Is this the right client?
				if ((*i) == clientid)
				{
					vnclog.Print(LL_INTINFO, VNCLOG("removing authorised client"));

					// Yes, so remove the client and kill it
					m_authClients.erase(i);
					m_clientmap[clientid] = NULL;

					done = TRUE;
					break;
				}
			}
		}

		// Signal that a client has quit
		m_clientquitsig->signal();

	} // Unlock the clientLock

	// Are there any authorised clients connected?
	if (m_authClients.empty() && (m_desktop != NULL))
	{
		vnclog.Print(LL_STATE, VNCLOG("deleting desktop server"));

		// Are there locksettings set?
		if (LockSettings() == 1)
		{
		    // Yes - lock the machine on disconnect!
			vncService::LockWorkstation();
		} else if (LockSettings() > 1)
		{
		    char username[UNLEN+1];

		    vncService::CurrentUser((char *)&username, sizeof(username));
		    if (strcmp(username, "") != 0)
		    {
				// Yes - force a user logoff on disconnect!
				if (!ExitWindowsEx(EWX_LOGOFF, 0))
					vnclog.Print(LL_CONNERR, VNCLOG("client disconnect - failed to logoff user!"));
		    }
		}

		// Delete the screen server
		delete m_desktop;
		m_desktop = NULL;
		vnclog.Print(LL_STATE, VNCLOG("desktop deleted"));
	}

	// Notify anyone interested of the change
	DoNotify(WM_SRV_CLIENT_DISCONNECT, 0, 0);

	vnclog.Print(LL_INTINFO, VNCLOG("RemoveClient() done"));
}

// NOTIFICATION HANDLING!

// Connect/disconnect notification
BOOL
vncServer::AddNotify(HWND hwnd)
{
	omni_mutex_lock l(m_clientsLock);

	// Add the window handle to the list
	m_notifyList.push_front(hwnd);

	return TRUE;
}

BOOL
vncServer::RemNotify(HWND hwnd)
{
	omni_mutex_lock l(m_clientsLock);

	// Remove the window handle from the list
	vncNotifyList::iterator i;
	for (i=m_notifyList.begin(); i!=m_notifyList.end(); i++)
	{
		if ((*i) == hwnd)
		{
			// Found the handle, so remove it
			m_notifyList.erase(i);
			return TRUE;
		}
	}

	return FALSE;
}

// Send a notification message
void
vncServer::DoNotify(UINT message, WPARAM wparam, LPARAM lparam)
{
	omni_mutex_lock l(m_clientsLock);

	// Send the given message to all the notification windows
	vncNotifyList::iterator i;
	for (i=m_notifyList.begin(); i!=m_notifyList.end(); i++)
	{
		PostMessage((*i), message, wparam, lparam);
	}
}

void
vncServer::UpdateMouse()
{
	vncClientList::iterator i;
	
	omni_mutex_lock l(m_clientsLock);

	// Post this mouse update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdateMouse();
	}
}

void
vncServer::UpdateClipText(const char* text)
{
	vncClientList::iterator i;
	
	omni_mutex_lock l(m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdateClipText(text);
	}
}

void
vncServer::UpdateCursorShape()
{
	vncClientList::iterator i;
	
	omni_mutex_lock l(m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdateCursorShape();
	}
}

void
vncServer::UpdatePalette()
{
	vncClientList::iterator i;
	
	omni_mutex_lock l(m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdatePalette();
	}
}

void
vncServer::UpdateLocalFormat()
{
	vncClientList::iterator i;
	
	omni_mutex_lock l(m_clientsLock);

	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->UpdateLocalFormat();
	}
}

void
vncServer::UpdateLocalClipText(LPSTR text)
{
//	vnclog.Print(LL_INTINFO, VNCLOG("Lock5\n"));
	omni_mutex_lock l(m_desktopLock);

	if (m_desktop != NULL)
		m_desktop->SetClipText(text);
}

// Name and port number handling
void
vncServer::SetName(const char * name)
{
	// Set the name of the desktop
	if (m_name != NULL)
	{
		free(m_name);
		m_name = NULL;
	}
	
	m_name = mystrdup(name);
}

// TightVNC 1.2.7
void
vncServer::SetPorts(const UINT port_rfb, const UINT port_http)
{
	if (m_port != port_rfb || m_port_http != port_http) {
		// Set port numbers to use
		m_port = port_rfb;
		m_port_http = port_http;

		// If there is already a listening socket then close and re-open it...
		BOOL socketon = SockConnected();
		SockConnect(FALSE);
		if (socketon)
		{
			SockConnect(TRUE);
		}
    }
}


// RealVNC method
/*
void
vncServer::SetPort(const UINT port)
{
    if (m_port != port)
    {
	/////////////////////////////////
	// Adjust the listen socket

	// Set the port number to use
	m_port = port;

	// If there is already a listening socket then close and re-open it...
	BOOL socketon = SockConnected();

	SockConnect(FALSE);
	if (socketon)
	    SockConnect(TRUE);

    }
}


UINT
vncServer::GetPort()
{
	return m_port;
}
*/

void
vncServer::SetPassword(const char *passwd)
{
	memcpy(m_password, passwd, MAXPWLEN);
}

void
vncServer::GetPassword(char *passwd)
{
	memcpy(passwd, m_password, MAXPWLEN);
}

// Remote input handling
void
vncServer::EnableRemoteInputs(BOOL enable)
{
	m_enable_remote_inputs = enable;
	vncClientList::iterator i;
		
	omni_mutex_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		GetClient(*i)->EnableKeyboard(m_enable_remote_inputs);
		GetClient(*i)->EnablePointer(m_enable_remote_inputs);
	}
}

BOOL vncServer::RemoteInputsEnabled()
{
	return m_enable_remote_inputs;
}

// Local input handling
void
vncServer::DisableLocalInputs(BOOL disable)
{
	m_disable_local_inputs = disable;
}


BOOL vncServer::LocalInputsDisabled()
{
	return m_disable_local_inputs;
}


// Socket connection handling
int vncServer::SockConnect(BOOL On)
{
	extern volatile bool g_bStopped;

	if (g_bStopped)
	{
		return -1;
	}
	// Are we being asked to switch socket connects on or off?
	if (On)
	{
		// Is there a listening socket?
		if (m_socketConn == NULL)
		{
			m_socketConn = new vncSockConnect();
			if (m_socketConn == NULL)
				return ERROR_CONNECTING_TO_PROXY;

			// Are we to use automatic port selection?
			if (m_autoportselect)
			{
				vnclog.Print(0, "vncServer::SockConnect: auto select port");
				BOOL ok = FALSE;
				BOOL cycle_break = FALSE;
				int res = ERROR_CONNECTING_TO_PROXY;

				// Yes, so cycle through the ports, looking for a free one!
				for (int i=0; i < 99; i++)
				{
					m_port = DISPLAY_TO_PORT(i);
					m_port_http = DISPLAY_TO_HPORT(i);

					vnclog.Print(LL_CLIENTS, VNCLOG("trying port number %d"), m_port);

					// Attempt to connect to the port
					VSocket tempsock;
					if (tempsock.Create())
					{
						if (!tempsock.Connect("localhost", m_port))
						{
							// Couldn't connect, so this port is probably usable!
							vnclog.Print(0, "vncServer::SockConnect: m_socketConn->Init(this, %d);", m_port);
							int init_res = m_socketConn->Init(this, m_port); //#here : actual place where we do try the connection and will show the startup dialog (in not silent mode)
							res = init_res;
							switch(init_res)
							{
							case CONNECTION_SUCCESSFUL:
								ok = TRUE;
								break;

							case ERROR_CONNECTING_TO_PROXY:
								cycle_break = TRUE;
								break;

							case NO_PROXY_SERVER_FOUND_TO_CONNECT:
								cycle_break = TRUE;
								break;

							case AUTHENTICATION_FAILED:
								cycle_break = TRUE;
								break;
							}

							if (ok || cycle_break)
								break;
						}
					}
				}

				if (!ok)
				{
					delete m_socketConn;
					m_socketConn = NULL;
					return res;
				}
			} else
			{
				// No autoportselect
				vnclog.Print(0, "vncServer::SockConnect: NO auto select port");
				vnclog.Print(0, "vncServer::SockConnect: m_socketConn->Init(this, %d);", m_port);
				int res = m_socketConn->Init(this, m_port);
				if (res != CONNECTION_SUCCESSFUL)
				{
					delete m_socketConn;
					m_socketConn = NULL;
					return res;
				}
			}

			// Now let's start the HTTP connection stuff
			EnableHTTPConnect(m_enableHttpConn);
			EnableXDMCPConnect(m_enableXdmcpConn);
		}
	}
	else
	{
		// *** JNW - Trying to fix up a lock-up when the listening socket closes
		KillAuthClients(TRUE);
		KillUnauthClients(TRUE);
		WaitUntilAuthEmpty();
		WaitUntilUnauthEmpty();

		// Is there a listening socket?
		if (m_socketConn != NULL)
		{
			// Close the socket
			delete m_socketConn;
			m_socketConn = NULL;
		}

		// Is there an HTTP socket active?
		EnableHTTPConnect(m_enableHttpConn);
	}

	return CONNECTION_SUCCESSFUL;
}

BOOL
vncServer::SockConnected()
{
	return m_socketConn != NULL;
}

BOOL
vncServer::EnableHTTPConnect(BOOL enable)
{
	return FALSE;
/*
	m_enableHttpConn = enable;
	if (enable && m_socketConn)
	{
		if (m_httpConn == NULL)
		{
			m_httpConn = new vncHTTPConnect;
			if (m_httpConn != NULL)
			{
				// Start up the HTTP server
				// TODO: allow applet params handling like in TightVNC 1.2.7
				if (!m_httpConn->Init(this,
									// PORT_TO_DISPLAY(m_port) + HTTP_PORT_OFFSET)
									m_port_http)
					)
				{
					delete m_httpConn;
					m_httpConn = NULL;
					return FALSE;
				}
			}
		}
	}
	else
	{
		if (m_httpConn != NULL)
		{
			// Close the socket
			delete m_httpConn;
			m_httpConn = NULL;
		}
	}

	return TRUE;
*/
}

BOOL
vncServer::EnableXDMCPConnect(BOOL enable)
{
	m_enableXdmcpConn = enable;
	if (enable && m_socketConn)
	{
		if (m_xdmcpConn==NULL)
		{
			char szCurrentDir[_MAX_PATH];
			if (GetModuleFileName(NULL, szCurrentDir, _MAX_PATH))
				{
					char* p = strrchr(szCurrentDir, '\\');
					if (p == NULL) return 0;
					*p = '\0';
					strcat_s(szCurrentDir,_MAX_PATH,"\\xdmcp\\xdmcp.exe");
				}
			STARTUPINFO ssi;
			PROCESS_INFORMATION ppi;
			ZeroMemory( &ssi, sizeof(ssi) );
			ssi.cb = sizeof(ssi);
			// Start the child process. 
			if( !CreateProcess( NULL, // No module name (use command line). 
				szCurrentDir, // Command line. 
		       NULL,             // Process handle not inheritable. 
		        NULL,             // Thread handle not inheritable. 
		       FALSE,            // Set handle inheritance to FALSE. 
		       NULL,                // No creation flags. 
		       NULL,             // Use parent's environment block. 
		       ".\\xdmcp",             // Use parent's starting directory. 
		       &ssi,              // Pointer to STARTUPINFO structure.
		       &ppi )             // Pointer to PROCESS_INFORMATION structure.
		   ) 
			{
				m_xdmcpConn=NULL;
			}
			else
			{
				WaitForInputIdle(ppi.hProcess, 10000);
				m_xdmcpConn=ppi.hProcess;
			}
			
		}
	}
	else
	{
		if (m_xdmcpConn != NULL)
		{
				TerminateProcess(m_xdmcpConn,0);
				m_xdmcpConn=NULL;

		}
	}

	return TRUE;
}

BOOL
vncServer::SetLoopbackOnly(BOOL loopbackOnly)
{
	if (loopbackOnly != m_loopbackOnly)
	{
		m_loopbackOnly = loopbackOnly;
		BOOL socketConn = SockConnected();
		SockConnect(FALSE);
		SockConnect(socketConn);
	}
	return TRUE;
}

BOOL
vncServer::LoopbackOnly()
{
	return m_loopbackOnly;
}

void
vncServer::GetScreenInfo(int &width, int &height, int &depth)
{
	rfbServerInitMsg scrinfo;
//	vnclog.Print(LL_INTINFO, VNCLOG("Lock6\n"));
	omni_mutex_lock l(m_desktopLock);

	//vnclog.Print(LL_INTINFO, VNCLOG("GetScreenInfo called\n"));

	// Is a desktop object currently active?
	// No, so create a dummy desktop and interrogate it
	if (m_desktop == NULL)
	{
		HDC				hrootdc;
		hrootdc = GetDC(NULL);
		if (hrootdc == NULL) {
				vnclog.Print(LL_INTERR, VNCLOG("Failed rootdc"));
				scrinfo.framebufferWidth = 0;
				scrinfo.framebufferHeight = 0;
				scrinfo.format.bitsPerPixel = 0;
		}
		else
		{
			scrinfo.framebufferWidth = GetDeviceCaps(hrootdc, HORZRES);
			scrinfo.framebufferHeight = GetDeviceCaps(hrootdc, VERTRES);
			HBITMAP membitmap = CreateCompatibleBitmap(hrootdc, scrinfo.framebufferWidth, scrinfo.framebufferHeight);
			if (membitmap == NULL) {
				scrinfo.framebufferWidth = 0;
				scrinfo.framebufferHeight = 0;
				scrinfo.format.bitsPerPixel = 0;
				}
			else
			{
				struct _BMInfo {
				BOOL			truecolour;
				BITMAPINFO		bmi;
				// Colormap info - comes straight after BITMAPINFO - **HACK**
				RGBQUAD			cmap[256];
				} m_bminfo;
				int result;
				HDC				hmemdc;
				memset(&m_bminfo, 0, sizeof(m_bminfo));
				m_bminfo.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				m_bminfo.bmi.bmiHeader.biBitCount = 0;
				hmemdc = CreateCompatibleDC(hrootdc);
				if (hmemdc == NULL) 
					{
						scrinfo.framebufferWidth = 0;
						scrinfo.framebufferHeight = 0;
						scrinfo.format.bitsPerPixel = 0;
					}
				else
					{
						result = ::GetDIBits(hmemdc, membitmap, 0, 1, NULL, &m_bminfo.bmi, DIB_RGB_COLORS);
						if (result == 0) 
							{
											scrinfo.framebufferWidth = 0;
											scrinfo.framebufferHeight = 0;
											scrinfo.format.bitsPerPixel = 0;
							}
						else
							{
								result = ::GetDIBits(hmemdc, membitmap,  0, 1, NULL, &m_bminfo.bmi, DIB_RGB_COLORS);
								if (result == 0) 
									{
										scrinfo.framebufferWidth = 0;
										scrinfo.framebufferHeight = 0;
										scrinfo.format.bitsPerPixel = 0;
									}
								else
									{
										scrinfo.format.bitsPerPixel = (CARD8) m_bminfo.bmi.bmiHeader.biBitCount;
										if (scrinfo.format.bitsPerPixel==24) scrinfo.format.bitsPerPixel=32;
									}
							}//result
					if (hmemdc != NULL) DeleteDC(hmemdc);
					}//memdc
				if (membitmap != NULL) DeleteObject(membitmap);
			}//membitmap
			if (hrootdc != NULL) ReleaseDC(NULL, hrootdc);
		}//rootdc
	
		// No, so create a dummy desktop and interrogate it
	/*	if (!desktop.Init(this))
		{
			scrinfo.framebufferWidth = 0;
			scrinfo.framebufferHeight = 0;
			scrinfo.format.bitsPerPixel = 0;
		}
		else
		{
			desktop.FillDisplayInfo(&scrinfo);
		}*/
	}
	else
	{
		m_desktop->FillDisplayInfo(&scrinfo);
	}

	// Get the info from the scrinfo structure
	width = scrinfo.framebufferWidth;
	height = scrinfo.framebufferHeight;
	depth = scrinfo.format.bitsPerPixel;
}

void
vncServer::SetAuthHosts(const char*hostlist) {
	omni_mutex_lock l(m_clientsLock);

	if (hostlist == 0) {
		vnclog.Print(LL_INTINFO, VNCLOG("authhosts cleared"));
		m_auth_hosts = 0;
		return;
	}

	vnclog.Print(LL_INTINFO, VNCLOG("authhosts set to \"%s\""), hostlist);
	if (m_auth_hosts != 0)
		free(m_auth_hosts);

	m_auth_hosts = mystrdup(hostlist);
}

char*
vncServer::AuthHosts() {
	omni_mutex_lock l(m_clientsLock);

	if (m_auth_hosts == 0)
		return mystrdup("");
	else
		return mystrdup(m_auth_hosts);
}

inline BOOL
MatchStringToTemplate(const char *addr, UINT addrlen,
				      const char *filtstr, UINT filtlen) {
	if (filtlen == 0)
		return 1;
	if (addrlen < filtlen)
		return 0;
	for (int x = 0; x < (int) filtlen; x++) {
		if (addr[x] != filtstr[x])
			return 0;
	}
	if ((addrlen > filtlen) && (addr[filtlen] != '.'))
		return 0;
	return 1;
}

vncServer::AcceptQueryReject
vncServer::VerifyHost(const char *hostname) {
	omni_mutex_lock l(m_clientsLock);

	// -=- Is the specified host blacklisted?
	vncServer::BlacklistEntry	*current = m_blacklist;
	vncServer::BlacklistEntry	*previous = 0;
	SYSTEMTIME					systime;
	FILETIME					ftime;
	LARGE_INTEGER				now;

	// Get the current time as a 64-bit value
	GetSystemTime(&systime);
	SystemTimeToFileTime(&systime, &ftime);
	now.LowPart=ftime.dwLowDateTime;now.HighPart=ftime.dwHighDateTime;
	now.QuadPart /= 10000000; // Convert it into seconds

	while (current) {

		// Has the blacklist entry timed out?
		if ((now.QuadPart - current->_lastRefTime.QuadPart) > 0) {

			// Yes.  Is it a "blocked" entry?
			if (current->_blocked) {
				// Yes, so unblock it & re-set the reference time
				current->_blocked = FALSE;
				current->_lastRefTime.QuadPart = now.QuadPart + 10;
			} else {
				// No, so remove it
				if (previous)
					previous->_next = current->_next;
				else
					m_blacklist = current->_next;
				vncServer::BlacklistEntry *next = current->_next;
				free(current->_machineName);
				delete current;
				current = next;
				continue;
			}

		}

		// Is this the entry we're interested in?
		if ((strcmp(current->_machineName, hostname) == 0) &&
			(current->_blocked)) {
			// Machine is blocked, so just reject it
			return vncServer::aqrReject;
		}

		previous = current;
		current = current->_next;
	}

	// Has a hostname been specified?
	if (hostname == 0) {
		vnclog.Print(LL_INTWARN, VNCLOG("verify failed - null hostname"));
		return vncServer::aqrReject;
	}

	// Set the state machine into the correct mode & process the filter
	enum vh_Mode {vh_ExpectDelimiter, vh_ExpectIncludeExclude, vh_ExpectPattern};
	vh_Mode machineMode = vh_ExpectIncludeExclude;
	
	vncServer::AcceptQueryReject verifiedHost = vncServer::aqrAccept;

	vncServer::AcceptQueryReject patternType = vncServer::aqrReject;
	UINT authHostsPos = 0;
	UINT patternStart = 0;
	UINT hostNameLen = strlen(hostname);

	// Run through the auth hosts string until we hit the end
	if (m_auth_hosts) {
		while (1) {

			// Which mode are we in?
			switch (machineMode) {

				// ExpectIncludeExclude - we should see a + or -.
			case vh_ExpectIncludeExclude:
				if (m_auth_hosts[authHostsPos] == '+') {
					patternType = vncServer::aqrAccept;
					patternStart = authHostsPos+1;
					machineMode = vh_ExpectPattern;
				} else if (m_auth_hosts[authHostsPos] == '-') {	
					patternType = vncServer::aqrReject;
					patternStart = authHostsPos+1;
					machineMode = vh_ExpectPattern;
				} else if (m_auth_hosts[authHostsPos] == '?') {	
					patternType = vncServer::aqrQuery;
					patternStart = authHostsPos+1;
					machineMode = vh_ExpectPattern;
				} else if (m_auth_hosts[authHostsPos] != '\0') {
					vnclog.Print(LL_INTWARN, VNCLOG("verify host - malformed AuthHosts string"));
					machineMode = vh_ExpectDelimiter;
				}
				break;

				// ExpectPattern - we expect to see a valid pattern
			case vh_ExpectPattern:
				// ExpectDelimiter - we're scanning for the next ':', skipping a pattern
			case vh_ExpectDelimiter:
				if ((m_auth_hosts[authHostsPos] == ':') ||
					(m_auth_hosts[authHostsPos] == '\0')) {
					if (machineMode == vh_ExpectPattern) {
						if (patternStart == 0) {
							vnclog.Print(LL_INTWARN, VNCLOG("verify host - pattern processing failed!"));
						} else {
							// Process the match
							if (MatchStringToTemplate(hostname, hostNameLen,
								&(m_auth_hosts[patternStart]), authHostsPos-patternStart)) {
								// The hostname matched - apply the include/exclude rule
								verifiedHost = patternType;
							}
						}
					}

					// We now expect another + or -
					machineMode = vh_ExpectIncludeExclude;
				}
				break;
			}

			// Have we hit the end of the pattern string?
			if (m_auth_hosts[authHostsPos] == '\0')
				break;
			authHostsPos++;
		}
	}

	// Based on the server's QuerySetting, adjust the verification result
	switch (verifiedHost) {
	case vncServer::aqrAccept:
		if (QuerySetting() >= 3)
			verifiedHost = vncServer::aqrQuery;
		break;
	case vncServer::aqrQuery:
		if (QuerySetting() <= 1)
			verifiedHost = vncServer::aqrAccept;
		else if (QuerySetting() == 4)
			verifiedHost = vncServer::aqrReject;
		break;
	case vncServer::aqrReject:
		if (QuerySetting() == 0)
			verifiedHost = vncServer::aqrQuery;
		break;
	};

	return verifiedHost;
}

void
vncServer::AddAuthHostsBlacklist(const char *machine) {
	omni_mutex_lock l(m_clientsLock);

	// -=- Is the specified host blacklisted?
	vncServer::BlacklistEntry	*current = m_blacklist;

	// Get the current time as a 64-bit value
	SYSTEMTIME					systime;
	FILETIME					ftime;
	LARGE_INTEGER				now;
	GetSystemTime(&systime);
	SystemTimeToFileTime(&systime, &ftime);
	now.LowPart=ftime.dwLowDateTime;now.HighPart=ftime.dwHighDateTime;
	now.QuadPart /= 10000000; // Convert it into seconds

	while (current) {

		// Is this the entry we're interested in?
		if (strcmp(current->_machineName, machine) == 0) {

			// If the host is already blocked then ignore
			if (current->_blocked)
				return;

			// Set the RefTime & failureCount
			current->_lastRefTime.QuadPart = now.QuadPart + 10;
			current->_failureCount++;

			if (current->_failureCount > 5)
				current->_blocked = TRUE;
			return;
		}

		current = current->_next;
	}

	// Didn't find the entry
	current = new vncServer::BlacklistEntry;
	current->_blocked = FALSE;
	current->_failureCount = 0;
	current->_lastRefTime.QuadPart = now.QuadPart + 10;
	current->_machineName = mystrdup(machine);
	current->_next = m_blacklist;
	m_blacklist = current;
}

void
vncServer::RemAuthHostsBlacklist(const char *machine) {
	omni_mutex_lock l(m_clientsLock);

	// -=- Is the specified host blacklisted?
	vncServer::BlacklistEntry	*current = m_blacklist;
	vncServer::BlacklistEntry	*previous = 0;

	while (current) {

		// Is this the entry we're interested in?
		if (strcmp(current->_machineName, machine) == 0) {
			if (previous)
				previous->_next = current->_next;
			else
				m_blacklist = current->_next;
			vncServer::BlacklistEntry *next = current->_next;
			free (current->_machineName);
			delete current;
			current = next;
			continue;
		}

		previous = current;
		current = current->_next;
	}
}

// Modif sf@2002
void vncServer::SetSingleWindowName(const char *szName)
{
    memcpy(m_szWindowName, szName, 32);
}

// Modef rdv@202
void
vncServer::SetNewSWSize(long w,long h,BOOL desktop)
{
	vncClientList::iterator i;
		
	omni_mutex_lock l(m_clientsLock);

	// Post this screen size update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		if (!GetClient(*i)->SetNewSWSize(w,h,desktop)) {
			vnclog.Print(LL_INTINFO, VNCLOG("Unable to set new desktop size"));
			KillClient(*i, TRUE);
		}
	}
}

void
vncServer::SetSWOffset(int x,int y)
{
	vncClientList::iterator i;
		
	omni_mutex_lock l(m_clientsLock);

	// Post this screen size update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->SetSWOffset(x,y);
	}

}

void
vncServer::SetScreenOffset(int x,int y,int type)
{
	vncClientList::iterator i;
		
	omni_mutex_lock l(m_clientsLock);

	// Post this screen size update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		// Post the update
		GetClient(*i)->SetScreenOffset(x,y,type);
	}

}

// Modif sf@2002 - v1.1.0 - Default Scaling
UINT vncServer::GetDefaultScale()
{
	return m_nDefaultScale;
}


BOOL vncServer::SetDefaultScale(int nScale)
{
	m_nDefaultScale = nScale;

	return TRUE;
}


BOOL vncServer::FileTransferEnabled()
{
	return m_fFileTransferEnabled;
}

BOOL vncServer::EnableFileTransfer(BOOL fEnable)
{
	m_fFileTransferEnabled = fEnable;

	return TRUE;
}

BOOL vncServer::MSLogonRequired()
{
	return m_fMSLogonRequired;
}

BOOL vncServer::RequireMSLogon(BOOL fEnable)
{
	m_fMSLogonRequired = fEnable;

	return TRUE;
}

BOOL vncServer::GetNewMSLogon()
{
	return m_fNewMSLogon;
}

BOOL vncServer::SetNewMSLogon(BOOL fEnable)
{
	m_fNewMSLogon = fEnable;

	return TRUE;
}
/*
//
// sf@2002 - v1.1.x - DSM Plugin
//
BOOL vncServer::IsDSMPluginEnabled()
{
	return m_fDSMPluginEnabled;
}

void vncServer::EnableDSMPlugin(BOOL fEnable)
{
	m_fDSMPluginEnabled = fEnable;
}

char* vncServer::GetDSMPluginName()
{
	return m_szDSMPlugin;
}


void vncServer::SetDSMPluginName(char* szDSMPlugin)
{
	strcpy(m_szDSMPlugin, szDSMPlugin);
	return;
}

//
// Load if necessary and initialize the current DSMPlugin
// This can only be done if NO client is connected (for the moment)
//
BOOL vncServer::SetDSMPlugin(void)
{
	if (AuthClientCount() > 0) return FALSE;

	if (!IsDSMPluginEnabled()) return FALSE;

	// If the plugin is loaded, unload it first
	// sf@2003 - it has been possibly pre-configured by
	// clicking on the plugin config button
	if (m_pDSMPlugin->IsLoaded())
	{
		// sf@2003 - We check if the loaded plugin is the same than
		// the currently selected one or not
		m_pDSMPlugin->DescribePlugin();
		if (stricmp(m_pDSMPlugin->GetPluginFileName(), GetDSMPluginName()))
		{
			m_pDSMPlugin->SetEnabled(false);
			m_pDSMPlugin->UnloadPlugin();
		}
	}

	if (!m_pDSMPlugin->IsLoaded())
	{
		if (!m_pDSMPlugin->LoadPlugin(GetDSMPluginName(), false))
		{
			vnclog.Print(LL_INTINFO, VNCLOG("DSMPlugin cannot be loaded\n"));
			return FALSE;
		}
	}

	// Now that it is loaded, init it
	if (m_pDSMPlugin->InitPlugin())
	{
		char szParams[MAXPWLEN + 64];
		char password[MAXPWLEN];
		GetPassword(password);
		// Does the plugin need the VNC password to do its job ?
		if (!stricmp(m_pDSMPlugin->GetPluginParams(), "VNCPasswordNeeded"))
			strcpy(szParams, WinVncDecryptPasswd((char *)password));
		else
			strcpy(szParams, "NoPassword");

		// The second parameter tells the plugin the kind of program is using it
		// (in WinVNC : "server-app" or "server-svc"
		strcat(szParams, ",");
		strcat(szParams, vncService::RunningAsService() ? "server-svc" : "server-app");
		if (m_pDSMPlugin->SetPluginParams(NULL, szParams/*vncDecryptPasswd((char *)password)))
/*		{
			m_pDSMPlugin->SetEnabled(true); // The plugin is ready to be used
			vnclog.Print(LL_INTINFO, VNCLOG("DSMPlugin Params OK\n"));
			return TRUE;
		}
		else
		{
			m_pDSMPlugin->SetEnabled(false);
			vnclog.Print(LL_INTINFO, VNCLOG("Unable to set DSMPlugin Params\n"));
		}
	}
	else
	{
		m_pDSMPlugin->SetEnabled(false);
		vnclog.Print(LL_INTINFO, VNCLOG("Unable to init DSMPlugin\n"));
	}

	/*
	MessageBox(NULL, 
	_T(_this->m_pDSMPlugin->DescribePlugin()),
	_T("Plugin Description"), MB_OK | MB_ICONEXCLAMATION );
	*/

/*	return TRUE;
}
*/
//
// sgf@2002 - for now, we disable cache rects when more than one client
// 
void vncServer::DisableCacheForAllClients()
{
	vncClientList::iterator i;
		
	omni_mutex_lock l(m_clientsLock);

	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		GetClient(*i)->EnableCache(FALSE);
	}

}

void
vncServer::Clear_Update_Tracker() {
	vncClientList::iterator i;
	omni_mutex_lock l(m_clientsLock);
	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		GetClient(*i)->Clear_Update_Tracker();

	}
}


void vncServer::Driver(BOOL enable)
{
	sethook=true;
	m_driver = enable;
}
	
void vncServer::Hook(BOOL enable)
{
	sethook=true;
	m_hook=enable;
}

void vncServer::SetHookings()
{
	if (sethook && m_desktop)
	{
		 m_desktop->SethookMechanism(Hook(),Driver());
	}
	sethook=false;
}

void vncServer::EnableXRichCursor(BOOL fEnable)
{
	m_fXRichCursor = fEnable;
}

BOOL
vncServer::SetDisableTrayIcon(BOOL disableTrayIcon)
{
	if (disableTrayIcon != m_disableTrayIcon)
	{
		m_disableTrayIcon = disableTrayIcon;
	}
	return TRUE;
}

BOOL
vncServer::GetDisableTrayIcon()
{
	return m_disableTrayIcon;
}
BOOL
vncServer::SetAllowEditClients(BOOL AllowEditClients)
{
	if (AllowEditClients != m_AllowEditClients)
	{
		m_AllowEditClients = AllowEditClients;
	}
	return TRUE;
}

BOOL
vncServer::GetAllowEditClients()
{
	return m_AllowEditClients;
}

BOOL
vncServer::All_clients_initialalized() {
	vncClientList::iterator i;
	// Post this update to all the connected clients
	for (i = m_authClients.begin(); i != m_authClients.end(); i++)
	{
		if (!GetClient(*i)->client_settings_passed) return false;

	}
	return true;
}


bool vncServer::IsClient(vncClient* pClient)
{
  vncClientList::iterator i;
  for (i = m_authClients.begin(); i != m_authClients.end(); i++)
    if (GetClient(*i) == pClient) return true;

  return false;
}
void vncServer::AutoConnectRetry( )
{
	if ( m_fAutoReconnect )
	{
		vnclog.Print(LL_INTINFO, VNCLOG("AutoConnectRetry(): started"));
		m_retry_timeout = SetTimer( NULL, 0, (1000*30), (TIMERPROC)_timerRetryHandler );
	}
}
void CALLBACK vncServer::_timerRetryHandler( HWND /*hWnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/ )
{
	vncServer* pIface = (vncServer*)pThis;
	pIface->_actualTimerRetryHandler( );
}
void vncServer::_actualTimerRetryHandler()
{
	vnclog.Print(LL_INTINFO, VNCLOG("Attempting AutoReconnect..."));
	
	KillTimer( NULL, m_retry_timeout );
	m_retry_timeout = 0;
	
	if ( m_fAutoReconnect && strlen(m_szAutoReconnectAdr) > 0)
	{
		VSocket *tmpsock;
		tmpsock = new VSocket;
		if (tmpsock) {
			// Connect out to the specified host on the VNCviewer listen port
			tmpsock->Create();
			if (tmpsock->Connect(m_szAutoReconnectAdr, m_AutoReconnectPort)) {
				if ( strlen( m_szAutoReconnectId ) > 0 )
				{
					tmpsock->Send(m_szAutoReconnectId,250);
					tmpsock->SetTimeout(0);
				}
				// Add the new client to this server
				AddClient(tmpsock, TRUE, TRUE);
			} else {
				delete tmpsock;
				m_retry_timeout = SetTimer( NULL, 0, (1000*30), (TIMERPROC)_timerRetryHandler );
			}
		}
	}
}
