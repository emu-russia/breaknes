#include "pch.h"

namespace IO
{
	IOSubsystem::IOSubsystem()
	{
	}
	
	IOSubsystem::~IOSubsystem()
	{
	}

	int IOSubsystem::CreateInstance(DeviceID device_id)
	{
		return -1;
	}

	void IOSubsystem::Attach(int port, int handle)
	{
	}
	
	void IOSubsystem::Detach(int port, int handle)
	{
	}

	void IOSubsystem::SetState(int handle, size_t io_state, uint32_t value)
	{
	}
	
	uint32_t IOSubsystem::GetState(int handle, size_t io_state)
	{
		return 0;
	}

	int IOSubsystem::GetPorts()
	{
		return 0;
	}
	
	void IOSubsystem::GetPortSupportedDevices(int port, std::list<DeviceID>& devices)
	{
		devices.clear();
	}

	void IOSubsystem::sim(int port, BaseLogic::TriState inputs[], BaseLogic::TriState outputs[])
	{
	}
}
