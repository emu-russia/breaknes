// Worker is put in a separate module, because it is the main essence of all the processes of the application and it is convenient to study and store it in isolation.

using System.DirectoryServices.ActiveDirectory;
using SharpTools;

namespace NSFPlayer
{
	public partial class FormMain : Form
	{
		private int StepsToStat = 32;
		private int StepsCounter = 0;

		private void backgroundWorker1_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
		{
			while (!backgroundWorker1.CancellationPending)
			{
				if (Paused || !(nsf_loaded || regdump_loaded || auxdump_loaded) || Dma)
				{
					Thread.Sleep(10);
					continue;
				}

				// Simulate APU

				if (auxdump_loaded)
				{
					Thread.SpinWait(10);
				}
				else
				{
					NSFPlayerInterop.Step();        // = 0.5 XTAL CLK
				}

				if (nsf_loaded)
				{
					if (nsf.IsCoreReady())
						nsf.SyncExec();
				}

				// Add audio sample

				FeedSample();

				// NSF Runtime logic

				if (nsf_loaded)
				{
					var aclk = NSFPlayerInterop.GetACLKCounter();
					if (aclk >= AclkToPlay)
					{
						ExecPLAY();
						var clk_per_second = aux_features.SampleRate / 2;
						var aclk_per_second = (clk_per_second / 12) / 2;
						AclkToPlay = aclk + (aclk_per_second * nsf.GetPeriod(PreferPal)) / 1000000;
					}

					if (InitRequired)
					{
						ExecINIT();
						InitRequired = false;
					}
				}

				// Show statistics that are updated once every 1 second.

				StepsCounter++;
				if (StepsCounter >= StepsToStat)
				{
					long now = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
					if (now > (timeStamp + 1000))
					{
						timeStamp = now;

						UpdateSampleBufStats();
						UpdateSignalPlot();

						var aclk_per_sec = NSFPlayerInterop.GetACLKCounter() - aclkCounter;
						toolStripStatusLabelACLK.Text = aclk_per_sec.ToString();

						aclkCounter = NSFPlayerInterop.GetACLKCounter();
					}
					StepsCounter = 0;
				}
			}
		}

	}
}
