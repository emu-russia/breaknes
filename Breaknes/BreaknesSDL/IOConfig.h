#pragma once

// IO settings of the SDL2 port (BreaknesSDL).
//
// The format repeats the config of the managed application (Breaknes/IOConfig.json),
// but the settings are stored in a separate file (IOConfigSDL.json by default) so
// that the two applications never overlap each other's settings (issue #516).

#include <string>
#include <vector>

namespace BreaknesSDL
{
	/// <summary>
	/// Default name of the IO settings file of the SDL2 port.
	/// The managed application uses its own "IOConfig.json", so the settings never overlap.
	/// </summary>
	const char* const kDefaultIOConfigPath = "IOConfigSDL.json";

	/// <summary>
	/// The motherboard that the SDL2 port always creates (hardcoded in main.cpp).
	/// </summary>
	const char* const kSDLBoardName = "NESBoard";
	const char* const kSDLBoardAPU = "RP2A03G";
	const char* const kSDLBoardPPU = "RP2C02G";
	const char* const kSDLBoardP1 = "NES";

	/// <summary>
	/// Associative binding of a device IOState (actuator) to an input event of the PC.
	/// The binding string is the name of the SDL input event, e.g. "Up",
	/// "Controller0_ButtonA", "Controller0_DPadUp", "Controller0_AxisLeftX-".
	/// </summary>
	struct IOConfigBinding
	{
		int actuator_id = 0;
		std::string binding;
	};

	/// <summary>
	/// Attach of the device to a motherboard port.
	/// </summary>
	struct IOConfigPort
	{
		std::string board;
		int port = -1;
	};

	/// <summary>
	/// A registered device of the device pool (added/removed by the user, see IO/IO.h
	/// for the list of DeviceID).
	/// </summary>
	struct IOConfigDevice
	{
		uint32_t device_id = 0;
		std::string name;
		std::vector<IOConfigPort> attached;
		std::vector<IOConfigBinding> bindings;
	};

	/// <summary>
	/// The whole IO settings document.
	/// </summary>
	struct IOConfig
	{
		std::string descr;
		std::vector<IOConfigDevice> devices;
	};

	/// <summary>
	/// Load/save of the IO settings and helper methods.
	/// </summary>
	class IOConfigManager
	{
	public:
		/// <summary>
		/// Load the IO settings from the JSON file.
		/// </summary>
		/// <returns>false if the file is missing or cannot be parsed.</returns>
		static bool Load(const char* path, IOConfig& config);

		/// <summary>
		/// Save the IO settings to the JSON file.
		/// </summary>
		static bool Save(const IOConfig& config, const char* path);

		/// <summary>
		/// Human-readable name of the device type by DeviceID (see IO/IO.h).
		/// </summary>
		static std::string DeviceIDToString(uint32_t device_id);

		/// <summary>
		/// Default device name used when a device is added without a user-specified name.
		/// </summary>
		static std::string GetDefaultDeviceName(uint32_t device_id);
	};
}
