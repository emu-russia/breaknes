using System;
using System.Windows.Forms;

namespace SharpTools
{
	public partial class FormSnatchWaves : Form
	{
		public FormSnatchWaves(ValueChangeData[] data)
		{
			InitializeComponent();

			wavesControl1.PlotWaves(data, 0);
			wavesControl1.EnableSelection(true);
		}

		private void FormSnatchWaves_KeyUp(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}

		private void toolStripButton1_Click(object sender, EventArgs e)
		{
			if (wavesControl1.IsSelectedSomething())
			{
				long bias;
				ValueChangeData[] data = wavesControl1.SnatchSelection(out bias);
				wavesControl1.ClearSelection();
				FormSnatchWaves snatch = new FormSnatchWaves(data);
				snatch.Show();
			}
			else
			{
				MessageBox.Show("Select something first with the left mouse button. A box will appear, then click on Snatch.", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
			}
		}
	}
}
