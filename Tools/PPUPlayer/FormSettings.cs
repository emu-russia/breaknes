using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;

using System.Xml.Serialization;
using System.IO;
using SharpTools;

namespace PPUPlayer
{
	public partial class FormSettings : Form
	{
		public FormSettings()
		{
			InitializeComponent();
		}

		public static PPUPlayerSettings LoadSettings()
		{
			PPUPlayerSettings settings = new();

			string text = Properties.Settings.Default.PPUPlayerSettings;
			if (text == "")
			{
				return SetDefaultSettings();
			}

			var ser = new XmlSerializer(typeof(PPUPlayerSettings));

			using (TextReader reader = new StringReader(text))
			{
				var res = ser.Deserialize(reader);
				if (res != null)
				{
					settings = (PPUPlayerSettings)res;
				}
			}

			return settings;
		}

		public static void SaveSettings(PPUPlayerSettings settings)
		{
			XmlSerializer ser = new XmlSerializer(typeof(PPUPlayerSettings));
			using (StringWriter textWriter = new StringWriter())
			{
				ser.Serialize(textWriter, settings);
				string text = textWriter.ToString();
				Properties.Settings.Default.PPUPlayerSettings = text;
				Properties.Settings.Default.Save();
			}
		}

		static PPUPlayerSettings SetDefaultSettings()
		{
			PPUPlayerSettings settings = new();

			settings.PPU_Revision = "RP2C02G";
			settings.CPU_CLK_Divider = "NTSC CPU (2A03) CLK÷12";
			settings.ResetPPU = false;
			settings.RenderAlwaysEnabled = false;
			settings.Visual2C02Mapping = false;
			settings.ColorDebug = true;
			settings.PpuRAWMode = false;
			settings.FreeModeVMirroring = true;
			settings.OAMDecay = BreaksCore.OAMDecayBehavior.Keep;
			settings.PpuNoise = 0.0f;
			settings.AllocConsole = false;
			settings.UseDacLevels = false;

			SaveSettings(settings);

			return settings;
		}

		private void FormSettings_Load(object sender, EventArgs e)
		{
			propertyGrid1.SelectedObject = LoadSettings();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			SaveSettings((PPUPlayerSettings)propertyGrid1.SelectedObject);
			Close();
		}

		[XmlRoot]
		public class PPUPlayerSettings
		{
			[XmlElement]
			[Category("Board Features")]
			[Description("PPU revision. Right now only the more or less studied PPU revisions are on the list, the rest will be added as chips are sent to me for research.")]
			[TypeConverter(typeof(FormatStringConverter_PPU_Revision))]
			public string? PPU_Revision { get; set; }

			[XmlElement]
			[Category("Board Features")]
			[Description("Fictional CPU clock divider. Instead of a real CPU, a RegDump processor is used, which feeds the PPU with register operations.")]
			[TypeConverter(typeof(FormatStringConverter_CLK_Divider))]
			public string? CPU_CLK_Divider { get; set; }

			[XmlElement]
			[Category("Board Features")]
			[Description("Perform a PPU reset. When simulating a reset, the /RES input signal will be set to 0 for the first 4 half cycles of CLK, to reset the PPU internally. Not all NES/Famicom motherboards support resetting. The Famicom for example never resets the PPU (/RES = 1 all the time).")]
			public bool ResetPPU { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Forcibly enable rendering ($2001[3] = $2001[4] always returns as 1). Used for debugging PPU signals, when the CPU I/F register dump is limited, or when you want to get faster simulation results. Keep in mind that with permanently enabled rendering the PPU becomes unstable and this hack should be applied when you know what you're doing.")]
			public bool RenderAlwaysEnabled { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Show color in the VRAM/Objects viewer. The CRAM currently in use will be loaded.")]
			public bool ColorDebug { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Enable the signal names from Visual2C02 instead of the Breaks names.")]
			public bool Visual2C02Mapping { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Pause the PPU after starting the simulation. Can be useful for debugging with breakpoints from the beginning of the simulation.")]
			public bool RunPaused { get; set; }

			[XmlElement]
			[Category("Board Features")]
			[Description("Use RAW color output. RAW color refers to the Chroma/Luma combination that comes to the video generator and the Emphasis bit combination.")]
			public bool PpuRAWMode { get; set; }

			[XmlElement]
			[Category("Misc")]
			[Description("Mirroring mode for Dummy NROM, which is used for Free Mode.")]
			public bool FreeModeVMirroring { get; set; }

			[XmlElement]
			[Category("Board Features")]
			[Description("Set one of the ways to decay OAM cells.")]
			public BreaksCore.OAMDecayBehavior OAMDecay { get; set; }

			[XmlElement]
			[Category("Board Features")]
			[Description("Determines the noise of the video signal. Value in volts (if not equal to zero, the signal will be noisy by +/- the specified value. Works only for the composite output.")]
			public float PpuNoise { get; set; }

			[Category("Debug")]
			[Description("Turn on the debug console. Requires a restart.")]
			[DefaultValue(false)]
			public bool AllocConsole { get; set; }

			[XmlElement]
			[Category("External DAC Levels")]
			[Description("Enable the use of DAC preset levels. There are 4 groups of levels: Sync level, ColorBurst level, 4 Luma levels and 4 EmphasizedLuma levels. Each level is represented by a pair of values (low level and high level). All values are floats, in volts (V)")]
			[DefaultValue(false)]
			public bool UseDacLevels { get; set; }

			[XmlElement]
			[Category("External DAC Levels")]
			[Description("SyncLevel low")]
			[DefaultValue(0)]
			public float SyncLevel_0 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("SyncLevel high")]
			[DefaultValue(0)]
			public float SyncLevel_1 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("BurstLevel low")]
			[DefaultValue(0)]
			public float BurstLevel_0 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("BurstLevel high")]
			[DefaultValue(0)]
			public float BurstLevel_1 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("LumaLevel[0] low")]
			[DefaultValue(0)]
			public float LumaLevel_0_0 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("LumaLevel[1] low")]
			[DefaultValue(0)]
			public float LumaLevel_0_1 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("LumaLevel[2] low")]
			[DefaultValue(0)]
			public float LumaLevel_0_2 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("LumaLevel[3] low")]
			[DefaultValue(0)]
			public float LumaLevel_0_3 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("LumaLevel[0] high")]
			[DefaultValue(0)]
			public float LumaLevel_1_0 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("LumaLevel[1] high")]
			[DefaultValue(0)]
			public float LumaLevel_1_1 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("LumaLevel[2] high")]
			[DefaultValue(0)]
			public float LumaLevel_1_2 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("LumaLevel[3] high")]
			[DefaultValue(0)]
			public float LumaLevel_1_3 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("EmphasizedLumaLevel[0] low")]
			[DefaultValue(0)]
			public float EmphasizedLumaLevel_0_0 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("EmphasizedLumaLevel[1] low")]
			[DefaultValue(0)]
			public float EmphasizedLumaLevel_0_1 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("EmphasizedLumaLevel[2] low")]
			[DefaultValue(0)]
			public float EmphasizedLumaLevel_0_2 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("EmphasizedLumaLevel[3] low")]
			[DefaultValue(0)]
			public float EmphasizedLumaLevel_0_3 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("EmphasizedLumaLevel[0] high")]
			[DefaultValue(0)]
			public float EmphasizedLumaLevel_1_0 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("EmphasizedLumaLevel[1] high")]
			[DefaultValue(0)]
			public float EmphasizedLumaLevel_1_1 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("EmphasizedLumaLevel[2] high")]
			[DefaultValue(0)]
			public float EmphasizedLumaLevel_1_2 { get; set; }
			[XmlElement]
			[Category("External DAC Levels")]
			[Description("EmphasizedLumaLevel[3] high")]
			[DefaultValue(0)]
			public float EmphasizedLumaLevel_1_3 { get; set; }
		}


		// https://stackoverflow.com/questions/24503462/how-to-show-drop-down-control-in-property-grid

		public class FormatStringConverter_PPU_Revision : StringConverter
		{
			public override Boolean GetStandardValuesSupported(ITypeDescriptorContext context) { return true; }
			public override Boolean GetStandardValuesExclusive(ITypeDescriptorContext context) { return true; }
			public override TypeConverter.StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
			{
				List<String> list = new List<String>();

				list.Add("RP2C02G");
				list.Add("RP2C04-0003");
				list.Add("RP2C07-0");
				list.Add("UMC UA6538");

				return new StandardValuesCollection(list);
			}
		}

		public class FormatStringConverter_CLK_Divider : StringConverter
		{
			public override Boolean GetStandardValuesSupported(ITypeDescriptorContext context) { return true; }
			public override Boolean GetStandardValuesExclusive(ITypeDescriptorContext context) { return true; }
			public override TypeConverter.StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
			{
				List<String> list = new List<String>();

				list.Add("NTSC CPU (2A03) CLK÷12");
				list.Add("PAL CPU (2A07) CLK÷16");
				list.Add("Decent Famiclone CPU (UMC UM6527P) CLK÷15");

				return new StandardValuesCollection(list);
			}
		}

		private void FormSettings_KeyDown(object sender, KeyEventArgs e)
		{
			if ( e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
