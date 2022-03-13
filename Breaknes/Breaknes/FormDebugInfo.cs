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

            foreach (BreaksCore.DebugInfoEntry entry in entries)
            {
                Console.WriteLine(entry.category);
                Console.WriteLine(entry.name);
                Console.WriteLine(entry.value.ToString());
            }

            // Construct an object to show in PropertyGrid

            object info = new object();

            propertyGrid1.SelectedObject = info;
        }
    }
}
