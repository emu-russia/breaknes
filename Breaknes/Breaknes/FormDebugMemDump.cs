using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using Be.Windows.Forms;

namespace Breaknes
{
    public partial class FormDebugMemDump : Form
    {
        byte[] mem = new byte[0x10000];

        public FormDebugMemDump()
        {
            InitializeComponent();
        }

        private void FormDebugMemDump_Load(object sender, EventArgs e)
        {
            // Test

            IByteProvider memProvider = new DynamicByteProvider(mem);
            hexBox1.ByteProvider = memProvider;

            hexBox1.Refresh();
        }
    }
}
