using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using Newtonsoft.Json;

namespace Breaknes
{
    public partial class FormSettings : Form
    {
        public FormSettings()
        {
            InitializeComponent();
        }

        private void FormSettings_Load(object sender, EventArgs e)
        {
            BreaknesSettings settings = new BreaknesSettings();

            propertyGrid1.SelectedObject = settings;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            // TBD: Save the settings in the depths of the application.

            Close();
        }
    }

    public class BreaknesSettings
    {
        [Category("Settings")]
        [Description("Select one of the supported motherboards. After starting the emulation, BreaksCore will automatically try to \"fit\" the specified .nes file into the selected motherboard. The APU/PPU is selected based on the motherboard, and the description of all the boards can be found in the BoardDescription.json, which is stored in the resources.")]
        [DefaultValue("")]
        [TypeConverter(typeof(FormatStringConverter))]
        public string? MainBoard {  get; set; }
    }

    // https://stackoverflow.com/questions/24503462/how-to-show-drop-down-control-in-property-grid

    public class FormatStringConverter : StringConverter
    {
        public override Boolean GetStandardValuesSupported(ITypeDescriptorContext context) { return true; }
        public override Boolean GetStandardValuesExclusive(ITypeDescriptorContext context) { return true; }
        public override TypeConverter.StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
        {
            List<String> list = new List<String>();

            list.Add("");

            string json = Encoding.UTF8.GetString(Properties.Resources.BoardDescription);

            var descr = JsonConvert.DeserializeObject<BoardDescription>(json);

            foreach (var board in descr.boards)
            {
                list.Add(board.name);
            }

            return new StandardValuesCollection(list);
        }
    }
}
