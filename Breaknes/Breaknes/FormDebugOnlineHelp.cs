using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Breaknes
{
    public partial class FormDebugOnlineHelp : Form
    {
        string savedUrl;

        public FormDebugOnlineHelp(string url)
        {
            InitializeComponent();
            savedUrl = url;
        }

        private void FormDebugOnlineHelp_Load(object sender, EventArgs e)
        {
            webView21.Source = new Uri(savedUrl);
            Text += " - " + savedUrl;
        }
    }
}
