
namespace NSFPlayer
{
	public partial class FormSnatch : Form
	{
		public FormSnatch(float[] data)
		{
			InitializeComponent();

			signalPlot1.PlotSignal(data);
			signalPlot1.EnableSelection(true);
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
