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
		IOMapped* mapped = nullptr;
		int num_devices = (int)devices.size();

		switch (device_id)
		{
			case DeviceID::FamiController_1:
				break;

			case DeviceID::FamiController_2:
				break;

			case DeviceID::NESController:
				mapped = new IOMapped;
				mapped->handle = num_devices;
				mapped->port = -1;
				mapped->device = new IO::NESController();
				devices.push_back(mapped);
				return num_devices;

			case DeviceID::DendyController:
				break;
		}

		return -1;
	}

	IOMapped* IOSubsystem::GetMappedDeviceByHandle(int handle)
	{
		for (auto it = devices.begin(); it != devices.end(); ++it) {

			IOMapped* mapped = *it;
			if (mapped->handle == handle)
				return mapped;
		}
		return nullptr;
	}

	void IOSubsystem::Attach(int port, int handle)
	{
		if (handle < 0 || port >= GetPorts() || port < 0)
			return;

		IOMapped* mapped = GetMappedDeviceByHandle(handle);
		if (mapped != nullptr) {

			mapped->port = port;
		}
	}
	
	void IOSubsystem::Detach(int port, int handle)
	{
		if (handle < 0 || port >= GetPorts() || port < 0)
			return;

		IOMapped* mapped = GetMappedDeviceByHandle(handle);
		if (mapped != nullptr) {

			mapped->port = -1;
		}
	}

	void IOSubsystem::SetState(int handle, size_t io_state, uint32_t value)
	{
		if (handle < 0)
			return;

		IOMapped* mapped = GetMappedDeviceByHandle(handle);
		if (mapped != nullptr && mapped->port >= 0) {

			mapped->device->SetState(io_state, value);
		}
	}
	
	uint32_t IOSubsystem::GetState(int handle, size_t io_state)
	{
		if (handle < 0)
			return 0;

		IOMapped* mapped = GetMappedDeviceByHandle(handle);
		if (mapped != nullptr && mapped->port >= 0) {

			return mapped->device->GetState(io_state);
		}

		return 0;
	}

	int IOSubsystem::GetNumStates(int handle)
	{
		if (handle < 0)
			return 0;

		IOMapped* mapped = GetMappedDeviceByHandle(handle);
		if (mapped != nullptr) {

			return mapped->device->GetIOStates();
		}
	}

	std::string IOSubsystem::GetStateName(int handle, size_t io_state)
	{
		if (handle < 0)
			return "";

		IOMapped* mapped = GetMappedDeviceByHandle(handle);
		if (mapped != nullptr) {
			
			return mapped->device->GetIOStateName(io_state);
		}

		return "";
	}

	// Implemented by the Motherboard instance

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
