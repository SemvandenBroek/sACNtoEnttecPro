#pragma once

UINT const WMAPP_READY = WM_APP + 2;

DWORD WINAPI ThreadFun(LPVOID lpParam);
void RunThread(HWND hwnd);
void FinishThread();
