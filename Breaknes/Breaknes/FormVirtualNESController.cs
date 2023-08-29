
namespace Breaknes
{
	public partial class FormVirtualNESController : Form
	{
		IOProcessor io;

		public FormVirtualNESController(IOConfigDevice device, IOProcessor io)
		{
			InitializeComponent();
			this.io = io;
		}

		private void checkBox1_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Up";
			io_event.value = (UInt32)(checkBox1.Checked ? 1 : 0);
			io.PushEvent(io_event);
		}

		private void checkBox4_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Down";
			io_event.value = (UInt32)(checkBox4.Checked ? 1 : 0);
			io.PushEvent(io_event);
		}

		private void checkBox2_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Left";
			io_event.value = (UInt32)(checkBox2.Checked ? 1 : 0);
			io.PushEvent(io_event);
		}

		private void checkBox3_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Right";
			io_event.value = (UInt32)(checkBox3.Checked ? 1 : 0);
			io.PushEvent(io_event);
		}

		private void checkBox5_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Select";
			io_event.value = (UInt32)(checkBox5.Checked ? 1 : 0);
			io.PushEvent(io_event);
		}

		private void checkBox6_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Start";
			io_event.value = (UInt32)(checkBox6.Checked ? 1 : 0);
			io.PushEvent(io_event);
		}

		private void checkBox7_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "B";
			io_event.value = (UInt32)(checkBox7.Checked ? 1 : 0);
			io.PushEvent(io_event);
		}

		private void checkBox8_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "A";
			io_event.value = (UInt32)(checkBox8.Checked ? 1 : 0);
			io.PushEvent(io_event);
		}
	}
}
