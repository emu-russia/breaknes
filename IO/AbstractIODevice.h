#pragma once

namespace IO
{
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
		std::map<size_t, IOState> states;

	public:
		IODevice() {}
		virtual ~IODevice() {}

		virtual uint32_t GetID() { return DeviceID::Abstract; }
		virtual std::string GetName() { return "Abstract"; }
		virtual void GetIOStates(std::list<size_t>& io_states) {}

		virtual void SetState(size_t io_state, uint32_t value) {}
		virtual uint32_t GetState(size_t io_state) { return 0; }

		virtual void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[]) {}
	};
}
