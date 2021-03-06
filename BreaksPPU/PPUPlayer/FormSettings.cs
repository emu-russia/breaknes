using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Xml.Serialization;
using System.IO;

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
				settings = (PPUPlayerSettings)ser.Deserialize(reader);
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
			settings.ResetPPU = false;
			settings.TraceEnable = false;
			settings.TraceMaxFields = 2;
			settings.TraceFilter = "";
			settings.TraceCollapseSameRows = true;
			settings.RenderAlwaysEnabled = false;
			settings.TraceTimeScale = 23;   // ns
			settings.ColorDebug = true;
			settings.PpuRAWMode = false;
			settings.FreeModeVMirroring = true;
			settings.OAMDecay = PPUPlayerInterop.OAMDecayBehavior.Keep;

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
			[Description("Perform a PPU reset. When simulating a reset, the /RES input signal will be set to 0 for the first 4 half cycles of CLK, to reset the PPU internally. Not all NES/Famicom motherboards support resetting. The Famicom for example never resets the PPU (/RES = 1 all the time).")]
			public bool ResetPPU { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Enable tracing. Trace starts collecting immediately after the PPU starts. After stopping the trace is cleared. History is stored for no more than the specified number of Fields.")]
			public bool TraceEnable { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("The number of full Fields for signal tracing. When the trace history is full, it is automatically cleared.")]
			public int TraceMaxFields { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Specify with ; only necessary signals for tracing. If no filter is specified, all signals will be traced.")]
			public string? TraceFilter { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Merge neighboring trace rows if signal values have not changed.")]
			public bool TraceCollapseSameRows { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Forcibly enable rendering ($2001[3] = $2001[4] always returns as 1). Used for debugging PPU signals, when the CPU I/F register dump is limited, or when you want to get faster simulation results. Keep in mind that with permanently enabled rendering the PPU becomes unstable and this hack should be applied when you know what you're doing.")]
			public bool RenderAlwaysEnabled { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Trace sampling resolution in nanoseconds. A typical interval is 1 half-cycle NTSC PPU lasting 23 nanoseconds. Used in the VCD dump.")]
			public int TraceTimeScale { get; set; }

			[XmlElement]
			[Category("Debug")]
			[Description("Show color in the VRAM/Objects viewer. The CRAM currently in use will be loaded.")]
			public bool ColorDebug { get; set; }

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
			public PPUPlayerInterop.OAMDecayBehavior OAMDecay { get; set; }
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

		private void FormSettings_KeyDown(object sender, KeyEventArgs e)
		{
			if ( e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
