
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

				BreaksCore.Step();

				if (!BreaksCore.InResetState())
				{
					BreaksCore.VideoOutSample sample;
					BreaksCore.SampleVideoSignal(out sample);
					vid_out.ProcessSample(sample);
				}

			}
		}
	}
}
