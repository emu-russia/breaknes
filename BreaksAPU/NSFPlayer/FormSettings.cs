using System.ComponentModel;
using System.Xml.Serialization;

namespace NSFPlayer
{
	public partial class FormSettings : Form
	{
		public DialogResult UserAnswer = DialogResult.Cancel;

		public FormSettings()
		{
			InitializeComponent();
		}

		public static APUPlayerSettings LoadSettings()
		{
			APUPlayerSettings settings = new();

			string text = Properties.Settings.Default.APUPlayerSettings;
			if (text == "")
			{
				return SetDefaultSettings();
			}

			var ser = new XmlSerializer(typeof(APUPlayerSettings));

			using (TextReader reader = new StringReader(text))
			{
				var res = ser.Deserialize(reader);
				if (res != null)
				{
					settings = (APUPlayerSettings)res;
				}
			}

			if (settings.FurryIntensity == 0)
				settings.FurryIntensity = 500.0f;
			if (settings.OutputSampleRate == 0)
				settings.OutputSampleRate = 48000;
			if (settings.AuxSampleRate == 0)
				settings.AuxSampleRate = 48000;

			return settings;
		}

		public static void SaveSettings(APUPlayerSettings settings)
		{
			XmlSerializer ser = new XmlSerializer(typeof(APUPlayerSettings));
			using (StringWriter textWriter = new StringWriter())
			{
				ser.Serialize(textWriter, settings);
				string text = textWriter.ToString();
				Properties.Settings.Default.APUPlayerSettings = text;
				Properties.Settings.Default.Save();
			}
		}

		static APUPlayerSettings SetDefaultSettings()
		{
			APUPlayerSettings settings = new();

			settings.APU_Revision = "RP2A03G";
			settings.OutputSampleRate = 48000;
			settings.AutoPlay = true;
			settings.FurryIntensity = 500.0f;
			settings.PreferPal = false;
			settings.AuxSampleRate = 3579544;       // 0.5 PHI for 2A03
			settings.DC = 0.0f;

			SaveSettings(settings);

			return settings;
		}

		private void FormSettings_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}

		private void FormSettings_Load(object sender, EventArgs e)
		{
			propertyGrid1.SelectedObject = LoadSettings();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			SaveSettings((APUPlayerSettings)propertyGrid1.SelectedObject);
			UserAnswer = DialogResult.OK;
			Close();
		}

		[XmlRoot]
		public class APUPlayerSettings
		{
			[XmlElement]
			[Category("Board Features")]
			[Description("APU revision. Right now only the more or less studied APU revisions are on the list, the rest will be added as chips are sent to me for research.")]
			[TypeConverter(typeof(FormatStringConverter_APU_Revision))]
			public string? APU_Revision { get; set; }

			[XmlElement]
			[Category("Host Features")]
			[Description("The AUX output is sampled at a high frequency, which cannot be played by a ordinary sound card. Therefore, some of the samples are skipped to match the DSound playback frequency (this setting).")]
			public int OutputSampleRate { get; set; }

			[XmlElement]
			[Category("Host Features")]
			[Description("Automatically start the simulation after loading the NSF/Regdump")]
			public bool AutoPlay { get; set; }
			
			[XmlElement]
			[Category("Host Features")]
			[Description("Amplification for FFT")]
			public float FurryIntensity { get; set; }

			[XmlElement]
			[Category("Host Features")]
			[Description("If the NSF header specifies that you can use hybrid PAL/NTSC - specify explicitly that we want PAL.")]
			public bool PreferPal { get; set; }
			
			[XmlElement]
			[Category("Host Features")]
			[Description("If you run APUSim - it says the frequency, but if you stick the AUX dump - what frequency is not clear. This setting allows you to select the sampling frequency of the AUX dump.")]
			[TypeConverter(typeof(FormatInt32ConverterConverter_AuxSampleRate))]
			public int AuxSampleRate { get; set; }

			[XmlElement]
			[Category("Host Features")]
			[Description("A constant offset that is added when converting a normalized sample received from APUSim to 16-bit PCM format for the DSound buffer.")]
			public float DC { get; set; }
		}

		// https://stackoverflow.com/questions/24503462/how-to-show-drop-down-control-in-property-grid

		public class FormatStringConverter_APU_Revision : StringConverter
		{
			public override Boolean GetStandardValuesSupported(ITypeDescriptorContext context) { return true; }
			public override Boolean GetStandardValuesExclusive(ITypeDescriptorContext context) { return true; }
			public override TypeConverter.StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
			{
				List<String> list = new List<String>();

				list.Add("RP2A03G");
				list.Add("RP2A03H");
				list.Add("RP2A07");
				list.Add("UA6527P");
				list.Add("TA03NP1");

				return new StandardValuesCollection(list);
			}
		}

		public class FormatInt32ConverterConverter_AuxSampleRate : Int32Converter
		{
			public override Boolean GetStandardValuesSupported(ITypeDescriptorContext context) { return true; }
			public override Boolean GetStandardValuesExclusive(ITypeDescriptorContext context) { return false; }
			public override TypeConverter.StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
			{
				List<int> list = new List<int>();

				list.Add(48000);        // Default 1:1, same as OutputSampleRate
				list.Add(42954544);		// 0.5 CLK for 2A03
				list.Add(3579544);      // 0.5 PHI for 2A03

				return new StandardValuesCollection(list);
			}
		}
	}
}
