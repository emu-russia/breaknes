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

		List<BreaksCore.MemDesciptor> mem = new List<BreaksCore.MemDesciptor>();

		public Form1()
		{
			InitializeComponent();
		}

		private void Form1_Load(object sender, EventArgs e)
		{
#if DEBUG
			AllocConsole();
#endif

			pictureBox1.BackColor = Color.Gray;
			toolStripButton3.Enabled = false;
			comboBox2.SelectedIndex = 0;
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
			}
		}

		private void choosenesImageToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialog2.ShowDialog() == DialogResult.OK)
			{
				nes_file = openFileDialog2.FileName;

				Console.WriteLine("The .nes file has been selected: " + nes_file);
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
			TotalOps = logData.Length / 4;
			Console.WriteLine("Number of PPU Dump records: " + TotalOps.ToString());

			byte[] nes = File.ReadAllBytes(nes_file);

			PPUPlayerInterop.CreateBoard("PPUPlayer", "None", "RP2C02G", "Fami");
			PPUPlayerInterop.InsertCartridge(nes, nes.Length);
			UpdateMemLayout();

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
		}

		PPULogEntry? NextLogEntry()
		{
			PPULogEntry entry = new PPULogEntry();

			var bytesLeft = logData.Length - logPointer;
			if (bytesLeft < 4)
			{
				return null;
			}

			UInt16 pclkDelta = logData[logPointer + 1];
			pclkDelta <<= 8;
			pclkDelta |= logData[logPointer + 0];

			entry.pclk = PPUPlayerInterop.GetPCLKCounter() + pclkDelta;
			entry.write = (logData[logPointer + 2] & 0x80) == 0 ? true : false;
			entry.reg = (byte)(logData[logPointer + 2] & 0x7);
			entry.value = logData[logPointer + 3];

			logPointer += 4;

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

				// Show statistics that are updated once every 1 second.

				float sample;
				PPUPlayerInterop.SampleVideoSignal(out sample);
				ProcessSample(sample);

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

		void ProcessSample(float sample)
		{

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

		private void testFieldToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void testScanToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void testDebugPropertyGridToolStripMenuItem_Click(object sender, EventArgs e)
		{

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

		/// <summary>
		/// Update DebugInfo
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
        private void button2_Click(object sender, EventArgs e)
        {

        }

		/// <summary>
		/// Dump Mem
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
        private void button1_Click(object sender, EventArgs e)
        {
			if (mem.Count == 0)
				return;

			int descrID = comboBox1.SelectedIndex;

			byte[] buf = new byte[mem[descrID].size];

			BreaksCore.DumpMem(descrID, buf);
			hexBox1.ByteProvider = new DynamicByteProvider(buf);
			hexBox1.Refresh();
		}

		/// <summary>
		/// Get a set of memory regions from the debugger and fill the ComboBox.
		/// </summary>
		void UpdateMemLayout()
        {
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
		}
    }
}
