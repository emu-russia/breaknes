using SharpDX.DirectInput;
using SharpTools;

// Demo:
// https://github.com/sharpdx/SharpDX-Samples/blob/master/Desktop/DirectInput/JoystickApp/Program.cs

namespace Breaknes
{

	/// <summary>
	/// Associates the connected device from the configuration with the actual connection in BreaksCore by handle.
	/// After receiving the IOEvent, all devices take turns receiving the list to filter and process it and send the SetState for BreaksCore by handle.
	/// </summary>
	public class AttachedDevice
	{
		public int handle = -1;
		public int port = -1;
		public IOConfigDevice device = new();
		public object? opaque = null;
	}

	/// <summary>
	/// A typical descriptor of some kind of I/O event. Events are registered in a queue and then processed by attached devices in a chain. Classics.
	/// </summary>
	public class IOEvent
	{
		public string name = "";
		public UInt32 value = 0;
		public int device_handle = -1;	// -1: broadcast
	}

	public class IOProcessor
	{
		Keyboard? keyboard = null;
		Joystick? joystick = null;
		List<AttachedDevice> devices = new();
		// Made public so that the bindings dialog can poll input devices by getting events from the queue
		public List<IOEvent> event_queue = new();

		public IOProcessor()
		{
			var directInput = new DirectInput();

			// Find keyboard
			keyboard = new Keyboard(directInput);
			keyboard.Properties.BufferSize = 128;
			keyboard.Acquire();

			// Find a Joystick Guid
			var joystickGuid = Guid.Empty;

			foreach (var deviceInstance in directInput.GetDevices(DeviceType.Gamepad, DeviceEnumerationFlags.AllDevices))
				joystickGuid = deviceInstance.InstanceGuid;

			// If Gamepad not found, look for a Joystick
			if (joystickGuid == Guid.Empty)
				foreach (var deviceInstance in directInput.GetDevices(DeviceType.Joystick, DeviceEnumerationFlags.AllDevices))
					joystickGuid = deviceInstance.InstanceGuid;

			// If Joystick not found, throws an error
			if (joystickGuid == Guid.Empty)
			{
				return;
			}

			// Instantiate the joystick
			joystick = new Joystick(directInput, joystickGuid);

			Console.WriteLine("Found Joystick/Gamepad with GUID: {0}", joystickGuid);

			// Query all suported ForceFeedback effects
			var allEffects = joystick.GetEffects();
			foreach (var effectInfo in allEffects)
				Console.WriteLine("Effect available {0}", effectInfo.Name);

			// Set BufferSize in order to use buffered data.
			joystick.Properties.BufferSize = 128;

			// Acquire the joystick
			joystick.Acquire();
		}

		/// <summary>
		/// Used to poll real devices, using some platform API to get their state.
		/// Call often enough, but not infrequently.
		/// </summary>
		public void PollDevices()
		{
			List<IOEvent> events = new();

			// Translate keyboard events

			if (keyboard != null)
			{
				keyboard.Poll();
				var datas = keyboard.GetBufferedData();
				foreach (var state in datas)
				{
					IOEvent key_event = new();
					key_event.name = state.Key.ToString();
					key_event.value = (uint)(state.IsPressed ? 1 : 0);
					key_event.device_handle = -1;
					events.Add(key_event);
				}
			}

			// Translate events from joystick (TODO: only one is supported so far)

			if (joystick != null)
			{
				joystick.Poll();
				var datas = joystick.GetBufferedData();
				foreach (var state in datas)
				{
					Console.WriteLine(state);

					IOEvent joy_event = new();

					string offset = state.Offset.ToString();

					if (offset.StartsWith("Button"))
					{
						joy_event.name = offset;
						joy_event.value = (uint)(state.Value != 0 ? 1 : 0);
						joy_event.device_handle = -1;
						events.Add(joy_event);
					}
					else if (offset.StartsWith("PointOfViewControllers"))
					{
						switch (state.Value)
						{
							// I don't know who came up with such a "brilliant" idea to make the D-Pad in such a way, but as it is. Pushback will be considered as pushback in all directions
							case -1:
								IOEvent joy_event1 = new();
								joy_event1.name = offset + "_Up";
								joy_event1.value = 0;
								joy_event1.device_handle = -1;
								events.Add(joy_event1);
								IOEvent joy_event2 = new();
								joy_event2.name = offset + "_Right";
								joy_event2.value = 0;
								joy_event2.device_handle = -1;
								events.Add(joy_event2);
								IOEvent joy_event3 = new();
								joy_event3.name = offset + "_Down";
								joy_event3.value = 0;
								joy_event3.device_handle = -1;
								events.Add(joy_event3);
								IOEvent joy_event4 = new();
								joy_event4.name = offset + "_Left";
								joy_event4.value = 0;
								joy_event4.device_handle = -1;
								events.Add(joy_event4);
								break;
							case 0:
								joy_event.name = offset + "_Up";
								joy_event.value = 1;
								joy_event.device_handle = -1;
								events.Add(joy_event);
								break;
							case 9000:
								joy_event.name = offset + "_Right";
								joy_event.value = 1;
								joy_event.device_handle = -1;
								events.Add(joy_event);
								break;
							case 18000:
								joy_event.name = offset + "_Down";
								joy_event.value = 1;
								joy_event.device_handle = -1;
								events.Add(joy_event);
								break;
							case 27000:
								joy_event.name = offset + "_Left";
								joy_event.value = 1;
								joy_event.device_handle = -1;
								events.Add(joy_event);
								break;
						}
					}
				}

				foreach (var io_event in events)
				{
					DumpEvent(io_event);
				}
			}

			event_queue.AddRange(events);
		}

		/// <summary>
		/// Process the current IO event queue and send all devices SetState in BreaksCore.
		/// </summary>
		public void DispatchEventQueue()
		{
			foreach (var io_event in event_queue)
			{
				foreach (var attached_device in devices)
				{
					foreach (var binding in attached_device.device.bindings)
					{
						if (binding.binding == io_event.name)
						{
							if (io_event.device_handle >= 0)
							{
								if (attached_device.handle == io_event.device_handle)
								{
									BreaksCore.IOSetState(attached_device.handle, binding.actuator_id, io_event.value);
								}
							}
							else
							{
								BreaksCore.IOSetState(attached_device.handle, binding.actuator_id, io_event.value);
							}
						}
					}
				}
			}
			event_queue.Clear();
		}

		public void AttachDevicesToBoard(string board_name)
		{
			IOConfig config = IOConfigManager.LoadIOConfig();

			devices = new();

			foreach (var device in config.devices)
			{
				foreach (var port in device.attached)
				{
					if (port.board != board_name)
						continue;
					int port_num = port.port;

					AttachedDevice attached_device = new();
					attached_device.device = device;
					attached_device.port = port_num;
					attached_device.handle = BreaksCore.IOCreateInstance(device.device_id);
					BreaksCore.IOAttach(port_num, attached_device.handle);

					// If a virtual device is connected, a modeless dialog must be created

					switch (device.device_id)
					{
						case 0x00010001:
							FormVirtualFamiController1 virt_fami1 = new FormVirtualFamiController1(device, attached_device.handle, this);
							virt_fami1.Show();
							attached_device.opaque = virt_fami1;
							break;
						case 0x00010002:
							FormVirtualFamiController2 virt_fami2 = new FormVirtualFamiController2(device, attached_device.handle, this);
							virt_fami2.Show();
							attached_device.opaque = virt_fami2;
							break;
						case 0x00010003:
							FormVirtualNESController virt_nes = new FormVirtualNESController(device, attached_device.handle, this);
							virt_nes.Show();
							attached_device.opaque = virt_nes;
							break;
						case 0x00010004:
							FormVirtualDendyController virt_dendy = new FormVirtualDendyController(device, attached_device.handle, this);
							virt_dendy.Show();
							attached_device.opaque = virt_dendy;
							break;
					}

					devices.Add(attached_device);
				}
			}

			event_queue.Clear();
		}

		public void DetachDevicesFromBoard()
		{
			foreach (var device in devices)
			{
				if (device.handle >= 0)
				{
					// Close modeless dialogs for virtual devices

					if (device.opaque != null)
					{
						switch (device.device.device_id)
						{
							case 0x00010001:
								((FormVirtualFamiController1)device.opaque).Close();
								break;
							case 0x00010002:
								((FormVirtualFamiController2)device.opaque).Close();
								break;
							case 0x00010003:
								((FormVirtualNESController)device.opaque).Close();
								break;
							case 0x00010004:
								((FormVirtualDendyController)device.opaque).Close();
								break;
						}
					}

					BreaksCore.IODetach(device.port, device.handle);
					BreaksCore.IODisposeInstance(device.handle);
				}
			}
			devices.Clear();
		}

		public void PushEvent(IOEvent io_event)
		{
			DumpEvent(io_event);
			event_queue.Add(io_event);
		}

		public void DumpEvent(IOEvent io_event)
		{
			string text = "";
			text += "IOEvent: " + io_event.name + ", value: " + io_event.value.ToString();
			Console.WriteLine(text);
		}
	}
}
