using System.Runtime.InteropServices;

namespace Breaknes
{
	public partial class FormMain : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		private BoardControl board = new();
		private VideoRender vid_out = new();
		private int debug_instances = 0;
		private string original_title;

		public FormMain()
		{
			InitializeComponent();
			//AllocConsole();
		}

		private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormAbout about = new FormAbout();
			about.ShowDialog();
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			board.Paused = true;
			board.EjectCartridge();
			board.DisposeBoard();
			Close();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
			original_title = Text;
			var settings = FormSettings.LoadSettings();
			board.CreateBoard(BoardDescriptionLoader.Load(), settings.MainBoard);
			backgroundWorker1.RunWorkerAsync();
			vid_out.SetOutputPictureBox(pictureBox1);
		}

		private void loadROMDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				string filename = openFileDialog1.FileName;
				board.EjectCartridge();
				board.InsertCartridge(filename);
				Text = original_title + " - " + filename;
				board.Paused = debug_instances != 0;
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
			board.Paused = true;
			board.EjectCartridge();
			board.DisposeBoard();
			var settings = FormSettings.LoadSettings();
			board.CreateBoard(BoardDescriptionLoader.Load(), settings.MainBoard);
			Text = original_title;
		}

		private void openDebuggerToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormDebugger debugger = new(board);
			debugger.FormClosed += Debugger_FormClosed;
			debugger.Show();
			debug_instances++;
		}

		private void Debugger_FormClosed(object? sender, FormClosedEventArgs e)
		{
			debug_instances--;
		}
	}
}
