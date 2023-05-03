using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using System.IO;
using System.Runtime.InteropServices;

using Be.Windows.Forms;
using SharpToolsCustomClass;
using SharpTools;
using System.ComponentModel.Design;

namespace PPUPlayer
{
	public partial class FormMain : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		string? ppu_dump;
		string? nes_file;

		int CPUOpsProcessed = 0;
		int TotalOps = 0;

		bool PromptWhenFinished = true;

		long timeStamp;
		long pclkCounter;
		int scanCounter;
		int fieldCounter;
		int fieldCounterPersistent;

		int PrevH = -1;
		int PrevV = -1;

		List<BreaksCore.MemDesciptor> mem = new();
		DataHumanizer humanizer = new();

		string DefaultTitle;

		bool SimulationStarted = false;

		public FormMain()
		{
			InitializeComponent();
		}

		private void Form1_Load(object sender, EventArgs e)
		{
#if DEBUG
			AllocConsole();
#endif

			pictureBoxField.BackColor = Color.Gray;
			toolStripButton3.Enabled = false;
			comboBox2.SelectedIndex = 0;

			DefaultTitle = this.Text;
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

		private void choosePPURegsDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialogRegdump.ShowDialog() == DialogResult.OK)
			{
				ppu_dump = openFileDialogRegdump.FileName;

				Console.WriteLine("The PPU registers dump is selected: " + ppu_dump);

				string text = DefaultTitle;
				text += " - " + ppu_dump;
				if (nes_file != null)
				{
					text += " + " + nes_file;
				}
				else
				{
					text += " + Dummy NROM";
				}
				this.Text = text;
			}
		}

		private void choosenesImageToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialog2.ShowDialog() == DialogResult.OK)
			{
				nes_file = openFileDialog2.FileName;

				Console.WriteLine("The .nes file has been selected: " + nes_file);

				string text = DefaultTitle;
				text += " - ";
				if (ppu_dump != null)
				{
					text += ppu_dump;
				}
				else
				{
					text += "No RegDump";
				}
				text += " + " + nes_file;
				this.Text = text;
			}
		}

		private void runToolStripMenuItem_Click(object sender, EventArgs e)
		{
			RunPPU();
		}

		private void toolStripButton1_Click(object sender, EventArgs e)
		{
			RunPPU();
		}

		private void stopPPUAndUnloadToolStripMenuItem_Click(object sender, EventArgs e)
		{
			StopPPU();
		}

		private void toolStripButton2_Click(object sender, EventArgs e)
		{
			StopPPU();
		}

		void RunPPU()
		{
			if (backgroundWorker1.IsBusy)
			{
				Console.WriteLine("Background Worker is already running.");
				return;
			}

			FormSettings.PPUPlayerSettings settings = FormSettings.LoadSettings();

			// If the user did not select anything, but just clicked the `Play` button - notify him that he is in free flight.

			if (ppu_dump == null && nes_file == null)
			{
				string text = DefaultTitle;
				text += " - Free Flight (" + settings.PPU_Revision + ")";
				this.Text = text;
			}

			// If the user has specified RegDump - use it. Otherwise, create a dummy RegDump with an `infinite` wait to read register $2002.

			byte[] reg_dump;

			if (ppu_dump != null)
			{
				if (Path.GetExtension(ppu_dump).ToLower() == ".hex")
				{
					reg_dump = LogisimHEXConv.HEXToByteArray(File.ReadAllText(ppu_dump));
				}
				else
				{
					reg_dump = File.ReadAllBytes(ppu_dump);
				}
				TotalOps = reg_dump.Length / 8;
				Console.WriteLine("Number of RegDump records: " + TotalOps.ToString());
			}
			else
			{
				reg_dump = new byte[] { 0xff, 0xff, 0xff, 0x7f, 0x82, 0x00, 0x00, 0x00 };
				TotalOps = 1;
				Console.WriteLine("Created one dummy read of register $2002 through an `infinite` number of cycles.");
			}

			// If the user specified a .nes file - use it. Otherwise, use Dummy NROM.

			byte[] nes;
			if (nes_file != null)
			{
				nes = File.ReadAllBytes(nes_file);
			}
			else
			{
				DummyNrom nrom = new DummyNrom(settings.FreeModeVMirroring);
				nes = nrom.GetNesImage();
			}

			string ppu_rev = settings.PPU_Revision == null ? "RP2C02G" : settings.PPU_Revision;

			BreaksCore.CreateBoard("PPUPlayer", "RP2A03G", ppu_rev, "Fami");
			int res = BreaksCore.InsertCartridge(nes, nes.Length);
			if (res != 0)
			{
				BreaksCore.DestroyBoard();
				MessageBox.Show(
					"The cartridge didn't want to plug into the slot. Check that the .nes is intact and has a Mapper supported by the PPU Player.",
					"Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				return;
			}
			if (settings.ResetPPU)
			{
				BreaksCore.Reset();
			}
			BreaksCore.SetOamDecayBehavior(settings.OAMDecay);
			BreaksCore.SetNoiseLevel(settings.PpuNoise);
			BreaksCore.LoadRegDump(reg_dump, reg_dump.Length);
			UpdateMemLayout();

			ResetVisualize(settings.PpuRAWMode);

			if (ppu_dump == null)
			{
				// In free mode there is no one to enable PPU rendering, so we do it forcibly.

				BreaksCore.RenderAlwaysEnabled(true);
			}
			else
			{
				BreaksCore.RenderAlwaysEnabled(settings.RenderAlwaysEnabled);
			}

			humanizer.SetColorDebugOutput(settings.ColorDebug);

			// Set the next CPU operation and start the simulation.

			if (TotalOps != 0)
			{
				ResetPpuStats();
				toolStripButton3.Enabled = true;
				backgroundWorker1.RunWorkerAsync();
			}
			else
			{
				MessageBox.Show(
					"The trace history of PPU register accesses does not contain any data.",
					"Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
			}

			BreaksCore.VideoSignalFeatures features;
			BreaksCore.GetPpuSignalFeatures(out features);
			//wavesControl1.EnabledDottedEveryNth(features.SamplesPerPCLK, true);

			SimulationStarted = true;
		}

		void StopPPU()
		{
			if (backgroundWorker1.IsBusy)
			{
				backgroundWorker1.CancelAsync();
				DisposeBoard();
			}
			else
			{
				Console.WriteLine("Background Worker is not running or has already completed its activity.");
			}
		}

		void DisposeBoard()
		{
			BreaksCore.EjectCartridge();
			BreaksCore.DestroyBoard();
			ppu_dump = null;
			nes_file = null;

			Paused = false;
			toolStripButton3.Checked = false;
			toolStripButton3.Enabled = false;

			UpdateMemLayout();

			this.Text = DefaultTitle;

			SimulationStarted = false;

			signalPlotScan.EnableSelection(false);

			wavesControl1.EnableSelection(false);
			ResetWaves();
			waves = new();
		}


		enum PPUStats
		{
			CPU_IF_Ops,
			Scans,
			Fields,
			PCLK_Sec,
			FPS,
		}

		void ResetPpuStats()
		{
			CPUOpsProcessed = 0;

			timeStamp = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
			scanCounter = 0;
			fieldCounter = 0;
			fieldCounterPersistent = 0;
			pclkCounter = 0;

			PrevH = -1;
			PrevV = -1;

			UpdatePpuStats(PPUStats.CPU_IF_Ops, 0);
			UpdatePpuStats(PPUStats.Scans, 0);
			UpdatePpuStats(PPUStats.Fields, 0);
			UpdatePpuStats(PPUStats.PCLK_Sec, 0);
			UpdatePpuStats(PPUStats.FPS, 0);
		}

		void UpdatePpuStats(PPUStats stats, int value)
		{
			switch (stats)
			{
				case PPUStats.CPU_IF_Ops:
					toolStripStatusLabel6.Text = value.ToString() + "/" + TotalOps.ToString();
					break;
				case PPUStats.Scans:
					toolStripStatusLabel7.Text = value.ToString();
					break;
				case PPUStats.Fields:
					toolStripStatusLabel8.Text = value.ToString();
					break;
				case PPUStats.PCLK_Sec:
					toolStripStatusLabel12.Text = value.ToString();
					break;
				case PPUStats.FPS:
					toolStripStatusLabel14.Text = value.ToString();
					break;
			}
		}

		private void toolStripButton3_Click(object sender, EventArgs e)
		{
			if (backgroundWorker1.IsBusy)
			{
				if (!Paused)
				{
					Paused = true;
					toolStripButton3.Checked = true;
				}
				else
				{
					Paused = false;
					toolStripButton3.Checked = false;
				}
				signalPlotScan.EnableSelection(Paused);
				wavesControl1.EnableSelection(Paused);
			}
		}

		void Button2Click()
		{
			List<BreaksCore.DebugInfoEntry> entries = BreaksCore.GetDebugInfo(ComboBoxToDebugInfoType());
			UpdateDebugInfo(entries);
		}

		/// <summary>
		/// Update DebugInfo
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void button2_Click(object sender, EventArgs e)
		{
			Button2Click();
		}

		BreaksCore.DebugInfoType ComboBoxToDebugInfoType()
		{
			switch (comboBox2.SelectedIndex)
			{
				case 0:
					return BreaksCore.DebugInfoType.DebugInfoType_PPU;
				case 1:
					return BreaksCore.DebugInfoType.DebugInfoType_PPURegs;
				case 2:
					return BreaksCore.DebugInfoType.DebugInfoType_Board;
				case 3:
					return BreaksCore.DebugInfoType.DebugInfoType_Cart;
			}

			return BreaksCore.DebugInfoType.DebugInfoType_Test;
		}

		void Button1Click()
		{
			if (mem.Count == 0)
			{
				hexBox1.ByteProvider = new DynamicByteProvider(new byte[0]);
				hexBox1.Refresh();
				pictureBoxForHuman.Image = null;
				pictureBoxForHuman.Invalidate();
				return;
			}

			int descrID = comboBox1.SelectedIndex;

			if (pictureBoxForHuman.Visible)
			{
				string descrName = mem[descrID].name;

				Bitmap bitmap = humanizer.ConvertHexToImage(descrName);
				pictureBoxForHuman.Image = bitmap;
				pictureBoxForHuman.Invalidate();
			}
			else
			{
				byte[] buf = new byte[mem[descrID].size];

				BreaksCore.DumpMem(descrID, buf);
				hexBox1.ByteProvider = new DynamicByteProvider(buf);
				hexBox1.Refresh();
			}
		}

		/// <summary>
		/// Dump Mem
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void button1_Click(object sender, EventArgs e)
		{
			Button1Click();
		}

		bool UpdateMemLayoutInProgress = false;

		/// <summary>
		/// Get a set of memory regions from the debugger and fill the ComboBox.
		/// </summary>
		void UpdateMemLayout()
		{
			UpdateMemLayoutInProgress = true;

			comboBox1.Items.Clear();

			mem = BreaksCore.GetMemoryLayout();

			foreach (var descr in mem)
			{
				comboBox1.Items.Add(descr.name);
			}

			if (mem.Count != 0)
			{
				comboBox1.SelectedIndex = 0;
			}

			UpdateMemLayoutInProgress = false;
		}

		void UpdateDebugInfo(List<BreaksCore.DebugInfoEntry> entries)
		{
			// Construct an object to show in PropertyGrid

			CustomClass myProperties = new CustomClass();

			foreach (BreaksCore.DebugInfoEntry entry in entries)
			{
				CustomProperty myProp = new CustomProperty();
				myProp.Name = entry.name;
				myProp.Category = entry.category;
				myProp.Value = entry.value;
				myProperties.Add(myProp);
			}

			propertyGrid1.SelectedObject = myProperties;
		}

		private void pictureBoxForHuman_Paint(object sender, PaintEventArgs e)
		{
			// https://stackoverflow.com/questions/54720916/disable-image-blending-on-a-picturebox

			//e.Graphics.InterpolationMode = InterpolationMode.NearestNeighbor;

			//e.Graphics.PixelOffsetMode = PixelOffsetMode.Half;

			//e.Graphics.DrawImage(pixelBitmap, GetScaledImageRect(pixelBitmap, DirectCast(sender, Control)));
		}

		private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormSettings formSettings = new();
			formSettings.FormClosed += FormSettings_FormClosed;
			formSettings.ShowDialog();
		}

		/// <summary>
		/// Apply settings that can be changed during the simulation.
		/// </summary>
		private void FormSettings_FormClosed(object? sender, FormClosedEventArgs e)
		{
			FormSettings.PPUPlayerSettings settings = FormSettings.LoadSettings();

			humanizer.SetColorDebugOutput(settings.ColorDebug);
			BreaksCore.SetOamDecayBehavior(settings.OAMDecay);
			if (SimulationStarted)
			{
				// TBD: Works unstable, sometimes loses phase (need to make a neater PLL)
				ResetVisualize(settings.PpuRAWMode);
			}
		}

		private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
		{
			Button2Click();
		}

		private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (UpdateMemLayoutInProgress)
				return;

			Button1Click();
		}

		private void OpenUrl(string url)
		{
			// https://stackoverflow.com/questions/4580263/how-to-open-in-default-browser-in-c-sharp

			try
			{
				System.Diagnostics.Process.Start(url);
			}
			catch
			{
				// hack because of this: https://github.com/dotnet/corefx/issues/10361
				if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
				{
					url = url.Replace("&", "^&");
					System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(url) { UseShellExecute = true });
				}
				else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
				{
					System.Diagnostics.Process.Start("xdg-open", url);
				}
				else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
				{
					System.Diagnostics.Process.Start("open", url);
				}
				else
				{
					throw;
				}
			}
		}

		private void sendFeedbackToolStripMenuItem_Click(object sender, EventArgs e)
		{
			OpenUrl("https://github.com/emu-russia/breaknes/issues");
		}

		private void checkForUpdatesToolStripMenuItem_Click(object sender, EventArgs e)
		{
			OpenUrl("https://github.com/emu-russia/breaknes/releases");
		}

		private void pPUColorSpaceToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormSettings.PPUPlayerSettings settings = FormSettings.LoadSettings();
			FormColorSpace formColorSpace = new FormColorSpace(settings.PPU_Revision);
			formColorSpace.Show();
		}

		/// <summary>
		/// Used to change the value of PPU registers on the fly. Use for Free Mode.
		/// </summary>
		private void propertyGrid1_PropertyValueChanged(object s, PropertyValueChangedEventArgs e)
		{
			if (e.ChangedItem != null)
			{
				BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_PPURegs,
					"PPU Regs", e.ChangedItem.Label, (UInt32)e.ChangedItem.Value);
			}
		}

		private void loadRAWDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
			{
				MessageBox.Show(
					"You need to run the simulation so that the PPU instance is active.",
					"Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				return;
			}

			if (openFileDialogHEX.ShowDialog() == DialogResult.OK)
			{
				FormRawViewer formRawViewer = new FormRawViewer(openFileDialogHEX.FileName);
				formRawViewer.Show();
			}
		}

		private void loadCompositeDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
			{
				MessageBox.Show(
					"You need to run the simulation so that the PPU instance is active.",
					"Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				return;
			}

			if (openFileDialogHEX.ShowDialog() == DialogResult.OK)
			{
				FormCompositeViewer formCompositeViewer = new FormCompositeViewer(openFileDialogHEX.FileName);
				formCompositeViewer.Show();
			}
		}

		private void loadRGBDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
			{
				MessageBox.Show(
					"You need to run the simulation so that the PPU instance is active.",
					"Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				return;
			}

			if (openFileDialogHEX.ShowDialog() == DialogResult.OK)
			{
				FormComponentViewer formComponentViewer = new FormComponentViewer(openFileDialogHEX.FileName);
				formComponentViewer.Show();
			}
		}

		/// <summary>
		/// Debug Step. Only in Paused mode
		/// </summary>
		private void DoDebugStep()
		{
			if (SimulationStarted && Paused && !DebugStep)
			{
				DebugStep = true;
			}
		}

		private void button3_Click_1(object sender, EventArgs e)
		{
			DoDebugStep();
		}

		private void FormMain_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.F11)
			{
				DoDebugStep();
			}
		}

	}
}
