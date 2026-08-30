#pragma once

// SDL2 input processor of the SDL2 port (BreaknesSDL).
//
// The architecture mirrors the managed IOProcessor (Breaknes/IOProcessing.cs):
// real input devices (keyboard, game controllers) are polled via the SDL2 Input
// API and turned into events; each attached device then translates the events
// it is bound to into SetState calls in BreaksCore (issue #516).

#include <string>
#include <vector>
#include <map>

#include "IOConfig.h"

#ifdef _WIN32
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

namespace BreaknesSDL
{
	/// <summary>
	/// The maximum number of simultaneously opened game controllers.
	/// Event names refer to the controller slot: "Controller0_ButtonA", ...
	/// </summary>
	const int kMaxControllers = 4;

	/// <summary>
	/// One input event, in the spirit of the managed IOEvent.
	/// The name is the string that the bindings refer to: SDL key names
	/// ("Up", "Down", "Left", "Right", "A", "S", "Z", "X", "Return", ...) or
	/// game controller events ("Controller0_ButtonA", "Controller0_DPadUp",
	/// "Controller0_AxisLeftX-", "Controller0_TriggerLeft", ...).
	/// </summary>
	struct IOEvent
	{
		std::string name;
		uint32_t value = 0;	// 0/1
	};

	/// <summary>
	/// A device instance created in BreaksCore and attached to a motherboard port.
	/// </summary>
	struct AttachedDevice
	{
		size_t handle = (size_t)-1;
		int port = -1;
		IOConfigDevice device;
	};

	/// <summary>
	/// One open SDL game controller (a slot).
	/// </summary>
	struct SDLController
	{
		SDL_GameController* gamepad = nullptr;
		SDL_JoystickID instance_id = 0;

		// Last digital state of the axes, to emit events only on transitions.
		// axis_digital[axis][0] = negative direction ("-"), [1] = positive ("+").
		// Set to -1 ("unknown") when the controller is opened so that the first
		// axis motion always emits.
		int axis_digital[SDL_CONTROLLER_AXIS_MAX][2] = {};
	};

	/// <summary>
	/// SDL2 input processor: turns SDL input events into IOEvents and forwards them
	/// to the devices attached to the BreaksCore motherboard via the bindings.
	/// </summary>
	class IOProcessor
	{
		std::vector<SDLController> controllers;				// fixed-size slots (kMaxControllers)
		std::map<SDL_JoystickID, int> controller_slot;		// joystick instance id -> slot

		std::vector<AttachedDevice> devices;
		std::vector<IOEvent> event_queue;

		int FindFreeControllerSlot();
		int GetControllerSlot(SDL_JoystickID instance_id);
		void OnControllerAdded(int device_index);
		void OnControllerRemoved(SDL_JoystickID instance_id);

		void PushEvent(const char* name, uint32_t value);
		void PushAxisDigital(int slot, SDL_GameControllerAxis axis, const char* event_name_neg, const char* event_name_pos, int16_t value);

	public:
		IOProcessor();
		~IOProcessor();

		/// <summary>
		/// Feed an SDL event to the processor (called from the main event loop).
		/// </summary>
		void ProcessEvent(const SDL_Event& event);

		/// <summary>
		/// Apply the accumulated events to the attached devices (SetState in BreaksCore).
		/// </summary>
		void Dispatch();

		/// <summary>
		/// Create device instances from the config file and attach them to the board ports.
		/// </summary>
		void AttachDevicesToBoard(const char* board_name, const char* config_path);

		/// <summary>
		/// Detach and dispose all device instances.
		/// </summary>
		void DetachDevicesFromBoard();

		/// <summary>
		/// Take the first pending event with value != 0 (a press). The rest of the
		/// queue is discarded. Used by the configurator to grab a key/button for a binding.
		/// </summary>
		bool TakeFirstPress(IOEvent& out_event);

		/// <summary>
		/// Name of a game controller button, e.g. "ButtonA", "DPadUp".
		/// </summary>
		static const char* ButtonToString(SDL_GameControllerButton button);
	};
}
