namespace SignalPlotDemo
{
	public partial class FormMain : Form
	{
		public FormMain()
		{
			InitializeComponent();
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void normalizedNoiseToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Random rnd = new Random();
			float[] samples = new float[100];
			for (int i=0; i<samples.Length; i++)
			{
				samples[i] = (float)rnd.NextDouble();
			}
			signalPlot1.PlotSignal(samples);
		}

		private void normalizedSineWaveToolStripMenuItem_Click(object sender, EventArgs e)
		{
			float[] samples = new float[300];
			for (int i=0; i<samples.Length; i++)
			{
				samples[i] = (float)Math.Sin (i / Math.PI);
			}
			signalPlot1.PlotSignal(samples);
		}

		private void loadFloatsDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void saveFloatsDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}
	}
}
