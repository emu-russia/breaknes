#include "pch.h"

#include "IOProcessing.h"

namespace BreaknesSDL
{
	IOProcessor::IOProcessor()
	{
		if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
		{
			printf("IO: SDL game controller subsystem could not initialize! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			SDL_GameControllerEventState(SDL_ENABLE);
		}

		// Fixed number of controller slots. The event names reference the slot:
		// "Controller0_ButtonA", "Controller1_DPadUp", ...
		controllers.resize(kMaxControllers);
	}

	IOProcessor::~IOProcessor()
	{
		DetachDevicesFromBoard();

		for (auto it = controllers.begin(); it != controllers.end(); ++it)
		{
			if (it->gamepad != nullptr)
			{
				SDL_GameControllerClose(it->gamepad);
				it->gamepad = nullptr;
			}
		}
		controllers.clear();
		controller_slot.clear();

		if (SDL_WasInit(SDL_INIT_GAMECONTROLLER))
		{
			SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
		}
	}

	int IOProcessor::FindFreeControllerSlot()
	{
		for (size_t i = 0; i < controllers.size(); i++)
		{
			if (controllers[i].gamepad == nullptr)
				return (int)i;
		}
		return -1;
	}

	int IOProcessor::GetControllerSlot(SDL_JoystickID instance_id)
	{
		auto it = controller_slot.find(instance_id);
		if (it != controller_slot.end())
			return it->second;
		return -1;
	}

	void IOProcessor::OnControllerAdded(int device_index)
	{
		if (!SDL_IsGameController(device_index))
			return;

		int slot = FindFreeControllerSlot();
		if (slot < 0)
		{
			printf("IO: no free controller slot, ignoring device %d\n", device_index);
			return;
		}

		SDL_GameController* gamepad = SDL_GameControllerOpen(device_index);
		if (gamepad == nullptr)
		{
			printf("IO: SDL_GameControllerOpen failed: %s\n", SDL_GetError());
			return;
		}

		SDLController& ctrl = controllers[slot];
		ctrl.gamepad = gamepad;
		ctrl.instance_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad));
		memset(ctrl.axis_digital, 0xFF, sizeof(ctrl.axis_digital));	// "unknown" -> first motion always emits
		controller_slot[ctrl.instance_id] = slot;

		printf("IO: game controller added (slot %d)\n", slot);
	}

	void IOProcessor::OnControllerRemoved(SDL_JoystickID instance_id)
	{
		auto it = controller_slot.find(instance_id);
		if (it == controller_slot.end())
			return;

		int slot = it->second;
		controller_slot.erase(it);

		if (slot >= 0 && slot < (int)controllers.size())
		{
			SDLController& ctrl = controllers[slot];

			if (ctrl.gamepad != nullptr)
			{
				SDL_GameControllerClose(ctrl.gamepad);
				ctrl.gamepad = nullptr;
			}
			ctrl.instance_id = 0;
			memset(ctrl.axis_digital, 0, sizeof(ctrl.axis_digital));

			printf("IO: game controller removed (slot %d)\n", slot);
		}
	}

	void IOProcessor::PushEvent(const char* name, uint32_t value)
	{
		IOEvent ev;
		ev.name = name;
		ev.value = value;
		event_queue.push_back(ev);
	}

	void IOProcessor::PushAxisDigital(int slot, SDL_GameControllerAxis axis, const char* event_name_neg, const char* event_name_pos, int16_t value)
	{
		const int Deadzone = 8000;

		if (slot < 0 || slot >= (int)controllers.size())
			return;

		SDLController& ctrl = controllers[slot];
		int axis_index = (int)axis;

		if (axis_index < 0 || axis_index >= SDL_CONTROLLER_AXIS_MAX)
			return;

		bool neg_state = value < -Deadzone;
		bool pos_state = value > Deadzone;

		if (event_name_neg != nullptr && ctrl.axis_digital[axis_index][0] != (int)neg_state)
		{
			ctrl.axis_digital[axis_index][0] = (int)neg_state;
			PushEvent(event_name_neg, neg_state ? 1 : 0);
		}

		if (event_name_pos != nullptr && ctrl.axis_digital[axis_index][1] != (int)pos_state)
		{
			ctrl.axis_digital[axis_index][1] = (int)pos_state;
			PushEvent(event_name_pos, pos_state ? 1 : 0);
		}
	}

	void IOProcessor::ProcessEvent(const SDL_Event& event)
	{
		switch (event.type)
		{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				// Skip keyboard auto-repeat
				if (event.key.repeat != 0)
					break;

				const char* key_name = SDL_GetKeyName(event.key.keysym.sym);
				if (key_name == nullptr || key_name[0] == 0)
					break;

				PushEvent(key_name, event.key.state == SDL_PRESSED ? 1 : 0);
				break;
			}

			case SDL_CONTROLLERDEVICEADDED:
				OnControllerAdded(event.cdevice.which);
				break;

			case SDL_CONTROLLERDEVICEREMOVED:
				OnControllerRemoved((SDL_JoystickID)event.cdevice.which);
				break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			{
				int slot = GetControllerSlot((SDL_JoystickID)event.cbutton.which);
				if (slot >= 0)
				{
					std::string name = std::string("Controller") + std::to_string(slot) + "_" +
						ButtonToString((SDL_GameControllerButton)event.cbutton.button);
					PushEvent(name.c_str(), event.cbutton.state == SDL_PRESSED ? 1 : 0);
				}
				break;
			}

			case SDL_CONTROLLERAXISMOTION:
			{
				int slot = GetControllerSlot((SDL_JoystickID)event.caxis.which);
				if (slot < 0)
					break;

				std::string prefix = std::string("Controller") + std::to_string(slot) + "_";
				SDL_GameControllerAxis axis = (SDL_GameControllerAxis)event.caxis.axis;

				// The sticks are mapped to two digital events each (a "-" and a "+" direction);
				// the analog triggers are mapped to a single threshold event.
				switch (axis)
				{
					case SDL_CONTROLLER_AXIS_LEFTX:
						PushAxisDigital(slot, axis, (prefix + "AxisLeftX-").c_str(), (prefix + "AxisLeftX+").c_str(), event.caxis.value);
						break;
					case SDL_CONTROLLER_AXIS_LEFTY:
						PushAxisDigital(slot, axis, (prefix + "AxisLeftY-").c_str(), (prefix + "AxisLeftY+").c_str(), event.caxis.value);
						break;
					case SDL_CONTROLLER_AXIS_RIGHTX:
						PushAxisDigital(slot, axis, (prefix + "AxisRightX-").c_str(), (prefix + "AxisRightX+").c_str(), event.caxis.value);
						break;
					case SDL_CONTROLLER_AXIS_RIGHTY:
						PushAxisDigital(slot, axis, (prefix + "AxisRightY-").c_str(), (prefix + "AxisRightY+").c_str(), event.caxis.value);
						break;
					case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
						PushAxisDigital(slot, axis, nullptr, (prefix + "TriggerLeft").c_str(), event.caxis.value);
						break;
					case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
						PushAxisDigital(slot, axis, nullptr, (prefix + "TriggerRight").c_str(), event.caxis.value);
						break;
				}
				break;
			}
		}
	}

	void IOProcessor::Dispatch()
	{
		for (auto it = event_queue.begin(); it != event_queue.end(); ++it)
		{
			const IOEvent& ev = *it;

			for (auto dit = devices.begin(); dit != devices.end(); ++dit)
			{
				const IOConfigDevice& device = dit->device;

				for (auto bit = device.bindings.begin(); bit != device.bindings.end(); ++bit)
				{
					if (bit->binding == ev.name)
					{
						IOSetState(dit->handle, (size_t)bit->actuator_id, ev.value);
					}
				}
			}
		}
		event_queue.clear();
	}

	void IOProcessor::AttachDevicesToBoard(const char* board_name, const char* config_path)
	{
		DetachDevicesFromBoard();

		IOConfig config;
		if (!IOConfigManager::Load(config_path, config))
		{
			printf("IO: no IO settings at '%s' (run 'breaknes --ioconfig' to create them)\n", config_path);
			return;
		}

		devices.clear();

		for (auto dit = config.devices.begin(); dit != config.devices.end(); ++dit)
		{
			const IOConfigDevice& device = *dit;

			for (auto pit = device.attached.begin(); pit != device.attached.end(); ++pit)
			{
				const IOConfigPort& port = *pit;

				if (port.board != board_name)
					continue;

				AttachedDevice attached;
				attached.device = device;
				attached.port = port.port;
				attached.handle = IOCreateInstance(device.device_id);

				if (attached.handle == (size_t)-1)
				{
					std::string device_name = device.name.empty() ? IOConfigManager::DeviceIDToString(device.device_id) : device.name;
					printf("IO: warning: cannot create instance of '%s' (device_id 0x%08X), not supported by the core\n",
						device_name.c_str(), device.device_id);
					continue;
				}

				IOAttach((size_t)port.port, attached.handle);

				std::string device_name = device.name.empty() ? IOConfigManager::DeviceIDToString(device.device_id) : device.name;
				printf("IO: attached '%s' (handle %d) to port %d\n", device_name.c_str(), (int)attached.handle, port.port);

				devices.push_back(attached);
			}
		}

		event_queue.clear();
	}

	void IOProcessor::DetachDevicesFromBoard()
	{
		for (auto it = devices.begin(); it != devices.end(); ++it)
		{
			if (it->handle != (size_t)-1)
			{
				IODetach((size_t)it->port, it->handle);
				IODisposeInstance(it->handle);
			}
		}
		devices.clear();
		event_queue.clear();
	}

	bool IOProcessor::TakeFirstPress(IOEvent& out_event)
	{
		for (auto it = event_queue.begin(); it != event_queue.end(); ++it)
		{
			if (it->value != 0)
			{
				out_event = *it;
				event_queue.clear();
				return true;
			}
		}
		event_queue.clear();
		return false;
	}

	const char* IOProcessor::ButtonToString(SDL_GameControllerButton button)
	{
		switch (button)
		{
			case SDL_CONTROLLER_BUTTON_A: return "ButtonA";
			case SDL_CONTROLLER_BUTTON_B: return "ButtonB";
			case SDL_CONTROLLER_BUTTON_X: return "ButtonX";
			case SDL_CONTROLLER_BUTTON_Y: return "ButtonY";
			case SDL_CONTROLLER_BUTTON_BACK: return "ButtonBack";
			case SDL_CONTROLLER_BUTTON_GUIDE: return "ButtonGuide";
			case SDL_CONTROLLER_BUTTON_START: return "ButtonStart";
			case SDL_CONTROLLER_BUTTON_LEFTSTICK: return "ButtonLeftStick";
			case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return "ButtonRightStick";
			case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "ButtonLeftShoulder";
			case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return "ButtonRightShoulder";
			case SDL_CONTROLLER_BUTTON_DPAD_UP: return "DPadUp";
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return "DPadDown";
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return "DPadLeft";
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return "DPadRight";
		}
		return "ButtonUnknown";
	}
}
