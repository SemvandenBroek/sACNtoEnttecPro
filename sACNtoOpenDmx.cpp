// sACNtoOpenDmx.cpp : Defines the entry point for the application.
//

#include "sACNtoOpenDmx.h"

using namespace std;

const int my_start_addr = 1;
const int MY_DMX_FOOTPRINT = 1;

uint8_t dmx_data;

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

		// Example for an sACN-enabled fixture...
		if (header.start_code == 0 && my_start_addr + MY_DMX_FOOTPRINT <= header.slot_count)
		{
			memcpy(&dmx_data, &pdata[my_start_addr], MY_DMX_FOOTPRINT);
			// Act on the data somehow
			std::cout << static_cast<int>(dmx_data) << std::endl;
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

int main()
{
	cout << "Hello CMake." << endl;

	etcpal::Logger logger;
	sacn::Init(logger);
	
	etcpal::Uuid my_cid = etcpal::Uuid();
	std::string my_name = "sACNtoOpenDmx";
	
	sacn::Receiver::Settings config(1);
	sacn::Receiver receiver;

	MyNotifyHandler my_notify_handler;
	receiver.Startup(config, my_notify_handler);

	bool keep_running = true;

	while (keep_running) 
	{
		int ch = getchar();
		keep_running = false;
	}

	receiver.Shutdown();
}