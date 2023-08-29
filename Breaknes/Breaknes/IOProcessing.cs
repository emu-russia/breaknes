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
		public IOConfigDevice device = new();
		public object? opaque = null;
	}

	/// <summary>
	/// A typical descriptor of some kind of I/O event. Events are registered in a queue and then processed by attached devices in a chain. Classics.
	/// </summary>
	public class IOEvent
	{
		public string name = "";
		public int actuator_id = -1;
		public UInt32 value = 0;
	}

	public class IOProcessor
	{
		Keyboard? keyboard = null;
		Joystick? joystick = null;
		List<AttachedDevice> devices = new();
		List<IOEvent> event_queue = new();

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

		public void PollDevices()
		{
			if (keyboard != null)
			{
				keyboard.Poll();
				var datas = keyboard.GetBufferedData();
				foreach (var state in datas)
					Console.WriteLine(state);
			}

			if (joystick != null)
			{
				joystick.Poll();
				var datas = joystick.GetBufferedData();
				foreach (var state in datas)
					Console.WriteLine(state);
			}
		}

		public void AttachDevicesToBoard(string board_name)
		{
			IOConfig config = IOConfigManager.LoadIOConfig();

			devices = new();

			foreach (var device in config.devices)
			{
				if (device.attached.Contains(board_name))
				{
					AttachedDevice attached_device = new();
					attached_device.device = device;
					attached_device.handle = BreaksCore.IOCreateInstance(device.device_id);

					// If a virtual device is connected, a modeless dialog must be created

					switch (device.device_id)
					{
						case 0x00010001:
							FormVirtualFamiController1 virt_fami1 = new FormVirtualFamiController1(device, this);
							virt_fami1.Show();
							attached_device.opaque = virt_fami1;
							break;
						case 0x00010002:
							FormVirtualFamiController2 virt_fami2 = new FormVirtualFamiController2(device, this);
							virt_fami2.Show();
							attached_device.opaque = virt_fami2;
							break;
						case 0x00010003:
							FormVirtualNESController virt_nes = new FormVirtualNESController(device, this);
							virt_nes.Show();
							attached_device.opaque = virt_nes;
							break;
						case 0x00010004:
							FormVirtualDendyController virt_dendy = new FormVirtualDendyController(device, this);
							virt_dendy.Show();
							attached_device.opaque = virt_dendy;
							break;
					}

					devices.Add(attached_device);
				}
			}
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

					BreaksCore.IODisposeInstance(device.handle);
				}
			}
			devices.Clear();
		}

		public void PushEvent(IOEvent io_event)
		{

		}

	}

}
