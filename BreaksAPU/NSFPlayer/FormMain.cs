namespace NSFPlayer
{
	public partial class FormMain : Form
	{
		public FormMain()
		{
			InitializeComponent();
		}

		private void loadNSFToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void playToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void pauseToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void stopToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void previousTrackToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void nextTrackToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void nSFInfoToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormAbout dlg = new FormAbout();
			dlg.ShowDialog();
		}

		private void loadSaveLogisimHexAsBinToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if ( openFileDialogHEX.ShowDialog() == DialogResult.OK )
			{
				string hex_filename = openFileDialogHEX.FileName;
				byte [] arr = LogisimHEXConv.HEXToByteArray (File.ReadAllText(hex_filename));

				if (saveFileDialogBin.ShowDialog() == DialogResult.OK)
				{
					string bin_filename = saveFileDialogBin.FileName;
					File.WriteAllBytes (bin_filename, arr);
				}
			}
		}
	}
}