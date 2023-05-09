using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTools
{
	public partial class FormAddBreakpoint : Form
	{
		public Breakpoint bp = new Breakpoint();
		public bool bp_added = false;

		public FormAddBreakpoint()
		{
			InitializeComponent();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			bp = breakpointProps1.ControlsToBreakpoint();
			bp_added = true;
			Close();
		}
	}
}
