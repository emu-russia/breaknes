using System.ComponentModel;
using System.Xml.Serialization;

namespace Breaknes
{
	public partial class FormSettings : Form
	{
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

			settings.MainBoard = "HVC-CPU-01";

			SaveSettings(settings);

			return settings;
		}

		private void FormSettings_Load(object sender, EventArgs e)
		{
			propertyGrid1.SelectedObject = LoadSettings();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			SaveSettings((BreaknesSettings)propertyGrid1.SelectedObject);
			Close();
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
		}

		// https://stackoverflow.com/questions/24503462/how-to-show-drop-down-control-in-property-grid

		public class FormatStringConverter_MainBoard : StringConverter
		{
			public override Boolean GetStandardValuesSupported(ITypeDescriptorContext context) { return true; }
			public override Boolean GetStandardValuesExclusive(ITypeDescriptorContext context) { return true; }
			public override TypeConverter.StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
			{
				List<String> list = new List<String>();

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
