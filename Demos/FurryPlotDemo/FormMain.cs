using System.Threading;

namespace FurryPlotDemo
{
	public partial class FormMain : Form
	{
		public FormMain()
		{
			InitializeComponent();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
			backgroundWorker1.RunWorkerAsync();
		}

		private void backgroundWorker1_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
		{
			Random rnd = new Random();

			float x = 0;

			while (true)
			{
				furryPlot1.AddSample((float)Math.Sin(x) + (float)Math.Sin(16 * x));
				x += 0.1f;
			}
		}
	}
}