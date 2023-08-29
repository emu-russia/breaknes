
namespace Breaknes
{
	public partial class FormWaitInputEvent : Form
	{
		public string? bindstr = "Dummy binding";
		public int saved_actuator_id;
		IOProcessor io;

		public FormWaitInputEvent(int actuator_id, IOProcessor io)
		{
			InitializeComponent();
			this.io = io;
			saved_actuator_id = actuator_id;
		}
	}
}
