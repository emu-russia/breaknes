using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Runtime.InteropServices;

namespace Breaknes
{
    public partial class FormDebugInfo : Form
    {
        BreaksCore.DebugInfoType savedType;

        public FormDebugInfo(BreaksCore.DebugInfoType type)
        {
            savedType = type;

            InitializeComponent();
        }

        private void FormDebugInfo_Load(object sender, EventArgs e)
        {
            Text += " - " + savedType.ToString();

            // Get debug entries of the selected type

            List<BreaksCore.DebugInfoEntry> entries = BreaksCore.GetDebugInfo(savedType);

            // Construct an object to show in PropertyGrid

            // TBD: Make a proper dynamic filling of PropertyGrid

            CookedDebugInfo info = new CookedDebugInfo();

            foreach (BreaksCore.DebugInfoEntry entry in entries)
            {
                CookedDebugInfoEntry prop = new CookedDebugInfoEntry();
                prop.name = entry.name;
                prop.value = entry.value;
                info.entries.Add(prop);
            }

            propertyGrid1.SelectedObject = info;
        }

        internal class CookedDebugInfoEntry
        {
            public string name { get; set; }
            public uint value { get; set; }
        }

        internal class CookedDebugInfo
        {
            [TypeConverter(typeof(ListConverter))]
            [Category("TBD")]
            public List<CookedDebugInfoEntry> entries { get; set; } = new List<CookedDebugInfoEntry>();
        }

    }
}
