#include <iostream>
#include "sacn/cpp/common.h"
#include "sacn/cpp/receiver.h"
#include "resource.h"
#include <Windows.h>
#include "sACNtoOpenDmx.h"
#include <mutex> 
#include "lib/usb_pro/pro_driver.h"


using namespace std;

unsigned char* myDmx = new unsigned char[530]();
mutex mtx;

HANDLE hThread;
bool keep_running = true;
uint16_t ftdi_connected = false;

typedef struct ThreadData {
	HWND hwnd;
} DATA, *PDATA;

class MyNotifyHandler : public sacn::Receiver::NotifyHandler
{
	void HandleUniverseData(sacn::Receiver::Handle receiver_handle, const etcpal::SockAddr& source_addr, const SacnHeaderData& header, const uint8_t* pdata, bool is_sampling)
	{
		// You wouldn't normally print a message on each sACN update, but this is just to demonstrate the
		// header fields available:
		std::cout << "Got sACN update from source " << etcpal::Uuid(header.cid).ToString() << " (address "
			<< source_addr.ToString() << ", name " << header.source_name << ") on universe "
			<< header.universe_id << ", priority " << header.priority << ", start code " << header.start_code;
		if (is_sampling)
			std::cout << " (during the sampling period)\n";
		else
			std::cout << "\n";

		if (header.start_code == 0 && ftdi_connected)
		{
			mtx.lock();
			memcpy(&myDmx[1], &pdata[0], header.slot_count);
			mtx.unlock();
		}
	}

	void MyNotifyHandler::HandleSourcesLost(sacn::Receiver::Handle handle, uint16_t universe, const std::vector<SacnLostSource>& lost_sources)
	{
		// You might not normally print a message on this condition, but this is just to demonstrate
		// the fields available:
		std::cout << "The following sources have gone offline:\n";
		for (const SacnLostSource& src : lost_sources)
		{
			std::cout << "CID: " << etcpal::Uuid(src.cid).ToString() << "\tName: " << src.name << "\tTerminated: "
				<< src.terminated << "\n";
			// Remove the source from your state tracking...
		}
	}
};

DWORD WINAPI ThreadFun(LPVOID lpParam)
{
	PDATA data = (PDATA)lpParam;

	sacn::Init();

	sacn::Receiver::Settings config(1);
	sacn::Receiver receiver;

	MyNotifyHandler my_notify_handler;
	receiver.Startup(config, my_notify_handler);

	int devices = FTDI_ListDevices();
	if (devices > 0)
	{
		ftdi_connected = FTDI_OpenDevice(0);
		unsigned char send_on_change_flag = 1;
		BOOL res = FTDI_SendData(8, &send_on_change_flag, 0);
	}

	SendNotifyMessage(data->hwnd, WMAPP_READY, NULL, NULL);

	while (keep_running && ftdi_connected) {
		mtx.lock();
		BOOL res = FTDI_SendData(6, myDmx, 75);
		mtx.unlock();
	}

	FTDI_ClosePort();

	receiver.Shutdown();
	return 0;
}

void RunThread(HWND hwnd) 
{
	PDATA pData = (PDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DATA));
	pData->hwnd = hwnd;
	DWORD dwThreadId;

	hThread = CreateThread(NULL, 0, ThreadFun, pData, 0, &dwThreadId);
}

void FinishThread()
{
	keep_running = false;
	WaitForSingleObject(hThread, INFINITE);
}