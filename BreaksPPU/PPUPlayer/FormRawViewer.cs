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
	public partial class FormRawViewer : Form
	{
		public FormRawViewer(string filename)
		{
			InitializeComponent();

			// Load the Logisim dump

			string text = File.ReadAllText(filename);
			UInt16[] dump = LogisimHEXConv.HEXToShortArray(text);

			// Process the dump field by field and fill the ComboBox and PictureView.
		}

		private void FormRawViewer_KeyDown(object sender, KeyEventArgs e)
		{
			if ( e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
