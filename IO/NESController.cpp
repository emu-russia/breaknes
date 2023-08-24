// NES Controller simulator
#include "pch.h"

namespace IO
{
	NESController::NESController()
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
		return DeviceID::NESController;
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

	void NESController::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[])
	{
	}
}
