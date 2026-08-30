// The worker is simple. Call `Step' on the motherboard and render the audio/video signal from it

using SharpTools;

namespace Breaknes
{
	public partial class FormMain : Form
	{
		private void backgroundWorker1_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
		{
			while (!backgroundWorker1.CancellationPending)
			{
				if (board.Paused)
				{
					Thread.Sleep(10);
					continue;
				}
				if (snd_out.Dma)
				{
					Thread.Sleep(1);
					continue;
				}

				BreaksCore.Step();

				if (!BreaksCore.InResetState())
				{
					// Sample and render the video signal of every connected TV Set.
					foreach (var vr in tv_renders)
					{
						BreaksCore.VideoOutSample sample;
						BreaksCore.SampleVideoSignalEx(vr.TvIndex, out sample);
						vr.ProcessSample(sample);
					}

					if (snd_out != null)
						snd_out.FeedSample();
				}
			}
		}
	}
}
