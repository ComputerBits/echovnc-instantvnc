#include "windows.h"
#include "StartDlg.h"
#include "commctrl.h"

static LRESULT CALLBACK StartDlgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static CStartDlg *g_pDlg = NULL;

CStartDlg::CStartDlg(void) : result(-1)
{
	hFontTitle = CreateFont(16,
						   0,
						   0,
						   0,
						   FW_BOLD,
						   0,
						   0,
						   0,
						   ANSI_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH | FF_DONTCARE,
						   "Tahoma");;

	hFontMessage = CreateFont(13,
							   0,
							   0,
							   0,
							   FW_NORMAL,
							   0,
							   0,
							   0,
							   ANSI_CHARSET,
							   OUT_DEFAULT_PRECIS,
							   CLIP_DEFAULT_PRECIS,
							   DEFAULT_QUALITY,
							   DEFAULT_PITCH | FF_DONTCARE,
							   "Tahoma");

	hFontBold = CreateFont(13,
						   0,
						   0,
						   0,
						   FW_BOLD,
						   0,
						   0,
						   0,
						   ANSI_CHARSET,
						   OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   DEFAULT_QUALITY,
						   DEFAULT_PITCH | FF_DONTCARE,
						   "Tahoma");
}

CStartDlg::~CStartDlg(void)
{
	DeleteObject(hFontTitle);
	DeleteObject(hFontMessage);
	DeleteObject(hFontBold);
	hFontTitle = hFontMessage = hFontBold = NULL;
}

void CStartDlg::OnInitDialog(HWND hWnd)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	MoveWindow(hWnd, (cx - (rect.right - rect.left)) / 2, (cy - (rect.bottom - rect.top)) / 2, rect.right - rect.left, rect.bottom - rect.top, 0);
}

static void WriteText(HDC hdc, LPCTSTR szMessage, int len, LPRECT pRect, int &x, int &y)
{
	SIZE size = {0, 0};
	int n = len * 2;
	bool bUSedCrLf = false;

	GetTextExtentPoint32(hdc, szMessage, 1, &size);
	if (x + size.cx > pRect->right - pRect->left)
	{
		x = 0;
		y += size.cy;
		bUSedCrLf = true;
	}

	do {
		n = n / 2;
		GetTextExtentPoint32(hdc, szMessage, n, &size);
	} while (x + size.cx > pRect->right - pRect->left);

	while (n < len)
	{
		GetTextExtentPoint32(hdc, szMessage, n + 1, &size);
		if (x + size.cx > pRect->right - pRect->left)
			break;
		n++;
	}

	if (!bUSedCrLf && (n < len))
	{
		int m = n;
		while (m >= 0)
		{
			bool bStop = true;
			switch (szMessage[m])
			{
				case ' ':
				case '.':
				case ';':
				case ',':
				case '!':
				case '?':
				case ')':
				case '}':
				case ']':
					break;
				default:
					bStop = false;
			}
			if (bStop)
				break;
			m--;
		}
		if (m < 0)
		{
			x = 0;
			y += size.cy;
		}
		else
		{
			if (m < len)
				n = m + 1;
		}
	}
	TextOut(hdc, pRect->left + x, pRect->top + y, szMessage, n);
	if (n < len)
	{
		x = 0;
		y += size.cy;
		WriteText(hdc, szMessage + n, len - n, pRect, x, y);
	}
	else
	{
		x += size.cx;
	}
}

void CStartDlg::OnPaint(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	SendMessage(GetDlgItem(hWnd, IDC_START_TITLE), WM_SETFONT, (WPARAM)hFontTitle, 0);

    LPCTSTR szMessage1 = "When InstantVNC is running on a Windows PC, it can be securely remote controlled from another PC anywhere in the world using the ";
	LPCTSTR szMessage2 = "EchoVNC Viewer";
	LPCTSTR szMessage3 = " application.";
	LPCTSTR szMessage4 = "To customize InstantVNC for your community of users, simply start the application with the ";
	LPCTSTR szMessage5 = "SHIFT";
	LPCTSTR szMessage6 = " key held down. In the customization screen, you can change all of InstantVNC's options, including echoServer login information, the splash screen shown at startup, even the desktop icon.";
	static int len = strlen(szMessage1);

	int x = 0;
	int y = 0;
	RECT rect;
	rect.left = 10;
	rect.right = 383;
	rect.top = 35;
	rect.bottom = 90;
	SIZE size = {0, 0};

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, hFontMessage);WriteText(hdc, szMessage1, strlen(szMessage1), &rect, x, y);
	SelectObject(hdc, hFontBold);	WriteText(hdc, szMessage2, strlen(szMessage2), &rect, x, y);
	SelectObject(hdc, hFontMessage);WriteText(hdc, szMessage3, strlen(szMessage3), &rect, x, y);

	GetTextExtentPoint32(hdc, szMessage4, 1, &size);
	x = 0;
	y += size.cy * 2;
	SelectObject(hdc, hFontMessage);WriteText(hdc, szMessage4, strlen(szMessage4), &rect, x, y);
	SelectObject(hdc, hFontBold);	WriteText(hdc, szMessage5, strlen(szMessage5), &rect, x, y);
	SelectObject(hdc, hFontMessage);WriteText(hdc, szMessage6, strlen(szMessage6), &rect, x, y);
	EndPaint(hWnd, &ps);
}

int CStartDlg::ShowStartup(HINSTANCE hInstance)
{
    HWND hWnd = 0;

	g_pDlg = new CStartDlg();
    hWnd = CreateDialog(hInstance, MAKEINTRESOURCE (IDD_START_DLG), 0, (DLGPROC)StartDlgProc);
    if (!hWnd)
    {
		delete g_pDlg;
		g_pDlg = NULL;
        return -1;
    }

    MSG  msg;
    int status;
	int dlgResult = -1;
	
    while ((status = GetMessage(&msg, 0, 0, 0)) != 0)
    {
        if (status == -1)
            return -1;
        if (!IsDialogMessage(hWnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	dlgResult = g_pDlg->result;
	delete g_pDlg;
	g_pDlg = NULL;

    return dlgResult;
}

static LRESULT CALLBACK StartDlgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg)
    {
		case WM_INITDIALOG:
			g_pDlg->OnInitDialog(hWnd);
			return FALSE;

		case WM_COMMAND:
		{
			UINT uiCommand = LOWORD(wParam);
			switch(uiCommand)
			{
				case IDC_START_RUN:
				case IDC_START_ASK:
				case IDC_START_EXIT:
					DestroyWindow(hWnd);
					g_pDlg->result = uiCommand;
					return true;

				default:
					return false;
			}
			return TRUE;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			return TRUE;

		case WM_PAINT:
			g_pDlg->OnPaint(hWnd, iMsg, wParam, lParam);
			return FALSE;

		case WM_NOTIFY: 
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			if (pnmh->idFrom == IDC_SYSLINK_URL) 
			{
				if ((pnmh->code == NM_CLICK) || (pnmh->code == NM_RETURN)) 
				{
					PNMLINK link = (PNMLINK) lParam;
					ShellExecute(NULL, "open", "http://www.echovnc.com", NULL, NULL, SW_SHOWNORMAL);
				}
			}
		}
		break;

    }
    return FALSE;
}
