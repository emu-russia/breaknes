using System.Runtime.InteropServices;

namespace NSFPlayer
{
	public partial class FormMain : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		private DSound? audio_backend;

		private int SourceSampleRate = 48000;
		private List<float> SampleBuf = new();

		private string DefaultTitle = "";

		public FormMain()
		{
			InitializeComponent();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
#if DEBUG
			AllocConsole();
#endif

			audio_backend = new DSound(Handle);

			DefaultTitle = this.Text;
		}

		private void loadNSFToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormSettings formSettings = new();
			formSettings.ShowDialog();
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

		#region "NSF Controls"

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
			FormNSFInfo info = new FormNSFInfo();
			info.ShowDialog();
		}

		#endregion "NSF Controls"

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

		#region "Sample Buffer Playback Controls"

		private void toolStripButtonPlay_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
				audio_backend.PlaySampleBuf(SourceSampleRate, SampleBuf);
		}

		private void toolStripButtonDiscard_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
				audio_backend.StopSampleBuf();
			SampleBuf.Clear();
		}

		private void toolStripButtonStop_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
				audio_backend.StopSampleBuf();
		}

		#endregion "Sample Buffer Playback Controls"

	}
}
