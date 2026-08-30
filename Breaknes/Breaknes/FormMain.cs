using Breaknes.Properties;
using SharpTools;
using System.Runtime.InteropServices;

namespace Breaknes
{
	public partial class FormMain : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		private BoardControl board = new();
		private List<VideoRender> tv_renders = new();
		private AudioRender? snd_out = null;
		private IOProcessor? io = null;
		private string original_title = "";
		private List<FormDebugger> debuggers = new();

		/// <summary>
		/// The board description entry selected in the settings (defines the PPU list,
		/// the TV layout and the TV<->PPU binding).
		/// </summary>
		private Board? currentBoard = null;

		public FormMain()
		{
			InitializeComponent();
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
			if (settings.AllocConsole)
			{
				AllocConsole();
			}
			board.onUpdateWaves += OnUpdateWaves;
			var boardDesc = BoardDescriptionLoader.Load();
			currentBoard = boardDesc.boards.FirstOrDefault(b => b.name == settings.MainBoard);
			board.CreateBoard(boardDesc, settings.MainBoard);
			if (settings.PPURegdump)
			{
				BreaksCore.EnablePpuRegDump(true, settings.PPURegdumpDir);
			}
			if (settings.APURegdump)
			{
				BreaksCore.EnableApuRegDump(true, settings.APURegdumpDir);
			}
			if (settings.NintendulatorLog)
			{
				BreaksCore.EnableNintendulatorLog(true);
			}
			ApplyLogSettings(settings);
			io = new IOProcessor();
			backgroundWorker1.RunWorkerAsync();
		}

		private void loadROMDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				string filename = openFileDialog1.FileName;
				board.Paused = true;
				board.EjectCartridge();
				if (board.InsertCartridge(filename) < 0)
				{
					MessageBox.Show("Unsupported or corrupted NES ROM image format.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
					return;
				}
				BreaksCore.Reset();
				Text = original_title + " - " + filename;
				var settings = FormSettings.LoadSettings();
				var rom_name = Path.GetFileNameWithoutExtension(filename);

				// Create the TV Set renders: one per connected TV, in the physical layout
				// described in the BoardDescription.json ("tv_layout", "tvs").
				int tvCount = BreaksCore.GetTVCount();
				SetupTvLayout(currentBoard?.tv_layout ?? "horizontal", tvCount);

				tv_renders.Clear();
				for (int tv = 0; tv < tvCount && tv < 2; tv++)
				{
					var vr = new VideoRender(tv, tv == 0 ? OnRenderField : null, settings.DumpVideo, settings.DumpVideoDir, rom_name);
					vr.SetOutputPictureBox(tv == 0 ? pictureBox1 : pictureBox2);
					tv_renders.Add(vr);
				}

				snd_out = new(Handle, settings.DumpAudio, settings.DumpAudioDir, rom_name, settings.IIR, settings.CutoffFrequency);

				if (io != null)
				{
					io.DetachDevicesFromBoard();
				}
				io = new IOProcessor();
				io.AttachDevicesToBoard(settings.MainBoard);

				board.Paused = debuggers.Count != 0;

				foreach (var inst in debuggers)
				{
					inst.UpdateOnRomLoad();
					inst.ResetWaves();
				}
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
			FormSettings form_settings = (FormSettings)sender;

			var settings = FormSettings.LoadSettings();

			if (form_settings.PurgeBoard)
			{
				board.Paused = true;
				board.EjectCartridge();
				board.DisposeBoard();

				var boardDesc = BoardDescriptionLoader.Load();
				currentBoard = boardDesc.boards.FirstOrDefault(b => b.name == settings.MainBoard);
				board.CreateBoard(boardDesc, settings.MainBoard);
				Text = original_title;

				// The old renders refer to the destroyed board; drop them and reset the TV layout.
				tv_renders.Clear();
				SetupTvLayout(currentBoard?.tv_layout ?? "horizontal", BreaksCore.GetTVCount());
			}

			BreaksCore.EnablePpuRegDump(settings.PPURegdump, settings.PPURegdumpDir);
			BreaksCore.EnableApuRegDump(settings.APURegdump, settings.APURegdumpDir);
			BreaksCore.EnableNintendulatorLog(settings.NintendulatorLog);
			ApplyLogSettings(settings);
		}

		/// <summary>
		/// Arrange the TV picture boxes in the window according to the physical TV layout
		/// from the BoardDescription.json ("tv_layout": "horizontal"/"vertical").
		/// </summary>
		private void SetupTvLayout(string layout, int tvCount)
		{
			tableLayoutPanel1.SuspendLayout();
			tableLayoutPanel1.Controls.Clear();
			tableLayoutPanel1.ColumnStyles.Clear();
			tableLayoutPanel1.RowStyles.Clear();

			if (tvCount > 1)
			{
				// Two TV Sets: host them in a single cell and dock the picture boxes.
				// The edge-docked box keeps a fixed 256x240 area, the second one fills
				// the rest (Zoom keeps the picture aspect ratio in both).
				tableLayoutPanel1.ColumnCount = 1;
				tableLayoutPanel1.RowCount = 1;
				tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F));
				tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 100F));

				pictureBox1.SizeMode = pictureBox2.SizeMode = PictureBoxSizeMode.Zoom;
				pictureBox1.BackColor = pictureBox2.BackColor = Color.Black;
				pictureBox2.Visible = true;

				bool vertical = layout == "vertical";

				if (vertical)
				{
					// Vertical: the first TV on top, the second one below it.
					pictureBox1.Dock = DockStyle.Top;
					pictureBox2.Dock = DockStyle.Fill;
					pictureBox1.Height = 240;
					ClientSize = new Size(256 + 32, 2 * 240 + 72);
				}
				else
				{
					// Horizontal: the first TV on the left, the second one to its right.
					pictureBox1.Dock = DockStyle.Left;
					pictureBox2.Dock = DockStyle.Fill;
					pictureBox1.Width = 256;
					ClientSize = new Size(2 * 256 + 32, 240 + 72);
				}

				// The Fill control goes first, so the edge-docked control takes the strip.
				tableLayoutPanel1.Controls.Add(pictureBox2);
				tableLayoutPanel1.Controls.Add(pictureBox1);
			}
			else
			{
				// Single TV: restore the original centered layout.
				tableLayoutPanel1.ColumnCount = 3;
				tableLayoutPanel1.RowCount = 1;
				tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50F));
				tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle());
				tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50F));
				tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 100F));

				pictureBox1.Dock = DockStyle.None;
				pictureBox1.SizeMode = PictureBoxSizeMode.Normal;
				pictureBox1.Location = new Point(72, 3);
				pictureBox1.Size = new Size(256, 240);
				pictureBox2.Visible = false;

				tableLayoutPanel1.Controls.Add(pictureBox1, 1, 0);
				ClientSize = new Size(401, 288);
			}

			tableLayoutPanel1.ResumeLayout();
		}

		/// <summary>
		/// Apply the logging settings to the native side (issue #517).
		/// </summary>
		private static void ApplyLogSettings(FormSettings.BreaknesSettings settings)
		{
			BreaksCore.SetLogEnabled(settings.LogEnabled);
			BreaksCore.SetLogSourceMask(settings.LogSourceMask);

			int sourceCount = BreaksCore.GetLogSourceCount();
			for (int src = 0; src < sourceCount; src++)
			{
				BreaksCore.SetLogCategoryMask(src, FormSettings.GetLogCategoryMask(settings, src));
			}

			BreaksCore.SetLogToFile(settings.LogToFile, settings.LogFile ?? "");
			BreaksCore.SetLogToStdout(settings.LogToStdout);
		}

		private void openDebuggerToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormDebugger debugger = new(board);
			debugger.FormClosed += Debugger_FormClosed;
			debugger.Show();
			debuggers.Add(debugger);
		}

		private void Debugger_FormClosed(object? sender, FormClosedEventArgs e)
		{
			debuggers.Remove((FormDebugger)sender);
		}

		private void OnRenderField()
		{
			if (io != null)
			{
				io.PollDevices();
				io.DispatchEventQueue();
			}

			foreach (var inst in debuggers)
			{
				inst.UpdateOnRenderField();
			}
		}

		private void OnUpdateWaves()
		{
			foreach (var inst in debuggers)
			{
				inst.UpdateWaves();
			}
		}

		private void iOSettingsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormIOConfig ioconfig = new FormIOConfig(io);
			ioconfig.ShowDialog();
		}
	}
}
