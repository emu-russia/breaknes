// Famicom Controllers (1/2) simulator
#include "pch.h"

using namespace BaseLogic;

namespace IO
{

	FamiController1::FamiController1() : IODevice()
	{
		states[(size_t)FamiController1State::Up].actuator_name = "Up";
		states[(size_t)FamiController1State::Up].value = 0;
		states[(size_t)FamiController1State::Down].actuator_name = "Down";
		states[(size_t)FamiController1State::Down].value = 0;
		states[(size_t)FamiController1State::Left].actuator_name = "Left";
		states[(size_t)FamiController1State::Left].value = 0;
		states[(size_t)FamiController1State::Right].actuator_name = "Right";
		states[(size_t)FamiController1State::Right].value = 0;
		states[(size_t)FamiController1State::Select].actuator_name = "Select";
		states[(size_t)FamiController1State::Select].value = 0;
		states[(size_t)FamiController1State::Start].actuator_name = "Start";
		states[(size_t)FamiController1State::Start].value = 0;
		states[(size_t)FamiController1State::B].actuator_name = "B";
		states[(size_t)FamiController1State::B].value = 0;
		states[(size_t)FamiController1State::A].actuator_name = "A";
		states[(size_t)FamiController1State::A].value = 0;
	}

	FamiController2::FamiController2() : IODevice()
	{
		states[(size_t)FamiController2State::Up].actuator_name = "Up";
		states[(size_t)FamiController2State::Up].value = 0;
		states[(size_t)FamiController2State::Down].actuator_name = "Down";
		states[(size_t)FamiController2State::Down].value = 0;
		states[(size_t)FamiController2State::Left].actuator_name = "Left";
		states[(size_t)FamiController2State::Left].value = 0;
		states[(size_t)FamiController2State::Right].actuator_name = "Right";
		states[(size_t)FamiController2State::Right].value = 0;
		states[(size_t)FamiController2State::B].actuator_name = "B";
		states[(size_t)FamiController2State::B].value = 0;
		states[(size_t)FamiController2State::A].actuator_name = "A";
		states[(size_t)FamiController2State::A].value = 0;
		states[(size_t)FamiController2State::Volume].actuator_name = "Volume";
		states[(size_t)FamiController2State::Volume].value = 0;
		states[(size_t)FamiController2State::MicLevel].actuator_name = "MicLevel";
		states[(size_t)FamiController2State::MicLevel].value = 0;
	}

	FamiController1::~FamiController1()
	{
	}

	FamiController2::~FamiController2()
	{
	}

	uint32_t FamiController1::GetID()
	{
		return (uint32_t)DeviceID::FamiController_1;
	}

	uint32_t FamiController2::GetID()
	{
		return (uint32_t)DeviceID::FamiController_2;
	}

	std::string FamiController1::GetName()
	{
		return "Famicom Controller (Port1)";
	}

	std::string FamiController2::GetName()
	{
		return "Famicom Controller (Port2)";
	}

	int FamiController1::GetIOStates()
	{
		return (int)FamiController1State::Max;
	}

	int FamiController2::GetIOStates()
	{
		return (int)FamiController2State::Max;
	}

	std::string FamiController1::GetIOStateName(size_t io_state)
	{
		if (io_state < (size_t)FamiController1State::Max) {
			return states[io_state].actuator_name;
		}
		else {
			return "";
		}
	}

	std::string FamiController2::GetIOStateName(size_t io_state)
	{
		if (io_state < (size_t)FamiController2State::Max) {
			return states[io_state].actuator_name;
		}
		else {
			return "";
		}
	}

	void FamiController1::SetState(size_t io_state, uint32_t value)
	{
		if (io_state < (size_t)FamiController1State::Max) {
			states[io_state].value = value;
		}
	}

	void FamiController2::SetState(size_t io_state, uint32_t value)
	{
		if (io_state < (size_t)FamiController2State::Max) {
			states[io_state].value = value;
		}
	}

	uint32_t FamiController1::GetState(size_t io_state)
	{
		if (io_state < (size_t)FamiController1State::Max) {
			return states[io_state].value;
		}
		else {
			return 0;
		}
	}

	uint32_t FamiController2::GetState(size_t io_state)
	{
		if (io_state < (size_t)FamiController2State::Max) {
			return states[io_state].value;
		}
		else {
			return 0;
		}
	}

	void FamiController1::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analog[])
	{
		// TODO: board binding
		TriState clk = TriState::Zero;
		TriState latch = TriState::Zero;
		TriState Q5;
		TriState Q6;
		TriState Q7;

		uint8_t buttons_state = 0;
		buttons_state |= (states[(size_t)FamiController1State::Right].value & 1) << 0;
		buttons_state |= (states[(size_t)FamiController1State::Left].value & 1) << 1;
		buttons_state |= (states[(size_t)FamiController1State::Down].value & 1) << 2;
		buttons_state |= (states[(size_t)FamiController1State::Up].value & 1) << 3;
		buttons_state |= (states[(size_t)FamiController1State::Start].value & 1) << 4;
		buttons_state |= (states[(size_t)FamiController1State::Select].value & 1) << 5;
		buttons_state |= (states[(size_t)FamiController1State::B].value & 1) << 6;
		buttons_state |= (states[(size_t)FamiController1State::A].value & 1) << 7;
		// Pressed button shorts the Px input to ground
		buttons_state = ~buttons_state;

		sr.sim(clk, latch, TriState::Zero, buttons_state, Q5, Q6, Q7);
	}

	void FamiController2::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analog[])
	{
		// TODO: board binding
		TriState clk = TriState::Zero;
		TriState latch = TriState::Zero;
		TriState Q5;
		TriState Q6;
		TriState Q7;

		uint8_t buttons_state = 0;
		buttons_state |= (states[(size_t)FamiController2State::Right].value & 1) << 0;
		buttons_state |= (states[(size_t)FamiController2State::Left].value & 1) << 1;
		buttons_state |= (states[(size_t)FamiController2State::Down].value & 1) << 2;
		buttons_state |= (states[(size_t)FamiController2State::Up].value & 1) << 3;
		buttons_state |= 1 << 4;
		buttons_state |= 1 << 5;
		buttons_state |= (states[(size_t)FamiController2State::B].value & 1) << 6;
		buttons_state |= (states[(size_t)FamiController2State::A].value & 1) << 7;
		// Pressed button shorts the Px input to ground
		buttons_state = ~buttons_state;

		sr.sim(clk, latch, TriState::Zero, buttons_state, Q5, Q6, Q7);

		// Mike
		// TODO: Volume
		analog[0] = states[(size_t)FamiController2State::MicLevel].value / 255.0f;
	}
}
