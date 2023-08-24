#pragma once

namespace IO
{
	/// <summary>
	/// A unique device identifier that completely defines the device model and logic.
	/// </summary>
	enum DeviceID
	{
		Abstract = 0,
		FamiController_1,
		FamiController_2,
		NESController,
		DendyController,
	};
}

#include "AbstractIODevice.h"
#include "FamiController.h"
#include "NESController.h"
#include "DendyController.h"

namespace IO
{
	struct IOMapped
	{
		int port;
		int handle;
		IODevice* device;
	};

	/// <summary>
	/// Each motherboard implementation inherits this class to implement port handling by their simulation.
	/// </summary>
	class IOSubsystem
	{

	protected:
		std::list<IOMapped*> devices;

	public:
		IOSubsystem();
		virtual ~IOSubsystem();

#pragma region "Interface for the emulator"

		int CreateInstance(DeviceID device_id);

		void Attach(int port, int handle);
		void Detach(int port, int handle);

		void SetState(int handle, size_t io_state, uint32_t value);
		uint32_t GetState(int handle, size_t io_state);

#pragma endregion "Interface for the emulator"

#pragma region "Interface for Motherboard implementation"

		virtual int GetPorts();
		virtual void GetPortSupportedDevices(int port, std::list<DeviceID>& devices);

		virtual void sim(int port, BaseLogic::TriState inputs[], BaseLogic::TriState outputs[]);

#pragma endregion "Interface for Motherboard implementation"

	};
}
