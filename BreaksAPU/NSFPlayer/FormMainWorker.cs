// Worker is put in a separate module, because it is the main essence of all the processes of the application and it is convenient to study and store it in isolation.

using SharpTools;

namespace APUPlayer
{
	public partial class FormMain : Form
	{
		private int StepsToStat = 32;
		private int StepsCounter = 0;

		private void backgroundWorker1_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
		{
			while (!backgroundWorker1.CancellationPending)
			{
				if (Paused || !(regdump_loaded || auxdump_loaded) || Dma)
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
					BreaksCore.Step();        // = 0.5 XTAL CLK
				}

				// Add audio sample

				FeedSample();

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

						var aclk_per_sec = BreaksCore.GetACLKCounter() - aclkCounter;
						toolStripStatusLabelACLK.Text = aclk_per_sec.ToString();

						aclkCounter = BreaksCore.GetACLKCounter();
						toolStripStatusLabelACLKCount.Text = aclkCounter.ToString();
					}
					StepsCounter = 0;
				}
			}
		}

	}
}
