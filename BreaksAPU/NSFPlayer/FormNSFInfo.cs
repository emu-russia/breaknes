using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NSFPlayer
{
	public partial class FormNSFInfo : Form
	{
		public FormNSFInfo(NSFHeader? head)
		{
			InitializeComponent();

			if (head != null)
			{
				propertyGrid1.SelectedObject = head;
			}
		}

		private void FormNSFInfo_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
