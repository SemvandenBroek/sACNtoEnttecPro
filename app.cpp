#include "app.h"
#include "sACNtoOpenDmx.h"

HINSTANCE g_hInst = NULL;
UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;

// Use a guid to uniquely identify our icon
class __declspec(uuid("9bc5e0fe-9d08-4ae6-8474-05c91ce83d36")) AppIcon;
NOTIFYICONDATA nid;

CHAR appTitle[100];

BOOL AddNotificationIcon(HWND hwnd)
{
	nid = { sizeof(nid) };
	nid.hWnd = hwnd;
	nid.guidItem = __uuidof(AppIcon);
	nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
	// add the icon, setting the icon, tooltip, and callback message.
	// the icon will be identified with the GUID
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
	LoadIconMetric(g_hInst, (PCWSTR)MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
	LoadString(g_hInst, TOOLTIP_DISCONNECTED, nid.szTip, ARRAYSIZE(nid.szTip));
	Shell_NotifyIcon(NIM_ADD, &nid);

	// NOTIFYICON_VERSION_4 is prefered
	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

void SetNotificationConnected(BOOL connected)
{
	if (connected)
	{
		LoadString(g_hInst, TOOLTIP_CONNECTED, nid.szTip, ARRAYSIZE(nid.szTip));
		LoadIconMetric(g_hInst, (PCWSTR)MAKEINTRESOURCE(IDI_NOTIFICATIONICON_CONNECTED), LIM_SMALL, &nid.hIcon);
	}
	else
	{
		LoadString(g_hInst, TOOLTIP_DISCONNECTED, nid.szTip, ARRAYSIZE(nid.szTip));
		LoadIconMetric(g_hInst, (PCWSTR)MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
	}
	Shell_NotifyIcon(NIM_MODIFY, &nid);
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
		Shell_NotifyIcon(NIM_DELETE, &nid);
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
		RunThread(hwnd);
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

	case WMAPP_STATUS:
		switch (LOWORD(lParam))
		{
		case FTDI_CONNECTED:
			SetNotificationConnected(true);
			break;
		case FTDI_DISCONNECTED:
			SetNotificationConnected(false);
			break;
		}
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