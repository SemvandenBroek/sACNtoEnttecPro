#include <iostream>
#include "sacn/cpp/common.h"
#include "sacn/cpp/receiver.h"
#include "resource.h"
#include <Windows.h>
#include "sACNtoOpenDmx.h"
#include <mutex> 
#include "lib/usb_pro/pro_driver.h"
#include <fstream>


using namespace std;

unsigned char* myDmx = new unsigned char[530]();

HWND m_hwnd;

HANDLE hThread;
bool keep_running = true;
uint16_t ftdi_connected = false;


class MyNotifyHandler : public sacn::Receiver::NotifyHandler
{
	void HandleUniverseData(sacn::Receiver::Handle receiver_handle, const etcpal::SockAddr& source_addr, const SacnHeaderData& header, const uint8_t* pdata, bool is_sampling)
	{

		if (header.start_code == 0)
		{
			memcpy(&myDmx[1], &pdata[0], header.slot_count);

			BOOL res = FTDI_SendData(6, myDmx, 75);
			if (!res) {
				ftdi_connected = false;
				SendNotifyMessage(m_hwnd, WMAPP_STATUS, NULL, FTDI_DISCONNECTED);
			}
		}
	}

	void MyNotifyHandler::HandleSourcesLost(sacn::Receiver::Handle handle, uint16_t universe, const std::vector<SacnLostSource>& lost_sources)
	{}
};

DWORD WINAPI ThreadFun(LPVOID lpParam)
{
	sacn::Init();

	sacn::Receiver::Settings config(1);
	sacn::Receiver receiver;

	MyNotifyHandler my_notify_handler;
	receiver.Startup(config, my_notify_handler);

	int devices = FTDI_ListDevices();
	if (devices > 0)
	{
		ftdi_connected = Connect();
	}

	SendNotifyMessage(m_hwnd, WMAPP_READY, NULL, NULL);

	while (keep_running) {
		if (ftdi_connected)
		{
			Sleep(100);
		}
		else {
			// Reconnect loop
			while (!(ftdi_connected = Connect()))
			{
				Sleep(100);
			}
		}
	}

	FTDI_ClosePort();

	receiver.Shutdown();
	return 0;
}

BOOL Connect()
{
	BOOL conn = FTDI_OpenDevice(0);
	if (conn)
	{
		unsigned char send_on_change_flag = 1;
		BOOL res = FTDI_SendData(8, &send_on_change_flag, 0);
		SendNotifyMessage(m_hwnd, WMAPP_STATUS, NULL, FTDI_CONNECTED);
	}
	return conn;
}

void RunThread(HWND hwnd) 
{
	m_hwnd = hwnd;
	DWORD dwThreadId;

	hThread = CreateThread(NULL, 0, ThreadFun, NULL, 0, &dwThreadId);
}

void FinishThread()
{
	keep_running = false;
	WaitForSingleObject(hThread, INFINITE);
}