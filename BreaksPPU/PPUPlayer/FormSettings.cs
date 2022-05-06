using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PPUPlayer
{
	public partial class FormSettings : Form
	{
		public FormSettings()
		{
			InitializeComponent();
		}

		private void FormSettings_Load(object sender, EventArgs e)
		{
			PPUPlayerSettings settings = new();

			settings.PPU_Revision = "RP2C02G";
			settings.TraceMaxFields = 2;

			propertyGrid1.SelectedObject = settings;
		}

		private void button1_Click(object sender, EventArgs e)
		{
			Close();
		}

		public class PPUPlayerSettings
		{
			[Category("Board Features")]
			[Description("PPU revision. Right now only the more or less studied PPU revisions are on the list, the rest will be added as chips are sent to me for research.")]
			[DefaultValue("RP2C02G")]
			[TypeConverter(typeof(FormatStringConverter_PPU_Revision))]
			public string? PPU_Revision { get; set; }

			[Category("Debug")]
			[Description("The number of full Fields for signal tracing. When the trace history is full, it is automatically cleared.")]
			[DefaultValue(2)]
			public int TraceMaxFields { get; set; }
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
				list.Add("RP2C04-0001");
				list.Add("RP2C07-0");

				return new StandardValuesCollection(list);
			}
		}
	}
}
