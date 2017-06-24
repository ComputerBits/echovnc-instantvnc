

// For translating all messages
// find and translate all MessageBox                          -- done
// find and translate all MsgBox                              -- done
// find and translate all sendMessage with strings            -- done
// find and copy here and delete all messages string const    -- done
// find and translate all SetWindowText                       -- done
// find and translate all SetDlgItemText                      -- done

// All VNCLOG messages are NOT translated for now


#ifdef LOCALIZATION_MESSAGES
#include <map>
#include <string>
#include "..\Common\UserData.h"
extern CUserData* g_pUserData;
// LOCALIZATION_MESSAGES is only declare in winvnc.cpp

#define MAX_LOAD_STRING	200

char sz_ID_FAILED_INIT[MAX_LOAD_STRING];  //    "Failed to initialise the socket system"
char sz_ID_NO_EXIST_INST[MAX_LOAD_STRING];  // "No existing instance of WinVNC could be contacted"
char sz_ID_WINVNC_WARNIN[MAX_LOAD_STRING];  //  "WinVNC Warning"
char sz_ID_WINVNC_ERROR[MAX_LOAD_STRING];
char sz_ID_DESKTOP_BITBLT_ROOT [MAX_LOAD_STRING]; //   "vncDesktop : root device doesn't support BitBlt\n"       "WinVNC cannot be used with this graphic device driver";
char sz_ID_DESKTOP_BITBLT_MEM [MAX_LOAD_STRING];  //   "vncDesktop : memory device doesn't support GetDIBits\n"  "WinVNC cannot be used with this graphics device driver";
char sz_ID_DESKTOP_PLANAR_NOTC [MAX_LOAD_STRING]; //   "vncDesktop : current display is PLANAR, not CHUNKY!\n"   "WinVNC cannot be used with this graphics device driver";
char sz_ID_OUTGOING_CONNECTION [MAX_LOAD_STRING] ; //  "Outgoing Connection";
char sz_ID_RICHED32_UNLOAD [MAX_LOAD_STRING]; //  "Unable to load the Rich Edit (RICHED32.DLL) control!";
char sz_ID_RICHED32_DLL_LD [MAX_LOAD_STRING]; //  "Rich Edit Dll Loading";
char sz_ID_ULTRAVNC_TEXTCHAT[MAX_LOAD_STRING]; // "The selected client is not an Ultr@VNC Viewer !\n"	"It presumably does not support TextChat\n";

char sz_ID_CHAT_WITH_S_ULTRAVNC [MAX_LOAD_STRING]; //   " Chat with <%s> - InstantVNC"

//dialogs
char sz_IDS_ADDRESS41053 [MAX_LOAD_STRING];
char sz_IDS_PASSWORD41054 [MAX_LOAD_STRING];
char sz_IDS_LOGINNAME41055 [MAX_LOAD_STRING]; 
char sz_IDS_HINT41056 [MAX_LOAD_STRING];
char sz_IDS_HINT41057 [MAX_LOAD_STRING];
char sz_IDS_OK41058 [MAX_LOAD_STRING];
char sz_IDS_Cancel41059 [MAX_LOAD_STRING];
char sz_IDS_PROXYHOST41060 [MAX_LOAD_STRING];
char sz_IDS_PROXYUSER41061 [MAX_LOAD_STRING];
char sz_IDS_PROXYPASS41062 [MAX_LOAD_STRING];
char sz_IDS_SEND41063 [MAX_LOAD_STRING];
char sz_IDS_MINIMIZE41064 [MAX_LOAD_STRING];
char sz_IDS_CLOSE41065 [MAX_LOAD_STRING];
char sz_IDS_ADVSETTINGS41066 [MAX_LOAD_STRING];

char sz_IDS_INSTANTVNC_NOADDRESS41050 [MAX_LOAD_STRING];
char sz_IDS_INSTANTVNC_NONAME41051 [MAX_LOAD_STRING];
char sz_IDS_INSTANTVNC_NOPASSWORD1052 [MAX_LOAD_STRING];


char sz_IDS_ERROR41067 [MAX_LOAD_STRING];
char sz_IDS_ERROR41068 [MAX_LOAD_STRING];
char sz_IDS_ERROR41069 [MAX_LOAD_STRING];
char sz_IDS_ERROR41070 [MAX_LOAD_STRING];
char sz_IDS_ERROR41071 [MAX_LOAD_STRING];
char sz_IDS_ERROR41072 [MAX_LOAD_STRING];
char sz_IDS_ERROR41073 [MAX_LOAD_STRING];
char sz_IDS_ERROR41074 [MAX_LOAD_STRING];
char sz_IDS_ERROR41075 [MAX_LOAD_STRING];

char sz_IDS_STRING41076 [MAX_LOAD_STRING];
char sz_IDS_STRING41077 [MAX_LOAD_STRING];
char sz_IDS_STRING41078 [MAX_LOAD_STRING];
char sz_IDS_STRING41079 [MAX_LOAD_STRING];

char sz_IDS_WWW41080 [MAX_LOAD_STRING];

char sz_IDS_STRING41081 [MAX_LOAD_STRING];
char sz_IDS_STRING41082 [MAX_LOAD_STRING];
char sz_IDS_STRING41083 [MAX_LOAD_STRING];

char sz_IDS_STRING41084 [MAX_LOAD_STRING];
char sz_IDS_STRING41085 [MAX_LOAD_STRING];
char sz_IDS_STRING41086 [MAX_LOAD_STRING];

char sz_IDS_STRING41087 [MAX_LOAD_STRING];
char sz_IDS_STRING41088 [MAX_LOAD_STRING];
char sz_IDS_STRING41089 [MAX_LOAD_STRING];
char sz_IDS_STRING41090 [MAX_LOAD_STRING];
char sz_IDS_STRING41091 [MAX_LOAD_STRING];
char sz_IDS_STRING41092 [MAX_LOAD_STRING];
char sz_IDS_STRING41093 [MAX_LOAD_STRING];

void LoadStringCaption(HINSTANCE hInstance, UINT nId, char* szString, std::map<int, std::string>& mapStrings)
{
	LoadString(hInstance, nId, szString, MAX_LOAD_STRING);
	if (mapStrings.find(nId) != mapStrings.end())
	{
		memset(szString, 0, MAX_LOAD_STRING);
		memcpy(szString, mapStrings[nId].c_str(), __min(mapStrings[nId].size(), MAX_LOAD_STRING));
	}
}

int Load_Localization(HINSTANCE hInstance) 
{
	std::map<int, std::string> strStrings;
	if (!g_pUserData->m_szIniPath.empty())
	{
//		if (NS_Common::IsFile(szImagePath))
		{
			int nLen = 128000;
			char *buff = new char[nLen];
			int res = GetPrivateProfileSection("Text", buff, 128000, g_pUserData->m_szIniPath.c_str());
			if (res != nLen - 2)
			{
				char* pointer = buff;
				while (*pointer != 0)
				{
					int nItemLen = strlen(pointer);
					int nSeparatorPos = strstr(pointer, "=") - pointer;
					pointer[nSeparatorPos] = NULL;
					int nIndex = atoi(pointer);
					std::string str = pointer + nSeparatorPos + 1;
					CUserData::ReplaceString(str, "#N", "\n");
					CUserData::ReplaceString(str, "#R", "\r");
					strStrings[nIndex] = str;
					pointer += nItemLen + 1;
				}
			}
			delete [] buff;
		}		
	}

   LoadStringCaption(hInstance, ID_FAILED_INIT, sz_ID_FAILED_INIT, strStrings); 
   LoadStringCaption(hInstance, ID_NO_EXIST_INST, sz_ID_NO_EXIST_INST, strStrings);
   LoadStringCaption(hInstance, ID_WINVNC_ERROR, sz_ID_WINVNC_ERROR, strStrings);
   LoadStringCaption(hInstance, ID_WINVNC_WARNIN, sz_ID_WINVNC_WARNIN, strStrings); 
   LoadStringCaption(hInstance, ID_DESKTOP_BITBLT_ROOT, sz_ID_DESKTOP_BITBLT_ROOT , strStrings);
   LoadStringCaption(hInstance, ID_DESKTOP_BITBLT_MEM, sz_ID_DESKTOP_BITBLT_MEM , strStrings);
   LoadStringCaption(hInstance, ID_DESKTOP_PLANAR_NOTC, sz_ID_DESKTOP_PLANAR_NOTC , strStrings); 
   LoadStringCaption(hInstance, ID_OUTGOING_CONNECTION, sz_ID_OUTGOING_CONNECTION , strStrings); 
   LoadStringCaption(hInstance, ID_RICHED32_UNLOAD, sz_ID_RICHED32_UNLOAD , strStrings); 
   LoadStringCaption(hInstance, ID_RICHED32_DLL_LD, sz_ID_RICHED32_DLL_LD , strStrings); 
   LoadStringCaption(hInstance, ID_ULTRAVNC_TEXTCHAT, sz_ID_ULTRAVNC_TEXTCHAT, strStrings); 
   LoadStringCaption(hInstance, ID_CHAT_WITH_S_ULTRAVNC, sz_ID_CHAT_WITH_S_ULTRAVNC, strStrings);


   LoadStringCaption(hInstance, IDS_ADDRESS41053, sz_IDS_ADDRESS41053, strStrings);
   LoadStringCaption(hInstance, IDS_PASSWORD41054, sz_IDS_PASSWORD41054, strStrings);
   LoadStringCaption(hInstance, IDS_LOGINNAME41055, sz_IDS_LOGINNAME41055, strStrings);
   LoadStringCaption(hInstance, IDS_HINT41056, sz_IDS_HINT41056, strStrings);
   LoadStringCaption(hInstance, IDS_HINT41057, sz_IDS_HINT41057, strStrings);
   LoadStringCaption(hInstance, IDS_OK41058, sz_IDS_OK41058, strStrings);
   LoadStringCaption(hInstance, IDS_Cancel41059, sz_IDS_Cancel41059, strStrings);
   LoadStringCaption(hInstance, IDS_PROXYHOST41060, sz_IDS_PROXYHOST41060, strStrings);
   LoadStringCaption(hInstance, IDS_PROXYUSER41061, sz_IDS_PROXYUSER41061, strStrings);
   LoadStringCaption(hInstance, IDS_PROXYPASS41062, sz_IDS_PROXYPASS41062, strStrings);
   LoadStringCaption(hInstance, IDS_SEND41063, sz_IDS_SEND41063, strStrings);
   LoadStringCaption(hInstance, IDS_MINIMIZE41064, sz_IDS_MINIMIZE41064, strStrings);
   LoadStringCaption(hInstance, IDS_CLOSE41065, sz_IDS_CLOSE41065, strStrings);
   LoadStringCaption(hInstance, IDS_ADVSETTINGS41066, sz_IDS_ADVSETTINGS41066, strStrings);


   LoadStringCaption(hInstance, IDS_INSTANTVNC_NOADDRESS, sz_IDS_INSTANTVNC_NOADDRESS41050, strStrings);
   LoadStringCaption(hInstance, IDS_INSTANTVNC_NONAME, sz_IDS_INSTANTVNC_NONAME41051, strStrings);
   LoadStringCaption(hInstance, IDS_INSTANTVNC_NOPASSWORD, sz_IDS_INSTANTVNC_NOPASSWORD1052, strStrings);

   LoadStringCaption(hInstance, IDS_ERROR41067, sz_IDS_ERROR41067, strStrings);
   LoadStringCaption(hInstance, IDS_ERROR41068, sz_IDS_ERROR41068, strStrings);
   LoadStringCaption(hInstance, IDS_ERROR41069, sz_IDS_ERROR41069, strStrings);
   LoadStringCaption(hInstance, IDS_ERROR41070, sz_IDS_ERROR41070, strStrings);
   LoadStringCaption(hInstance, IDS_ERROR41071, sz_IDS_ERROR41071, strStrings);
   LoadStringCaption(hInstance, IDS_ERROR41072, sz_IDS_ERROR41072, strStrings);
   LoadStringCaption(hInstance, IDS_ERROR41073, sz_IDS_ERROR41073, strStrings);
   LoadStringCaption(hInstance, IDS_ERROR41074, sz_IDS_ERROR41074, strStrings);
   LoadStringCaption(hInstance, IDS_ERROR41075, sz_IDS_ERROR41075, strStrings);

   LoadStringCaption(hInstance, IDS_STRING41076, sz_IDS_STRING41076, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41077, sz_IDS_STRING41077, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41078, sz_IDS_STRING41078, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41079, sz_IDS_STRING41079, strStrings);

   LoadStringCaption(hInstance, IDS_WWW41080, sz_IDS_WWW41080, strStrings);

   LoadStringCaption(hInstance, IDS_STRING41081, sz_IDS_STRING41081, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41082, sz_IDS_STRING41082, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41083, sz_IDS_STRING41083, strStrings);

   LoadStringCaption(hInstance, IDS_STRING41084, sz_IDS_STRING41084, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41085, sz_IDS_STRING41085, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41086, sz_IDS_STRING41086, strStrings);

   LoadStringCaption(hInstance, IDS_STRING41087, sz_IDS_STRING41087, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41088, sz_IDS_STRING41088, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41089, sz_IDS_STRING41089, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41090, sz_IDS_STRING41090, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41091, sz_IDS_STRING41091, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41092, sz_IDS_STRING41092, strStrings);
   LoadStringCaption(hInstance, IDS_STRING41093, sz_IDS_STRING41093, strStrings);
  return 0;
}

#else

extern char sz_ID_FAILED_INIT[64];  //    "Failed to initialise the socket system"
extern char sz_ID_NO_EXIST_INST[64];  // "No existing instance of WinVNC could be contacted"
extern char sz_ID_WINVNC_ERROR[64];
extern char sz_ID_WINVNC_WARNIN[64];  //  "WinVNC Warning"
extern char sz_ID_AUTH_NOT_FO [128]; //    "You selected ms-logon, but the auth.dll\nwas not found.Check you installation";
extern char sz_ID_DESKTOP_BITBLT_ROOT [128]; //   "vncDesktop : root device doesn't support BitBlt\n"       "WinVNC cannot be used with this graphic device driver";
extern char sz_ID_DESKTOP_BITBLT_MEM [128];  //   "vncDesktop : memory device doesn't support GetDIBits\n"  "WinVNC cannot be used with this graphics device driver";
extern char sz_ID_DESKTOP_PLANAR_NOTC [128]; //   "vncDesktop : current display is PLANAR, not CHUNKY!\n"   "WinVNC cannot be used with this graphics device driver";
extern char sz_ID_OUTGOING_CONNECTION [64] ; //  "Outgoing Connection";
extern char sz_ID_RICHED32_UNLOAD [64]; //  "Unable to load the Rich Edit (RICHED32.DLL) control!";
extern char sz_ID_RICHED32_DLL_LD [64]; //  "Rich Edit Dll Loading";
extern char sz_ID_ULTRAVNC_TEXTCHAT[128]; // "The selected client is not an Ultr@VNC Viewer !\n"	"It presumably does not support TextChat\n";
extern char sz_ID_CHAT_WITH_S_ULTRAVNC [64]; //   " Chat with <%s> - InstantVNC"



extern char sz_IDS_ADDRESS41053 [128];
extern char sz_IDS_PASSWORD41054 [128];
extern char sz_IDS_LOGINNAME41055 [128]; 
extern char sz_IDS_HINT41056 [200];
extern char sz_IDS_HINT41057 [200];
extern char sz_IDS_OK41058 [128];
extern char sz_IDS_Cancel41059 [128];
extern char sz_IDS_PROXYHOST41060 [128];
extern char sz_IDS_PROXYUSER41061 [128];
extern char sz_IDS_PROXYPASS41062 [128];
extern char sz_IDS_SEND41063 [128];
extern char sz_IDS_MINIMIZE41064 [128];
extern char sz_IDS_CLOSE41065 [128];
extern char sz_IDS_ADVSETTINGS41066 [128];

extern char sz_IDS_INSTANTVNC_NOADDRESS41050 [128];
extern char sz_IDS_INSTANTVNC_NONAME41051 [128];
extern char sz_IDS_INSTANTVNC_NOPASSWORD1052 [128];


extern char sz_IDS_ERROR41067 [200];
extern char sz_IDS_ERROR41068 [200];
extern char sz_IDS_ERROR41069 [200];
extern char sz_IDS_ERROR41070 [200];
extern char sz_IDS_ERROR41071 [200];
extern char sz_IDS_ERROR41072 [200];
extern char sz_IDS_ERROR41073 [200];
extern char sz_IDS_ERROR41074 [200];
extern char sz_IDS_ERROR41075 [200];

extern char sz_IDS_STRING41076 [128];
extern char sz_IDS_STRING41077 [128];
extern char sz_IDS_STRING41078 [128];
extern char sz_IDS_STRING41079 [128];

extern char sz_IDS_WWW41080 [128];

extern char sz_IDS_STRING41081 [128];
extern char sz_IDS_STRING41082 [128];
extern char sz_IDS_STRING41083 [128];

extern char sz_IDS_STRING41084 [128];
extern char sz_IDS_STRING41085 [128];
extern char sz_IDS_STRING41086 [128];

extern char sz_IDS_STRING41087 [128];
extern char sz_IDS_STRING41088 [128];
extern char sz_IDS_STRING41089 [128];
extern char sz_IDS_STRING41090 [128];
extern char sz_IDS_STRING41091 [128];

extern char sz_IDS_STRING41092 [128];
extern char sz_IDS_STRING41093 [128];
#endif
