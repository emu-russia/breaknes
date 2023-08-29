using SharpDX.DirectInput;

// Demo:
// https://github.com/sharpdx/SharpDX-Samples/blob/master/Desktop/DirectInput/JoystickApp/Program.cs

namespace Breaknes
{

	public class IOProcessor
	{
		Keyboard? keyboard = null;
		Joystick? joystick = null;

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

	}

}
