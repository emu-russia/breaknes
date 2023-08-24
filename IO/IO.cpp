#include "pch.h"

namespace IO
{
	IOSubsystem::IOSubsystem()
	{
	}
	
	IOSubsystem::~IOSubsystem()
	{
		// Disconnect and remove all devices

		while (!devices.empty()) {

			IOMapped* mapped = devices.back();

			if (mapped->device != nullptr) {
				delete mapped->device;
			}

			devices.pop_back();
			delete mapped;
		}
	}

	int IOSubsystem::CreateInstance(DeviceID device_id)
	{
		return -1;
	}

	void IOSubsystem::Attach(int port, int handle)
	{
		if (handle < 0)
			return;
	}
	
	void IOSubsystem::Detach(int port, int handle)
	{
		if (handle < 0)
			return;

	}

	void IOSubsystem::SetState(int handle, size_t io_state, uint32_t value)
	{
		if (handle < 0)
			return;

	}
	
	uint32_t IOSubsystem::GetState(int handle, size_t io_state)
	{
		if (handle < 0)
			return 0;

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
