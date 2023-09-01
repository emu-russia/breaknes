using System.Text;
using Newtonsoft.Json;

namespace Breaknes
{
	public class IOConfig
	{
		public IOConfigInfo info = new IOConfigInfo();
		public IOConfigDevice[] devices = Array.Empty<IOConfigDevice>();
	}

	public class IOConfigInfo
	{
		public string descr = "This JSON contains a list of IO devices and their Attach/Detach statuses with Motherboard and Bindings with Input API.";
	}

	public class IOConfigPort
	{
		public string board = "";
		public int port = -1;
	}

	public class IOConfigDevice
	{
		public UInt32 device_id = 0;
		public string name = "";
		public IOConfigPort[] attached = Array.Empty<IOConfigPort>();
		public IOConfigBinding[] bindings = Array.Empty<IOConfigBinding>();
	}

	public class IOConfigBinding
	{
		public int actuator_id = 0;
		public string binding = "";
		public IOConfigBinding(int actuator_id, string binding)
		{
			this.actuator_id = actuator_id;
			this.binding = binding;
		}
		public IOConfigBinding() { }
	}

	public class IOConfigManager
	{
		static public IOConfig LoadIOConfig()
		{
			string json = File.ReadAllText("IOConfig.json", Encoding.UTF8);
			var config = JsonConvert.DeserializeObject<IOConfig>(json);
			if (config == null)
			{
				config = new IOConfig();
			}
			return config;
		}

		static public void SaveIOConfig(IOConfig config)
		{
			string text = SerializeIOConfig(config);
			File.WriteAllText("IOConfig.json", text, Encoding.UTF8);
		}

		static public string SerializeIOConfig(IOConfig config)
		{
			return JsonConvert.SerializeObject(config, Formatting.Indented);
		}

		static public string DeviceIDToString(UInt32 device_id)
		{
			switch (device_id)
			{
				case 0x00000001: return "Famicom Controller (Port1)";
				case 0x00000002: return "Famicom Controller (Port2)";
				case 0x00000003: return "NES Controller";
				case 0x00000004: return "Dendy Turbo Controller";
				case 0x00010001: return "Virtual Famicom Controller (Port1)";
				case 0x00010002: return "Virtual Famicom Controller (Port2)";
				case 0x00010003: return "Virtual NES Controller";
				case 0x00010004: return "Virtual Dendy Turbo Controller";
			}
			return device_id.ToString();
		}
	}
}
