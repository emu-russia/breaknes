using System.ComponentModel;
using System.Xml.Serialization;

namespace Breaknes
{
	public partial class FormSettings : Form
	{
		public bool PurgeBoard = false;
		private string prev_board;

		public FormSettings()
		{
			InitializeComponent();
		}

		public static BreaknesSettings LoadSettings()
		{
			BreaknesSettings settings = new();

			string text = Properties.Settings.Default.BreaknesSettings;
			if (text == "")
			{
				return SetDefaultSettings();
			}

			var ser = new XmlSerializer(typeof(BreaknesSettings));

			using (TextReader reader = new StringReader(text))
			{
				var res = ser.Deserialize(reader);
				if (res != null)
				{
					settings = (BreaknesSettings)res;
				}
			}

			return settings;
		}

		public static void SaveSettings(BreaknesSettings settings)
		{
			XmlSerializer ser = new XmlSerializer(typeof(BreaknesSettings));
			using (StringWriter textWriter = new StringWriter())
			{
				ser.Serialize(textWriter, settings);
				string text = textWriter.ToString();
				Properties.Settings.Default.BreaknesSettings = text;
				Properties.Settings.Default.Save();
			}
		}

		static BreaknesSettings SetDefaultSettings()
		{
			BreaknesSettings settings = new();

			settings.MainBoard = "NES-001 (PCB rev. -01 to -04)";
			settings.DumpAudio = false;
			settings.DumpAudioDir = "";
			settings.DumpVideo = false;
			settings.DumpVideoDir = "";
			settings.AllocConsole = false;
			settings.IIR = true;
			settings.CutoffFrequency = 12000;
			settings.PPURegdump = false;
			settings.PPURegdumpDir = "";
			settings.APURegdump = false;
			settings.APURegdumpDir = "";

			SaveSettings(settings);

			return settings;
		}

		private void FormSettings_Load(object sender, EventArgs e)
		{
			var settings = LoadSettings();
			prev_board = settings.MainBoard;
			propertyGrid1.SelectedObject = settings;
		}

		private void button1_Click(object sender, EventArgs e)
		{
			BreaknesSettings settings = (BreaknesSettings)propertyGrid1.SelectedObject;
			SaveSettings(settings);
			Close();
			PurgeBoard = settings.MainBoard != prev_board;
		}

		private void FormSettings_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}

		public class BreaknesSettings
		{
			[Category("Settings")]
			[Description("Select one of the supported motherboards. After starting the emulation, BreaksCore will automatically try to \"fit\" the specified .nes file into the selected motherboard. The APU/PPU is selected based on the motherboard, and the description of all the boards can be found in the BoardDescription.json, which is stored in the resources.")]
			[DefaultValue("")]
			[TypeConverter(typeof(FormatStringConverter_MainBoard))]
			public string? MainBoard { get; set; }

			[Category("Debug")]
			[Description("Enable sound dump. Format: 16-bit, little-endian, mono, 48000 Hz")]
			[DefaultValue(false)]
			public bool DumpAudio { get; set; }

			[Category("Debug")]
			[Description("The directory where the sound dump will be saved. File name RomName_aux.bin")]
			[DefaultValue("")]
			[EditorAttribute(typeof(System.Windows.Forms.Design.FolderNameEditor), typeof(System.Drawing.Design.UITypeEditor))]
			public string DumpAudioDir { get; set; } = "";

			[Category("Debug")]
			[Description("Turn on the PPU fields dump. This will dump a field with raw pixels and .bmp")]
			[DefaultValue(false)]
			public bool DumpVideo { get; set; }

			[Category("Debug")]
			[Description("The directory where the field dumps will be stored. The names of the files you can figure out by yourself when you see the dumps.")]
			[DefaultValue("")]
			[EditorAttribute(typeof(System.Windows.Forms.Design.FolderNameEditor), typeof(System.Drawing.Design.UITypeEditor))]
			public string DumpVideoDir { get; set; } = "";

			[Category("Debug")]
			[Description("Turn on the debug console. Requires a restart.")]
			[DefaultValue(false)]
			public bool AllocConsole { get; set; }

			[XmlElement]
			[Category("Sound Features")]
			[Description("Implementation of a simple IIR filter, in addition to decimation")]
			public bool IIR { get; set; }

			[XmlElement]
			[Category("Sound Features")]
			[Description("Frequency for IIR filter (try 12000 for example)")]
			public int CutoffFrequency { get; set; }

			[Category("Debug")]
			[Description("Enable saving the history of accesses to PPU registers (regdump). Regdump can be used later in the PPUPlayer utility.")]
			[DefaultValue(false)]
			public bool PPURegdump { get; set; }

			[Category("Debug")]
			[Description("Directory where the history of accesses to PPU registers will be saved (regdump)")]
			[DefaultValue("")]
			[EditorAttribute(typeof(System.Windows.Forms.Design.FolderNameEditor), typeof(System.Drawing.Design.UITypeEditor))]
			public string PPURegdumpDir { get; set; } = "";

			[Category("Debug")]
			[Description("Enable saving the history of accesses to APU registers (regdump). Regdump can be used later in the APUPlayer utility.")]
			[DefaultValue(false)]
			public bool APURegdump { get; set; }

			[Category("Debug")]
			[Description("Directory where the history of accesses to APU registers will be saved (regdump)")]
			[DefaultValue("")]
			[EditorAttribute(typeof(System.Windows.Forms.Design.FolderNameEditor), typeof(System.Drawing.Design.UITypeEditor))]
			public string APURegdumpDir { get; set; } = "";
		}

		// https://stackoverflow.com/questions/24503462/how-to-show-drop-down-control-in-property-grid

		public class FormatStringConverter_MainBoard : StringConverter
		{
			public override Boolean GetStandardValuesSupported(ITypeDescriptorContext context) { return true; }
			public override Boolean GetStandardValuesExclusive(ITypeDescriptorContext context) { return true; }
			public override TypeConverter.StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
			{
				List<String> list = new();

				list.Add("BogusBoard");

				var descr = BoardDescriptionLoader.Load();

				if (descr != null)
				{
					foreach (var board in descr.boards)
					{
						list.Add(board.name);
					}
				}

				return new StandardValuesCollection(list);
			}
		}
	}
}
