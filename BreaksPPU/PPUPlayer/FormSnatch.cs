using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PPUPlayer
{
	public partial class FormSnatch : Form
	{
		private PPUPlayerInterop.VideoSignalFeatures ppu_features;

		public FormSnatch(float[] data)
		{
			InitializeComponent();

			PPUPlayerInterop.GetSignalFeatures(out ppu_features);

			signalPlot1.PlotSignal(data);
			signalPlot1.EnableSelection(true);
			signalPlot1.EnabledDottedEveryNth(ppu_features.SamplesPerPCLK, true);
		}

		private void toolStripButton1_Click(object sender, EventArgs e)
		{
			if (signalPlot1.IsSelectedSomething())
			{
				float[] data = signalPlot1.SnatchSelection();
				signalPlot1.ClearSelection();
				FormSnatch snatch = new FormSnatch(data);
				snatch.Show();
			}
			else
			{
				MessageBox.Show("Select something first with the left mouse button. A box will appear, then click on Snatch.", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
			}
		}

		private void FormSnatch_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
