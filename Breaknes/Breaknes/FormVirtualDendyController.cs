
namespace Breaknes
{
	public partial class FormVirtualDendyController : Form
	{
		IOProcessor io;
		int handle;

		public FormVirtualDendyController(IOConfigDevice device, int handle, IOProcessor io)
		{
			InitializeComponent();
			this.io = io;
			this.handle = handle;
			this.Text += " " + device.name;
		}

		private void checkBox1_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_Up";
			io_event.value = (UInt32)(checkBox1.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox4_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_Down";
			io_event.value = (UInt32)(checkBox4.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox2_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_Left";
			io_event.value = (UInt32)(checkBox2.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox3_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_Right";
			io_event.value = (UInt32)(checkBox3.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox5_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_Select";
			io_event.value = (UInt32)(checkBox5.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox6_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_Start";
			io_event.value = (UInt32)(checkBox6.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox10_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_TurboB";
			io_event.value = (UInt32)(checkBox10.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox9_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_TurboA";
			io_event.value = (UInt32)(checkBox9.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox7_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_B";
			io_event.value = (UInt32)(checkBox7.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox8_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Dendy_A";
			io_event.value = (UInt32)(checkBox8.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}
	}
}
