using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using Be.Windows.Forms;
using System.Drawing.Drawing2D;

using PPUPlayerCustomClass;

namespace PPUPlayer
{
	public partial class Form1 : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		string? ppu_dump;
		string? nes_file;

		int logPointer = 0;
		byte[] logData = new byte[0];
		PPULogEntry? currentEntry;
		int recordCounter = 0;
		int CPUOpsProcessed = 0;
		int TotalOps = 0;

		bool Paused = false;

		bool PromptWhenFinished = true;

		long timeStamp;
		int pclkCounter;
		int scanCounter;
		int fieldCounter;
		int fieldCounterPersistent;

		int PrevH = -1;
		int PrevV = -1;

		List<BreaksCore.MemDesciptor> mem = new();
		DataHumanizer humanizer = new();

		bool TraceEnabled = false;
		int TraceMaxFields = 0;
		string TraceFilter = "";
		bool TraceCollapseSameRows = true;

		string DefaultTitle;

		public Form1()
		{
			InitializeComponent();
		}

		private void Form1_Load(object sender, EventArgs e)
		{
#if DEBUG
			AllocConsole();
#endif

#if !DEBUG
			debugToolStripMenuItem.Visible = false;
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
			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				ppu_dump = openFileDialog1.FileName;

				Console.WriteLine("The PPU registers dump is selected: " + ppu_dump);

				string text = DefaultTitle;
				text += " - " + ppu_dump;
				if (nes_file != null)
				{
					text += " + " + nes_file;
				}
				else
				{
					text += " + Need .nes ROM";
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

			if (ppu_dump == null || nes_file == null)
			{
				MessageBox.Show(
					"Before you start the simulation you need to select a PPU register dump and some .nes file, preferably with mapper 0.",
					"Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				return;
			}

			logData = File.ReadAllBytes(ppu_dump);
			logPointer = 0;
			TotalOps = logData.Length / 8;
			Console.WriteLine("Number of PPU Dump records: " + TotalOps.ToString());

			byte[] nes = File.ReadAllBytes(nes_file);

			FormSettings.PPUPlayerSettings settings = FormSettings.LoadSettings();

			string ppu_rev = settings.PPU_Revision == null ? "RP2C02G" : settings.PPU_Revision;

			PPUPlayerInterop.CreateBoard("PPUPlayer", "None", ppu_rev, "Fami");
			int res = PPUPlayerInterop.InsertCartridge(nes, nes.Length);
			if (res != 0)
			{
				PPUPlayerInterop.DestroyBoard();
				MessageBox.Show(
					"The cartridge didn't want to plug into the slot. Check that the .nes is intact and has a Mapper supported by the PPU Player.",
					"Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				return;
			}
			if (settings.ResetPPU)
			{
				PPUPlayerInterop.ResetPPU();
			}
			UpdateMemLayout();
			ResetVisualize();

			PPUPlayerInterop.RenderAlwaysEnabled(settings.RenderAlwaysEnabled);

			humanizer.SetColorDebugOutput(settings.ColorDebug);

			TraceEnabled = settings.TraceEnable;
			TraceMaxFields = settings.TraceMaxFields;
			if (settings.TraceFilter != null)
			{
				TraceFilter = settings.TraceFilter;
			}
			else
			{
				TraceFilter = "";
			}
			TraceCollapseSameRows = settings.TraceCollapseSameRows;
			SetTraceTimeResolutionNanos(settings.TraceTimeScale);
			ResetTrace(TraceMaxFields);

			currentEntry = NextLogEntry();

			if (currentEntry != null)
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
			PPUPlayerInterop.EjectCartridge();
			PPUPlayerInterop.DestroyBoard();
			ppu_dump = null;
			nes_file = null;

			Paused = false;
			toolStripButton3.Checked = false;
			toolStripButton3.Enabled = false;

			UpdateMemLayout();

			this.Text = DefaultTitle;
		}

		PPULogEntry? NextLogEntry()
		{
			PPULogEntry entry = new PPULogEntry();

			var bytesLeft = logData.Length - logPointer;
			if (bytesLeft < 8)
			{
				return null;
			}

			UInt32 pclkDelta = logData[logPointer + 3];
			pclkDelta <<= 8;
			pclkDelta |= logData[logPointer + 2];
			pclkDelta <<= 8;
			pclkDelta |= logData[logPointer + 1];
			pclkDelta <<= 8;
			pclkDelta |= logData[logPointer + 0];

			entry.pclk = PPUPlayerInterop.GetPCLKCounter() + (int)pclkDelta;
			entry.write = (logData[logPointer + 4] & 0x80) == 0 ? true : false;
			entry.reg = (byte)(logData[logPointer + 4] & 0x7);
			entry.value = logData[logPointer + 5];

			logPointer += 8;

			return entry;
		}

		public class PPULogEntry
		{
			public int pclk;
			public bool write;
			public byte reg;
			public byte value;
		}

		private void backgroundWorker1_DoWork_1(object sender, DoWorkEventArgs e)
		{
			while (!backgroundWorker1.CancellationPending)
			{
				if (Paused)
				{
					Thread.Sleep(10);
					continue;
				}

				// Check that the PPU Dump records have run out

				if (currentEntry == null)
				{
					Console.WriteLine("PPU Dump records are out.");

					UpdatePpuStats(PPUStats.CPU_IF_Ops, CPUOpsProcessed);
					UpdatePpuStats(PPUStats.Scans, scanCounter);
					UpdatePpuStats(PPUStats.Fields, fieldCounterPersistent);

					DisposeBoard();

					if (PromptWhenFinished)
					{
						MessageBox.Show(
							"The PPU Player has finished executing the current batch.",
							"Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
					}

					return;
				}

				// Check that it is time to perform a CPU operation

				if (PPUPlayerInterop.GetPCLKCounter() == currentEntry.pclk)
				{
					if (currentEntry.write)
					{
						PPUPlayerInterop.CPUWrite(currentEntry.reg, currentEntry.value);
					}
					else
					{
						PPUPlayerInterop.CPURead(currentEntry.reg);
					}

					currentEntry = NextLogEntry();

					CPUOpsProcessed++;
					recordCounter++;
					if (recordCounter > 1000)
					{
						recordCounter = 0;
						UpdatePpuStats(PPUStats.CPU_IF_Ops, CPUOpsProcessed);
					}
				}

				// Perform one half-cycle of the PPU

				PPUPlayerInterop.Step();

				if (TraceEnabled)
				{
					ProcessTrace(scanCounter);
				}

				// Logic related to the processing of H/V values

				int h = PPUPlayerInterop.GetHCounter();
				int v = PPUPlayerInterop.GetVCounter();

				if (h != PrevH)
				{
					if (h == 0 && PrevH > 0)
					{
						// Next Scan
						scanCounter++;
					}

					//UpdatePpuStats(PPUStats.HCounter, h);
					PrevH = h;
				}

				// TBD: At this point, without resetting the PPU, the stats may go a little off, as the V value goes crazy, after the PPU starts.
				// After the first "warm-up" Scan, the V value settles down.
				// If a PPU reset is done, the above effect does not occur.

				if (v != PrevV)
				{
					if (v == 0 && PrevV > 0)
					{
						// Next Field

						fieldCounter++;
						fieldCounterPersistent++;
					}

					//UpdatePpuStats(PPUStats.VCounter, v);
					PrevV = v;
				}

				// Get a single sample of the video signal. If the PPU is in the process of resetting - do not count samples at that moment.

				if (PPUPlayerInterop.PPUInResetState() == 0)
				{
					float sample;
					PPUPlayerInterop.SampleVideoSignal(out sample);
					ProcessSample(sample);
				}

				// Show statistics that are updated once every 1 second.

				long now = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
				if (now > (timeStamp + 1000))
				{
					timeStamp = now;

					UpdatePpuStats(PPUStats.PCLK_Sec, PPUPlayerInterop.GetPCLKCounter() - pclkCounter);
					UpdatePpuStats(PPUStats.FPS, fieldCounter);

					UpdatePpuStats(PPUStats.Scans, scanCounter);
					UpdatePpuStats(PPUStats.Fields, fieldCounterPersistent);

					pclkCounter = PPUPlayerInterop.GetPCLKCounter();
					fieldCounter = 0;
				}
			}

			Console.WriteLine("Background Worker canceled.");
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
			recordCounter = 0;
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
			}
		}

		private void testStatsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			UpdatePpuStats(PPUStats.CPU_IF_Ops, 1);
			UpdatePpuStats(PPUStats.Scans, 2);
			UpdatePpuStats(PPUStats.Fields, 3);
			UpdatePpuStats(PPUStats.PCLK_Sec, 4);
			UpdatePpuStats(PPUStats.FPS, 5);
		}

		private void testDebugPropertyGridToolStripMenuItem_Click(object sender, EventArgs e)
		{
			List<BreaksCore.DebugInfoEntry> entries = BreaksCore.GetTestDebugInfo();
			UpdateDebugInfo(entries);
		}

		private void testHexBoxToolStripMenuItem_Click(object sender, EventArgs e)
		{
			byte[] dump = new byte[0x100];
			for (int i=0; i<0x100; i++)
			{
				dump[i] = (byte)i;
			}
			hexBox1.ByteProvider = new DynamicByteProvider(dump);
			hexBox1.Refresh();
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

	}
}
