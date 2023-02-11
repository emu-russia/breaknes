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
using static NSFPlayer.FormSettings;

namespace NSFPlayer
{
	public partial class FormSettings : Form
	{
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
			[Description("Automatically start the simulation after loading the NSF")]
			public bool AutoPlay { get; set; }
			
			[XmlElement]
			[Category("Host Features")]
			[Description("Amplification for FFT")]
			public float FurryIntensity { get; set; }
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
	}
}
