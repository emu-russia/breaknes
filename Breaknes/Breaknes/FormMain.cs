using System.Runtime.InteropServices;

namespace Breaknes
{
	public partial class FormMain : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		BoardControl board = new();

		public FormMain()
		{
			InitializeComponent();

#if DEBUG
			AllocConsole();
#endif
		}

		private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormAbout about = new FormAbout();
			about.ShowDialog();
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Paused = true;
			board.EjectCartridge();
			board.DisposeBoard();
			Close();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
			var settings = FormSettings.LoadSettings();
			board.CreateBoard(BoardDescriptionLoader.Load(), settings.MainBoard);
			backgroundWorker1.RunWorkerAsync();
		}

		private void loadROMDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				string filename = openFileDialog1.FileName;
				board.EjectCartridge();
				board.InsertCartridge(filename);
				Paused = false;
			}
		}

		private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormSettings settings = new FormSettings();
			settings.FormClosed += Settings_FormClosed;
			settings.ShowDialog();
		}

		private void Settings_FormClosed(object? sender, FormClosedEventArgs e)
		{
			Paused = true;
			board.EjectCartridge();
			board.DisposeBoard();
			var settings = FormSettings.LoadSettings();
			board.CreateBoard(BoardDescriptionLoader.Load(), settings.MainBoard);
		}
	}
}
