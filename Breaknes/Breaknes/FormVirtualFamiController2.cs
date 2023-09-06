
namespace Breaknes
{
	public partial class FormVirtualFamiController2 : Form
	{
		IOProcessor io;
		int handle;

		public FormVirtualFamiController2(IOConfigDevice device, int handle, IOProcessor io)
		{
			InitializeComponent();
			this.io = io;
			this.handle = handle;
			this.Text += " " + device.name;
		}

		private void checkBox1_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Fami2_Up";
			io_event.value = (UInt32)(checkBox1.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox4_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Fami2_Down";
			io_event.value = (UInt32)(checkBox4.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox2_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Fami2_Left";
			io_event.value = (UInt32)(checkBox2.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox3_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Fami2_Right";
			io_event.value = (UInt32)(checkBox3.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox7_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Fami2_B";
			io_event.value = (UInt32)(checkBox7.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void checkBox8_CheckedChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Fami2_A";
			io_event.value = (UInt32)(checkBox8.Checked ? 1 : 0);
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void trackBar1_ValueChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Fami2_Volume";
			io_event.value = (UInt32)trackBar1.Value;
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}

		private void trackBar2_ValueChanged(object sender, EventArgs e)
		{
			IOEvent io_event = new();
			io_event.name = "Fami2_MicLevel";
			io_event.value = (UInt32)trackBar2.Value;
			io_event.device_handle = handle;
			io.PushEvent(io_event);
		}
	}
}
