#ifndef H_ELEVATE_PRIVILEGE
#define H_ELEVATE_PRIVILEGE

#define MAX_PWD_LEN 8
class CServicePassword
{
protected:
	static char m_passwd[MAX_PWD_LEN + 1];
	static bool m_dlgResult;
public:
	static BOOL CALLBACK CServicePasswordDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool askVncPassword(char *password, int nMaxLen);
};

class CElevatePrivilege
{
protected:
	static char m_passwd[32];
	static char m_user[32];
	static bool m_dlgResult;
public:
	static	BOOL CALLBACK ElevatePrivilegeDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool runAsUser(const char *szParam, const char *szCmdLine = NULL);
};

#endif