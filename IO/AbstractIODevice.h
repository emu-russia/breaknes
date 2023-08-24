#pragma once

namespace IO
{
	#define IO_MaxStates 0x100		// Maximum number of IO device actuators

	struct IOState
	{
		std::string actuator_name;
		uint32_t value;
	};

	/// <summary>
	/// Each IO device inherits this class to implement the processing of actuator states.
	/// </summary>
	class IODevice
	{
	protected:
		IOState states[IO_MaxStates]{};

	public:
		IODevice() {}
		virtual ~IODevice() {}

		virtual uint32_t GetID() { return DeviceID::Abstract; }
		virtual std::string GetName() { return "Abstract"; }
		virtual int GetIOStates() { return 0; }
		virtual std::string GetIOStateName(size_t io_state) { return ""; }

		virtual void SetState(size_t io_state, uint32_t value) {}
		virtual uint32_t GetState(size_t io_state) { return 0; }

		/// <summary>
		/// The number and types of IO port signals are determined by the Motherboard implementation. The IODevice implementation must take this into account
		/// </summary>
		virtual void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[]) {}
	};
}
