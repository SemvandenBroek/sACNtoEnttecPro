#include "app.h"
#include "sACNtoOpenDmx.h"

HINSTANCE g_hInst = NULL;
UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;

// Use a guid to uniquely identify our icon
class __declspec(uuid("9D0B8B92-4E1C-488e-A1E1-2331AFCE2CB7")) AppIcon;

CHAR appTitle[100];

BOOL AddNotificationIcon(HWND hwnd)
{
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	nid.guidItem = __uuidof(AppIcon);
	nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
	// add the icon, setting the icon, tooltip, and callback message.
	// the icon will be identified with the GUID
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
	LoadIconMetric(g_hInst, (PCWSTR)MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
	LoadString(g_hInst, IDS_APP_TITLE, nid.szTip, ARRAYSIZE(nid.szTip));
	Shell_NotifyIcon(NIM_ADD, &nid);

	// NOTIFYICON_VERSION_4 is prefered
	nid.uVersion = NOTIFYICON_VERSION_4;
	BOOL bRet = Shell_NotifyIcon(NIM_SETVERSION, &nid);
	
	if (!bRet)
	{
		for (int i = 0; i < 4; i++)
		{
			if (bRet)
				return bRet;
			++nid.uID;
			bRet = Shell_NotifyIcon(NIM_SETVERSION, &nid);
		}
	}
	return -1;
}

void ShowContextMenu(HWND hwnd)
{
	POINT pt;
	GetCursorPos(&pt);

	HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
	if (hMenu)
	{
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		if (hSubMenu)
		{
			// our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
			SetForegroundWindow(hwnd);

			// respect menu drop alignment
			UINT uFlags = TPM_RIGHTBUTTON;
			if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
			{
				uFlags |= TPM_RIGHTALIGN;
			}
			else
			{
				uFlags |= TPM_LEFTALIGN;
			}

			TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
		}
		DestroyMenu(hMenu);
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CREATE:
		// add the notification icon
		if (!AddNotificationIcon(hwnd))
		{
			MessageBox(hwnd,
				"Please read the ReadMe.txt file for troubleshooting",
				"Error adding icon", MB_OK);
			return -1;
		}
		break;

	case WM_COMMAND:
	{
		int const wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			StopApplication(hwnd);
			break;
		}
	}

	case WMAPP_NOTIFYCALLBACK:
		switch (LOWORD(lParam))
		{
		case WM_CONTEXTMENU:
		{
			ShowContextMenu(hwnd);
		}
		break;
		}
		break;

	case WMAPP_READY:
		if (InSendMessage())
			ReplyMessage(TRUE);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return TRUE;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, PWSTR pCmdLine, int nCmdShow)
{
	g_hInst = hInstance;

	LoadString(hInstance, IDS_APP_TITLE, appTitle, ARRAYSIZE(appTitle));
	

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = appTitle;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		appTitle,                     // Window class
		NULL,    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, SW_HIDE);
	RunThread(hwnd);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void StopApplication(HWND hwnd)
{
	FinishThread();
	DestroyWindow(hwnd);
}