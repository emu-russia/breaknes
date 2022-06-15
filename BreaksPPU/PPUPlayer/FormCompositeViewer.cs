using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.IO;

namespace PPUPlayer
{
	public partial class FormCompositeViewer : Form
	{
		public FormCompositeViewer(string filename)
		{
			InitializeComponent();
		}

		private void FormCompositeViewer_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
