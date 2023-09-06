
namespace Breaknes
{
	public partial class FormWaitInputEvent : Form
	{
		public string? bindstr = null;
		public int saved_actuator_id;
		IOProcessor io;

		public FormWaitInputEvent(int actuator_id, IOProcessor io)
		{
			InitializeComponent();
			this.io = io;
			saved_actuator_id = actuator_id;
		}

		private void FormWaitInputEvent_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				backgroundWorker1.CancelAsync();
				bindstr = null;
				Close();
			}
		}

		private void backgroundWorker1_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
		{
			while(!backgroundWorker1.CancellationPending)
			{
				io.PollDevices();

				if (io.event_queue.Count != 0)
				{
					bindstr = io.event_queue[0].name;
					io.event_queue.Clear();
					Close();
				}
			}
		}

		private void FormWaitInputEvent_Load(object sender, EventArgs e)
		{
			backgroundWorker1.RunWorkerAsync();
		}
	}
}
