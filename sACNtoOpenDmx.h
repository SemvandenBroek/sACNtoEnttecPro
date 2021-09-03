#pragma once

UINT const WMAPP_READY = WM_APP + 2;
UINT const WMAPP_STATUS = WM_APP + 3;

DWORD WINAPI ThreadFun(LPVOID lpParam);
BOOL Connect();
void RunThread(HWND hwnd);
void FinishThread();
