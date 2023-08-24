#include "pch.h"

namespace IO
{
	IOSubsystem::IOSubsystem()
	{
	}
	
	IOSubsystem::~IOSubsystem()
	{
	}

	int CreateInstance(DeviceID device_id)
	{
		return -1;
	}

	void Attach(int port, int handle)
	{
	}
	
	void Detach(int port, int handle)
	{
	}

	void SetState(int handle, size_t io_state, uint32_t value)
	{
	}
	
	uint32_t GetState(int handle, size_t io_state)
	{
		return 0;
	}

	int GetPorts()
	{
		return 0;
	}
	
	void GetPortSupportedDevices(int port, std::list<DeviceID>& devices)
	{
		devices.clear();
	}

	void sim(int port, BaseLogic::TriState inputs[], BaseLogic::TriState outputs[])
	{
	}
}
