//  Copyright (C) 2005-2010, Echogent Systems, Inc. All Rights Reserved.
//
//  This file is part of the InstantVNC application.
//
//  InstantVNC is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//  TextLister.cpp

#include "TextLister.h"
#include "resource.h"
#pragma warning( disable : 4786 )

#include "..\InstantVNC\TextData.h"

extern CTextData objTextData;

MapString mapCaptions;
MapString mapStrings;

CTextLister::CTextLister()
{

}

CTextLister::~CTextLister()
{

}

void CTextLister::FillStartingData()
{
	mapCaptions.clear();

	mapCaptions[41058] = "Button: OK";
	mapStrings[41058] = "OK";

	mapCaptions[41059] = "Button: Cancel";
	mapStrings[41059] = "Cancel";

	mapCaptions[41050] = "Error: Empty Address";
	mapStrings[41050] = "Empty Address has been found";

	mapCaptions[41051] = "Error: Empty Login Name";
	mapStrings[41051] = "Empty Login Name has been found";

	mapCaptions[41052] = "Error: Empty Password";
	mapStrings[41052] = "Empty Password has been found";

	mapCaptions[41053] = "Label: Address";
	mapStrings[41053] = "Address:";

	mapCaptions[41054] = "Label: Password";
	mapStrings[41054] = "Password:";

	mapCaptions[41055] = "Label: Login Name";
	mapStrings[41055] = "Login Name:";

	mapCaptions[41056] = "Label: OK button description";
	mapStrings[41056] = "After you click OKAY your PC will accept remote control.";

	mapCaptions[41057] = "Label: Main page help";
	mapStrings[41057] = "Click OKAY below to allow InstantVNC to make a connection to this echoServer";

	mapCaptions[41060] = "Label: Proxy Host";
	mapStrings[41060] = "Proxy Host";

	mapCaptions[41061] = "Label: Proxy Username";
	mapStrings[41061] = "Proxy Username";

	mapCaptions[41062] = "Label: Proxy Password";
	mapStrings[41062] = "Proxy Password";

	mapCaptions[41066] = "Button: Advanced Settings";
	mapStrings[41066] = "Advanced Settings...";

	mapCaptions[41063] = "Button: Send";
	mapStrings[41063] = "Send";

	mapCaptions[41064] = "Button: Minimize";
	mapStrings[41064] = "Minimize";

	//errors
	mapCaptions[41067] = "Error: echoWare DLL not found";
	mapStrings[41067] = "Sorry...InstantVNC cannot find the required echoWare DLL";

	mapCaptions[41068] = "Error: echoServer not found";
	mapStrings[41068] = "Sorry...cannot find a server at <#ADDRESS>";

	mapCaptions[41069] = "Error: echoServer not found advanced";
	mapStrings[41069] = "EchoServer has not been found at #ADDRESS:#PORT\n\nVerify that you have entered correct EchoServer IP/DNS";

	mapCaptions[41070] = "Error: echoServer authentication failed";
	mapStrings[41070] = "EchoServer: #ADDRESS\n\nFailed to connect to the EchoServer.\n\nVerify that you have entered correct userID, password and EchoServer IP/DNS.";

	mapCaptions[41071] = "Success: echoServer connection established";
	mapStrings[41071] = "Success! You are now connected to <#ADDRESS> as\r\n<#USER>. To end this connection, right-click the service\r\ntray icon below, and select EXIT.";

	mapCaptions[41072] = "Success: echoServer connection established with password";
	mapStrings[41072] = "Success! You are now connected to <#ADDRESS> as\r\n<#USER>. To end this connection, right-click the service\r\ntray icon below, and select EXIT.\r\nThe login password for this session is #PASS";

	mapCaptions[41073] = "Status: Connecting";
	mapStrings[41073] = "Contacting server at <#ADDRESS>...";

	mapCaptions[41074] = "Status: Authenticating";
	mapStrings[41074] = "Authenticating with server...";

	mapCaptions[41075] = "Status: Resolving";
	mapStrings[41075] = "Resolving <#ADDRESS>...";

	mapCaptions[41076] = "Tray Icon Menu: Disconnect";
	mapStrings[41076] = "Disconnect";

	mapCaptions[41077] = "Tray Icon Menu: Connect";
	mapStrings[41077] = "Connect";

	mapCaptions[41078] = "Tray Icon Menu: About...";
	mapStrings[41078] = "About...";

	mapCaptions[41079] = "Tray Icon Menu: Exit";
	mapStrings[41079] = "Exit";

	mapCaptions[41080] = "Label: About link hint";
	mapStrings[41080] = "For more Information and Links please visit:";

	mapCaptions[41081] = "Tray Icon Status: Connected to ";
	mapStrings[41081] = "Connected to:";

	mapCaptions[41082] = "Tray Icon Status: Connected as";
	mapStrings[41082] = "Connected as:";

	mapCaptions[41083] = "Tray Icon Status: Not Listening";
	mapStrings[41083] = "Not Listening";


	mapCaptions[41084] = "Label: Prompt for host";
	mapStrings[41084] = "Please provide an address for InstantVNC:";

	mapCaptions[41085] = "Label: Prompt for login name";
	mapStrings[41085] = "Please provide a login name for InstantVNC:";

	mapCaptions[41086] = "Label: Prompt for address";
	mapStrings[41086] = "Please provide a login password for InstantVNC:";

	mapCaptions[41087] = "Caption: Address Prompt";
	mapStrings[41087] = "InstantVNC - Enter Address";

	mapCaptions[41088] = "Caption: Login Name Prompt";
	mapStrings[41088] = "InstantVNC - Enter Login Name";

	mapCaptions[41089] = "Caption: Password Prompt";
	mapStrings[41089] = "InstantVNC - Enter Login Password";

	mapCaptions[41090] = "Label: About login password";
	mapStrings[41090] = "The login password for this session is #PASS";

	mapCaptions[41091] = "Caption: Password";
	mapStrings[41091] = "Password: ";


	if (objTextData.Data.size() > 0)
	{
		MapString::iterator itr;
		for (itr = objTextData.Data.begin(); itr != objTextData.Data.end(); itr++)
		{
			mapStrings[itr->first] = itr->second;
		}	
		return;
	}
}


int CTextLister::Show(HINSTANCE hInstance, HWND hwndParent)
{
	_hwndParent = hwndParent;
	int nResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_Text), hwndParent, (DLGPROC)DialogProc, (LPARAM)this);
	return nResult;
}

LRESULT CALLBACK CTextLister::DialogProc( HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	CTextLister *g_pDlg = (CTextLister*)lParam;
	switch(iMsg)
	{
		case WM_INITDIALOG:
		{
			g_pDlg->OnInitDialog(hDlg);
			break;
		}
		case WM_COMMAND:
		{
			UINT uiCommand = HIWORD(wParam);
			switch(uiCommand)
			{
				case LBN_SELCHANGE:
				{
					HWND hList = GetDlgItem(hDlg, IDC_TextList);
					int nIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
					int nStringIndex = SendMessage(hList, LB_GETITEMDATA, nIndex, 0);
					SendMessage(GetDlgItem(hDlg, IDC_Text), WM_SETTEXT, 0, (LONG)mapStrings[nStringIndex].c_str());
					return TRUE;
				}
				case EN_CHANGE:
				{
					HWND hList = GetDlgItem(hDlg, IDC_TextList);
					HWND hText = GetDlgItem(hDlg, IDC_Text);
					int nIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
					if (LB_ERR == nIndex)
						return TRUE;
					int nStringIndex = SendMessage(hList, LB_GETITEMDATA, nIndex, 0);
					int nLen = SendMessage(hText, WM_GETTEXTLENGTH, 0, 0);
					if (nLen == 0)
						mapStrings[nStringIndex] = "";
					else
					{
						char* data = new char[nLen + 1];
						SendMessage(hText, WM_GETTEXT, nLen + 1, (LONG)data);
						mapStrings[nStringIndex] = data;
						delete [] data;
					}
					return TRUE;
				}
			}

			uiCommand = LOWORD(wParam);
			switch(uiCommand)
			{
				case IDOK:
				{
					mapStrings.swap(objTextData.Data);
					EndDialog( hDlg, uiCommand);
					return TRUE;
				}
				case IDCANCEL:
				{
					EndDialog( hDlg, uiCommand);
					return TRUE;
				}
			}

		}
		default:
			return 0;
	}
	return 0;
}

void CTextLister::OnInitDialog( HWND hDlg )
{
	_hwnd = hDlg;
	HWND hList = GetDlgItem(_hwnd, IDC_TextList);
	HWND hText = GetDlgItem(_hwnd, IDC_Text);
	SetWindowLong(hList, GWL_STYLE, GetWindowLong(hList, GWL_STYLE) | LBS_NOTIFY );
	SetWindowLong(hText, GWL_STYLE, GetWindowLong(hList, GWL_STYLE) | ES_MULTILINE );
	FillStartingData();
	MapString::iterator itr;
	for (itr = mapCaptions.begin(); itr != mapCaptions.end(); itr++)
	{
		int nIndex = SendMessage(hList, LB_ADDSTRING, 0, (long)itr->second.c_str());
		SendMessage(hList, LB_SETITEMDATA, nIndex, (long)itr->first);
	}
}