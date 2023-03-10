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
			Close();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
		}

		private void loadROMDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				string filename = openFileDialog1.FileName;
				board.DisposeBoard();
				var settings = FormSettings.LoadSettings();
				board.CreateBoardAndStickCartridge( BoardDescriptionLoader.Load(), settings.MainBoard, filename);
			}
		}

		private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormSettings settings = new FormSettings();
			settings.ShowDialog();
		}
	}
}
