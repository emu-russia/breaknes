
using SharpTools;

namespace Breaknes
{
	public partial class FormMain : Form
	{
		public bool Paused = true;

		private void backgroundWorker1_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
		{
			while (!backgroundWorker1.CancellationPending)
			{
				if (Paused)
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
