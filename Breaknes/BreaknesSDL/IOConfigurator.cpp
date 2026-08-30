#include "pch.h"

#include "IOConfig.h"
#include "IOProcessing.h"
#include "IOConfigurator.h"

#include "../../IO/IO.h"

namespace BreaknesSDL
{
	namespace
	{
		/// <summary>
		/// The device types offered for adding, in the order of the managed
		/// FormIOAddDevice combo (the DeviceID list is in IO/IO.h).
		/// </summary>
		static const IO::DeviceID kDeviceTypes[] =
		{
			IO::DeviceID::FamiController_1,			// Famicom Controller (Port1)
			IO::DeviceID::FamiController_2,			// Famicom Controller (Port2)
			IO::DeviceID::NESController,			// NES Controller
			IO::DeviceID::DendyController,			// Dendy Turbo Controller
			IO::DeviceID::VirtualFamiController_1,	// Virtual Famicom Controller (Port1)
			IO::DeviceID::VirtualFamiController_2,	// Virtual Famicom Controller (Port2)
			IO::DeviceID::VirtualNESController,		// Virtual NES Controller
			IO::DeviceID::VirtualDendyController,	// Virtual Dendy Turbo Controller
		};
		const int kNumDeviceTypes = (int)(sizeof(kDeviceTypes) / sizeof(kDeviceTypes[0]));

		/// <summary>
		/// The ports of the NES-type motherboard created by the SDL2 port (the default
		/// board of the BoardDescription.json, see kSDLBoardName).
		/// Mirrors the core: Breaknes/BreaksCore/NESBoard.cpp, NESBoardIO.
		/// </summary>
		struct PortDesc
		{
			int port;
			const char* name;
		};
		const PortDesc kPorts[] =
		{
			{ 0, "NES Controller Port1" },
			{ 1, "NES Controller Port2" },
		};
		const int kNumPorts = (int)(sizeof(kPorts) / sizeof(kPorts[0]));

		/// <summary>
		/// Which device types the NES-type board ports accept (DeviceID), see NESBoardIO::GetPortSupportedDevices.
		/// </summary>
		static bool IsDeviceSupportedByBoard(uint32_t device_id)
		{
			return device_id == (uint32_t)IO::DeviceID::NESController ||
				device_id == (uint32_t)IO::DeviceID::VirtualNESController;
		}

		/// <summary>
		/// Read one trimmed line from stdin. Returns false on EOF.
		/// </summary>
		static bool ReadLine(char* buf, size_t buf_size)
		{
			if (fgets(buf, (int)buf_size, stdin) == nullptr)
				return false;

			size_t len = strlen(buf);
			while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
				buf[--len] = 0;

			return true;
		}

		/// <summary>
		/// Parse a decimal integer. Returns false on failure.
		/// </summary>
		static bool ParseInt(const char* str, int& out)
		{
			if (str == nullptr || str[0] == 0)
				return false;

			char* end = nullptr;
			long value = strtol(str, &end, 10);
			if (end == str)
				return false;

			out = (int)value;
			return true;
		}

		static std::string BindingsToString(const IOConfigDevice& device, int actuator_id)
		{
			std::string out;
			bool first = true;

			for (auto it = device.bindings.begin(); it != device.bindings.end(); ++it)
			{
				if (it->actuator_id == actuator_id)
				{
					if (!first)
						out += ", ";
					out += it->binding;
					first = false;
				}
			}

			if (first)
				out = "(none)";

			return out;
		}

		static bool IsAttachedToPort(const IOConfigDevice& device, int port, const char* board_name)
		{
			for (auto it = device.attached.begin(); it != device.attached.end(); ++it)
			{
				if (it->board == board_name && it->port == port)
					return true;
			}
			return false;
		}

		static void RemovePort(IOConfigDevice& device, int port, const char* board_name)
		{
			std::vector<IOConfigPort> remaining;
			for (auto it = device.attached.begin(); it != device.attached.end(); ++it)
			{
				if (!(it->board == board_name && it->port == port))
					remaining.push_back(*it);
			}
			device.attached = remaining;
		}

		static void PrintHelp()
		{
			printf("\nCommands:\n");
			printf("  h           - this help\n");
			printf("  l           - list the device pool\n");
			printf("  a           - add a device\n");
			printf("  d <index>   - remove a device\n");
			printf("  b <index>   - bind the actuators of a device to input events\n");
			printf("  p <index>   - attach/detach a device to the motherboard ports\n");
			printf("  s           - save the settings to the config file\n");
			printf("  q           - quit\n");
			printf("\nInput events that can be bound:\n");
			printf("  keyboard    - SDL key names: Up, Down, Left, Right, A, S, Z, X, Return, Space, ...\n");
			printf("  gamepad     - Controller<slot>_ButtonA, _DPadUp, _ButtonStart, ...\n");
			printf("              - Controller<slot>_AxisLeftX- / _AxisLeftX+ (stick directions)\n");
			printf("              - Controller<slot>_TriggerLeft / _TriggerRight\n");
		}

		static void ListDevices(const IOConfig& config)
		{
			printf("\nDevice pool:\n");

			if (config.devices.empty())
			{
				printf("  (empty)\n");
			}

			for (size_t i = 0; i < config.devices.size(); i++)
			{
				const IOConfigDevice& device = config.devices[i];

				printf("  [%d] %s \"%s\"\n",
					(int)i,
					IOConfigManager::DeviceIDToString(device.device_id).c_str(),
					device.name.c_str());

				printf("       attached: ");
				if (device.attached.empty())
				{
					printf("(none)\n");
				}
				else
				{
					bool first = true;
					for (auto it = device.attached.begin(); it != device.attached.end(); ++it)
					{
						printf("%s%s[%d]", first ? "" : ", ", it->board.c_str(), it->port);
						first = false;
					}
					printf("\n");
				}

				printf("       bindings: %d\n", (int)device.bindings.size());
			}
			printf("\n");
		}

		static bool AddDevice(IOConfig& config)
		{
			printf("\nDevice types:\n");
			for (int i = 0; i < kNumDeviceTypes; i++)
			{
				printf("  [%d] %s\n", i, IOConfigManager::DeviceIDToString((uint32_t)kDeviceTypes[i]).c_str());
			}

			printf("Select device type: ");
			fflush(stdout);

			char line[0x100];
			if (!ReadLine(line, sizeof(line)))
				return false;

			int type_index = -1;
			if (!ParseInt(line, type_index) || type_index < 0 || type_index >= kNumDeviceTypes)
			{
				printf("Invalid device type.\n");
				return false;
			}

			uint32_t device_id = (uint32_t)kDeviceTypes[type_index];

			printf("Device name (default: %s): ", IOConfigManager::GetDefaultDeviceName(device_id).c_str());
			fflush(stdout);

			std::string name;
			char name_line[0x100];
			if (ReadLine(name_line, sizeof(name_line)) && name_line[0] != 0)
			{
				name = name_line;
			}
			if (name.empty())
			{
				name = IOConfigManager::GetDefaultDeviceName(device_id);
			}

			IOConfigDevice device;
			device.device_id = device_id;
			device.name = name;
			config.devices.push_back(device);

			printf("Added: %s \"%s\"\n",
				IOConfigManager::DeviceIDToString(device_id).c_str(), name.c_str());
			printf("Now bind the actuators ('b %d') and attach to a port ('p %d').\n",
				(int)config.devices.size() - 1, (int)config.devices.size() - 1);

			if (device_id == (uint32_t)IO::DeviceID::DendyController || device_id == (uint32_t)IO::DeviceID::VirtualDendyController)
			{
				printf("Note: the Dendy Turbo Controller is not implemented in BreaksCore yet.\n");
			}

			return true;
		}

		static bool RemoveDevice(IOConfig& config, int index)
		{
			if (index < 0 || index >= (int)config.devices.size())
			{
				printf("Invalid index.\n");
				return false;
			}

			printf("Removed: %s \"%s\"\n",
				IOConfigManager::DeviceIDToString(config.devices[index].device_id).c_str(),
				config.devices[index].name.c_str());

			config.devices.erase(config.devices.begin() + index);
			return true;
		}

		/// <summary>
		/// Wait for one input event (a press) and return its name.
		/// ESC cancels. The analogue of the managed FormWaitInputEvent.
		/// </summary>
		static bool WaitForInputEvent(IOProcessor& io, std::string& event_name)
		{
			printf("Press a key or a controller button... (ESC to cancel)\n");
			fflush(stdout);

			bool cancel = false;

			while (!cancel)
			{
				SDL_Delay(10);

				SDL_Event event;
				while (SDL_PollEvent(&event))
				{
					if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
					{
						cancel = true;
						break;
					}
					io.ProcessEvent(event);
				}

				if (cancel)
					break;

				IOEvent ev;
				if (io.TakeFirstPress(ev))
				{
					event_name = ev.name;
					printf("Got: %s\n", event_name.c_str());
					return true;
				}
			}

			printf("Cancelled.\n");
			return false;
		}

		/// <summary>
		/// Bind one actuator of a device to an input event.
		/// </summary>
		static bool BindActuator(IOProcessor& io, IOConfigDevice& device, int actuator_id, const std::string& actuator_name)
		{
			printf("\nActuator '%s' (id %d), current binding: %s\n",
				actuator_name.c_str(), actuator_id, BindingsToString(device, actuator_id).c_str());

			std::string event_name;
			if (!WaitForInputEvent(io, event_name))
				return false;

			// Assign the binding: replace the existing binding of the actuator, if any
			// (like the managed AssignBinding).
			bool found = false;
			for (auto it = device.bindings.begin(); it != device.bindings.end(); ++it)
			{
				if (it->actuator_id == actuator_id)
				{
					it->binding = event_name;
					found = true;
					break;
				}
			}

			if (!found)
			{
				IOConfigBinding binding;
				binding.actuator_id = actuator_id;
				binding.binding = event_name;
				device.bindings.push_back(binding);
			}

			printf("Bound '%s' to %s.\n", actuator_name.c_str(), event_name.c_str());
			return true;
		}

		static bool BindDevice(IOProcessor& io, IOConfig& config, int index)
		{
			if (index < 0 || index >= (int)config.devices.size())
			{
				printf("Invalid index.\n");
				return false;
			}

			IOConfigDevice& device = config.devices[index];

			// Enumerate the actuators of the device type via BreaksCore
			// (the analogue of PopulateActuatorsList in the managed binding dialogs).

			std::vector<std::string> actuator_names;

			size_t handle = IOCreateInstance(device.device_id);
			if (handle == (size_t)-1)
			{
				printf("The device type '%s' is not implemented in BreaksCore, nothing to bind.\n",
					IOConfigManager::DeviceIDToString(device.device_id).c_str());
				return false;
			}

			size_t num_states = IOGetNumStates(handle);
			for (size_t i = 0; i < num_states; i++)
			{
				char name_buf[0x100] = {};
				IOGetStateName(handle, i, name_buf, sizeof(name_buf));
				actuator_names.push_back(name_buf);
			}
			IODisposeInstance(handle);

			bool changed = false;
			bool back = false;

			while (!back)
			{
				printf("\nDevice [%d] %s \"%s\"\n",
					index,
					IOConfigManager::DeviceIDToString(device.device_id).c_str(),
					device.name.c_str());

				for (size_t i = 0; i < actuator_names.size(); i++)
				{
					printf("  [%d] %-12s binding: %s\n",
						(int)i, actuator_names[i].c_str(), BindingsToString(device, (int)i).c_str());
				}

				printf("Commands: <actuator index> - bind, c - clear all bindings, q - back\n");
				printf("> ");
				fflush(stdout);

				char line[0x100];
				if (!ReadLine(line, sizeof(line)))
					break;

				char cmd = line[0];

				if (cmd == 'q')
				{
					back = true;
				}
				else if (cmd == 'c')
				{
					device.bindings.clear();
					changed = true;
					printf("All bindings of the device cleared.\n");
				}
				else
				{
					int actuator = -1;
					if (ParseInt(line, actuator) && actuator >= 0 && actuator < (int)actuator_names.size())
					{
						if (BindActuator(io, device, actuator, actuator_names[actuator]))
						{
							changed = true;
						}
					}
					else
					{
						printf("Unknown command.\n");
					}
				}
			}

			return changed;
		}

		static bool AttachDevice(IOConfig& config, int index, const char* board_name)
		{
			if (index < 0 || index >= (int)config.devices.size())
			{
				printf("Invalid index.\n");
				return false;
			}

			IOConfigDevice& device = config.devices[index];

			printf("\nDevice [%d] %s \"%s\"\n",
				index,
				IOConfigManager::DeviceIDToString(device.device_id).c_str(),
				device.name.c_str());

			if (!IsDeviceSupportedByBoard(device.device_id))
			{
				printf("The %s cannot be attached to the %s: only NES Controller and Virtual NES Controller are supported by its ports.\n",
					IOConfigManager::DeviceIDToString(device.device_id).c_str(), board_name);
				return false;
			}

			bool changed = false;
			bool back = false;

			while (!back)
			{
				printf("\nBoard: %s\n", board_name);

				for (int i = 0; i < kNumPorts; i++)
				{
					bool attached = IsAttachedToPort(device, kPorts[i].port, board_name);
					printf("  [%d] %-22s attached: %s\n", kPorts[i].port, kPorts[i].name, attached ? "yes" : "no");
				}

				printf("Commands: <port> - toggle attach, q - back\n");
				printf("> ");
				fflush(stdout);

				char line[0x100];
				if (!ReadLine(line, sizeof(line)))
					break;

				char cmd = line[0];

				if (cmd == 'q')
				{
					back = true;
				}
				else
				{
					int port = -1;
					if (ParseInt(line, port))
					{
						bool valid = false;
						for (int i = 0; i < kNumPorts; i++)
						{
							if (kPorts[i].port == port)
								valid = true;
						}

						if (!valid)
						{
							printf("Invalid port.\n");
							continue;
						}

						if (IsAttachedToPort(device, port, board_name))
						{
							RemovePort(device, port, board_name);
							printf("Detached from port %d.\n", port);
						}
						else
						{
							IOConfigPort port_entry;
							port_entry.board = board_name;
							port_entry.port = port;
							device.attached.push_back(port_entry);
							printf("Attached to port %d.\n", port);
						}
						changed = true;
					}
					else
					{
						printf("Unknown command.\n");
					}
				}
			}

			return changed;
		}

		static bool SaveConfig(const IOConfig& config, const char* path)
		{
			if (IOConfigManager::Save(config, path))
			{
				printf("Saved to '%s'.\n", path);
				return true;
			}
			printf("Failed to save to '%s'!\n", path);
			return false;
		}
	}

	void RunIOConfigurator(const char* config_path, const char* board_name)
	{
		printf("\n=== BreaknesSDL IO Configurator ===\n\n");

		IOConfig config;
		if (IOConfigManager::Load(config_path, config))
		{
			printf("Loaded existing settings from '%s'.\n", config_path);
		}
		else
		{
			config = IOConfig();
			printf("No settings found at '%s', starting with an empty device pool.\n", config_path);
		}

		// The processor is used to grab input events (keyboard/game controller)
		// while binding actuators to them.
		IOProcessor io;

		PrintHelp();

		bool modified = false;
		bool quit = false;

		while (!quit)
		{
			ListDevices(config);

			printf("> ");
			fflush(stdout);

			char line[0x100];
			if (!ReadLine(line, sizeof(line)))
			{
				printf("\n");
				quit = true;
				break;
			}

			char cmd = line[0];
			int index = -1;
			if (line[0] != 0 && line[1] == ' ')
			{
				ParseInt(line + 2, index);
			}

			switch (cmd)
			{
				case 'h':
					PrintHelp();
					break;

				case 'l':
					break;	// the list is printed at the top of the loop

				case 'a':
					if (AddDevice(config))
						modified = true;
					break;

				case 'd':
					if (index < 0)
					{
						printf("Use: d <index>\n");
					}
					else if (RemoveDevice(config, index))
					{
						modified = true;
					}
					break;

				case 'b':
					if (index < 0)
					{
						printf("Use: b <index>\n");
					}
					else if (BindDevice(io, config, index))
					{
						modified = true;
					}
					break;

				case 'p':
					if (index < 0)
					{
						printf("Use: p <index>\n");
					}
					else if (AttachDevice(config, index, board_name))
					{
						modified = true;
					}
					break;

				case 's':
					if (SaveConfig(config, config_path))
					{
						modified = false;
					}
					break;

				case 'q':
					if (modified)
					{
						printf("Save changes to '%s'? (y/n): ", config_path);
						fflush(stdout);

						char answer[0x100];
						if (ReadLine(answer, sizeof(answer)) && (answer[0] == 'y' || answer[0] == 'Y'))
						{
							SaveConfig(config, config_path);
						}
					}
					quit = true;
					break;

				default:
					printf("Unknown command. 'h' for help.\n");
					break;
			}
		}
	}
}
