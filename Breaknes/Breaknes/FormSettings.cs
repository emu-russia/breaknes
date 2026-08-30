using System.ComponentModel;
using System.Text;
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

			settings.MainBoard = "NES (NES-CPU-07, 1987)";
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
			settings.NintendulatorLog = false;
			settings.LogEnabled = false;
			settings.LogToFile = false;
			settings.LogFile = "breaknes.log";
			settings.LogToStdout = false;
			settings.LogSourceMask = 0;
			settings.LogCategoryMasks = "";

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

		private void button2_Click(object sender, EventArgs e)
		{
			BreaknesSettings settings = (BreaknesSettings)propertyGrid1.SelectedObject!;
			FormLogging dlg = new FormLogging(settings);
			dlg.ShowDialog(this);
			propertyGrid1.Refresh();
		}

		/// <summary>
		/// Get the stored category mask of one log source from the
		/// "source:mask;source:mask;..." string of the settings.
		/// </summary>
		public static ulong GetLogCategoryMask(BreaknesSettings settings, int source)
		{
			if (settings.LogCategoryMasks == null)
				return 0;

			foreach (var part in settings.LogCategoryMasks.Split(';', StringSplitOptions.RemoveEmptyEntries))
			{
				var kv = part.Split(':');
				if (kv.Length == 2 && int.TryParse(kv[0].Trim(), out int s) && s == source && ulong.TryParse(kv[1].Trim(), out ulong mask))
					return mask;
			}

			return 0;
		}

		/// <summary>
		/// Store the category masks of all sources in the "source:mask;source:mask;..."
		/// format of the settings.
		/// </summary>
		public static void SetLogCategoryMasks(BreaknesSettings settings, Dictionary<int, ulong> masks)
		{
			var sb = new StringBuilder();

			foreach (var kv in masks)
			{
				if (sb.Length != 0)
					sb.Append(';');
				sb.Append(kv.Key).Append(':').Append(kv.Value);
			}

			settings.LogCategoryMasks = sb.ToString();
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

			[Category("Debug")]
			[Description("Enable writing the Nintendulator-compatible CPU instruction trace log. The log is written to the Nintendulator.log file in the working directory and can be used to compare this emulator with the Nintendulator emulator.")]
			[DefaultValue(false)]
			public bool NintendulatorLog { get; set; }

			[Category("Logging")]
			[Description("Master switch of the logging facility (issue #517). The sources and their categories are configured in the Logging dialog.")]
			[DefaultValue(false)]
			public bool LogEnabled { get; set; }

			[Category("Logging")]
			[Description("Write the log to a file.")]
			[DefaultValue(false)]
			public bool LogToFile { get; set; }

			[Category("Logging")]
			[Description("The log file path. The file is truncated on every start.")]
			[DefaultValue("")]
			[EditorAttribute(typeof(System.Windows.Forms.Design.FileNameEditor), typeof(System.Drawing.Design.UITypeEditor))]
			public string LogFile { get; set; } = "breaknes.log";

			[Category("Logging")]
			[Description("Also write the log to stdout (requires the debug console, see AllocConsole).")]
			[DefaultValue(false)]
			public bool LogToStdout { get; set; }

			[Category("Logging")]
			[Description("Log source mask: bit n enables the log source n (0 = no sources). Managed by the Logging dialog.")]
			[DefaultValue(0)]
			public ulong LogSourceMask { get; set; }

			[Category("Logging")]
			[Description("Per-source category masks in the \"source:mask;source:mask;...\" format. Managed by the Logging dialog.")]
			[DefaultValue("")]
			public string LogCategoryMasks { get; set; } = "";
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
