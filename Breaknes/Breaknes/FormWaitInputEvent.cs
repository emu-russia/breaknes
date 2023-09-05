
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
				Close();
			}
		}
	}
}
