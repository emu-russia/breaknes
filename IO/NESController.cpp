// NES Controller simulator
#include "pch.h"

using namespace BaseLogic;

namespace IO
{
	NESController::NESController() : IODevice()
	{
		states[(size_t)NESControllerState::Up].actuator_name = "Up";
		states[(size_t)NESControllerState::Up].value = 0;
		states[(size_t)NESControllerState::Down].actuator_name = "Down";
		states[(size_t)NESControllerState::Down].value = 0;
		states[(size_t)NESControllerState::Left].actuator_name = "Left";
		states[(size_t)NESControllerState::Left].value = 0;
		states[(size_t)NESControllerState::Right].actuator_name = "Right";
		states[(size_t)NESControllerState::Right].value = 0;
		states[(size_t)NESControllerState::Select].actuator_name = "Select";
		states[(size_t)NESControllerState::Select].value = 0;
		states[(size_t)NESControllerState::Start].actuator_name = "Start";
		states[(size_t)NESControllerState::Start].value = 0;
		states[(size_t)NESControllerState::B].actuator_name = "B";
		states[(size_t)NESControllerState::B].value = 0;
		states[(size_t)NESControllerState::A].actuator_name = "A";
		states[(size_t)NESControllerState::A].value = 0;
	}

	NESController::~NESController()
	{
	}

	uint32_t NESController::GetID()
	{
		return (uint32_t)DeviceID::NESController;
	}

	std::string NESController::GetName()
	{
		return "NES Controller";
	}

	int NESController::GetIOStates()
	{
		return (int)NESControllerState::Max;
	}

	std::string NESController::GetIOStateName(size_t io_state)
	{
		if (io_state < (size_t)NESControllerState::Max) {
			return states[io_state].actuator_name;
		}
		else {
			return "";
		}
	}

	void NESController::SetState(size_t io_state, uint32_t value)
	{
		if (io_state < (size_t)NESControllerState::Max) {
			states[io_state].value = value;
		}
	}

	uint32_t NESController::GetState(size_t io_state)
	{
		if (io_state < (size_t)NESControllerState::Max) {
			return states[io_state].value;
		}
		else {
			return 0;
		}
	}

	void NESController::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analog[])
	{
		TriState clk = inputs[0];
		TriState latch = inputs[1];
		TriState Q5;
		TriState Q6;
		TriState Q7;

		if (trace_enable && latch == TriState::One) {
			start_trace = true;
			posedge_counter = 0;
		}

		uint8_t buttons_state = 0;
		buttons_state |= (states[(size_t)NESControllerState::Right].value & 1) << 0;
		buttons_state |= (states[(size_t)NESControllerState::Left].value & 1) << 1;
		buttons_state |= (states[(size_t)NESControllerState::Down].value & 1) << 2;
		buttons_state |= (states[(size_t)NESControllerState::Up].value & 1) << 3;
		buttons_state |= (states[(size_t)NESControllerState::Start].value & 1) << 4;
		buttons_state |= (states[(size_t)NESControllerState::Select].value & 1) << 5;
		buttons_state |= (states[(size_t)NESControllerState::B].value & 1) << 6;
		buttons_state |= (states[(size_t)NESControllerState::A].value & 1) << 7;
		// Pressed button shorts the Px input to ground
		buttons_state = ~buttons_state;

		sr.sim(clk, latch, TriState::Zero, buttons_state, Q5, Q6, Q7);
		outputs[0] = Q7;		// pin4 (D0)
		outputs[1] = TriState::Z;	// pin5 (D4)
		outputs[2] = TriState::Z;	// pin6 (D3)

		if (start_trace) {
			Trace(inputs, outputs);
		}

		if (trace_enable) {
			if (IsPosedge(prev_clk, clk)) {
				posedge_counter++;
				if (posedge_counter >= 8) {
					start_trace = false;
				}
			}
			prev_clk = clk;
		}
	}

	void NESController::Trace(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[])
	{
		printf("seq: %d, sr: 0x%02X, Q7: %d\n", (int)posedge_counter, sr.get(), ToByte(outputs[0]));
	}
}
