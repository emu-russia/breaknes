// Background Worker is separated for easier navigation, study and debugging.

using System;
using System.ComponentModel;
using System.Windows.Forms;
using System.Threading;
using SharpTools;

namespace PPUPlayer
{
	public partial class FormMain : Form
	{
		private int StepsToStat = 32;
		private int StepsCounter = 0;

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

				if (CPUOpsProcessed >= TotalOps)
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

				// Perform one half-cycle of the PPU

				BreaksCore.Step();

				if (TraceEnabled)
				{
					ProcessTrace(scanCounter);
				}

				// Logic related to the processing of H/V values

				int h = BreaksCore.GetHCounter();
				int v = BreaksCore.GetVCounter();

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

				if (!BreaksCore.InResetState())
				{
					BreaksCore.VideoOutSample sample;
					BreaksCore.SampleVideoSignal(out sample);
					ProcessSample(sample);
				}

				// Show statistics that are updated once every 1 second.

				StepsCounter++;
				if (StepsCounter >= StepsToStat)
				{
					long now = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
					if (now > (timeStamp + 1000))
					{
						timeStamp = now;

						CPUOpsProcessed = (int)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "CPUOpsProcessed");
						UpdatePpuStats(PPUStats.CPU_IF_Ops, CPUOpsProcessed);

						UpdatePpuStats(PPUStats.PCLK_Sec, (int)(BreaksCore.GetPCLKCounter() - pclkCounter));
						UpdatePpuStats(PPUStats.FPS, fieldCounter);

						UpdatePpuStats(PPUStats.Scans, scanCounter);
						UpdatePpuStats(PPUStats.Fields, fieldCounterPersistent);

						pclkCounter = BreaksCore.GetPCLKCounter();
						fieldCounter = 0;
					}
					StepsCounter = 0;
				}
			}

			Console.WriteLine("Background Worker canceled.");
		}

	}
}
