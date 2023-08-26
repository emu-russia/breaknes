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

	public class IOConfigDevice
	{
		public UInt32 device_id = 0;
		public string name = "";
		public string[] attached = Array.Empty<string>();
		public IOConfigBinding[] bindings = Array.Empty<IOConfigBinding>();
	}

	public class IOConfigBinding
	{
		public string actuator = "";
		public string binding = "";
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
			string text = JsonConvert.SerializeObject(config, Formatting.Indented);
			File.WriteAllText("IOConfig.json", text, Encoding.UTF8);
		}
	}
}
