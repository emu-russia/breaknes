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
					BreaksCore.VideoOutSample sample;
					BreaksCore.SampleVideoSignal(out sample);
					vid_out.ProcessSample(sample);

					if (snd_out != null)
						snd_out.FeedSample();
				}
			}
		}
	}
}
