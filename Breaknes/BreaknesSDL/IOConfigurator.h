#pragma once

namespace BreaknesSDL
{
	/// <summary>
	/// Runs the interactive IO configurator in the console.
	/// The logic is based on the IO settings dialog of the managed application
	/// (Breaknes/FormIOConfig.cs): the device pool (add/remove devices), the
	/// bindings (IOState <-> input event) and the Attach/Detach to motherboard ports.
	/// Requires a created motherboard (for the actuator enumeration via BreaksCore).
	/// </summary>
	/// <param name="config_path">Path of the IO settings file (IOConfigSDL.json by default).</param>
	void RunIOConfigurator(const char* config_path);
}
