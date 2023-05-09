
namespace SharpTools
{
	public partial class FormAddBreakpoint : Form
	{
		public Breakpoint bp = new Breakpoint();
		public bool bp_added = false;

		public FormAddBreakpoint()
		{
			InitializeComponent();
			breakpointProps1.SetEnabled();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			bp = breakpointProps1.ControlsToBreakpoint();
			bp_added = true;
			Close();
		}
	}
}
