
namespace SharpTools
{
	public partial class BreakpointsPanel : UserControl
	{
		private List<Breakpoint> breakpoints = new List<Breakpoint>();
		private bool any_enabled = false;

		public delegate void OnBreakpointTrigger(Breakpoint bp);
		public OnBreakpointTrigger? onBreakpointTrigger = null;

		public BreakpointsPanel()
		{
			InitializeComponent();
		}

		public void Reset()
		{
			breakpoints = new List<Breakpoint>();
			listView1.Items.Clear();
			any_enabled = false;
		}

		public void CheckBreakpoints()
		{
			if (!any_enabled)
				return;

			foreach (var breakpoint in breakpoints)
			{
				if (breakpoint.enabled)
				{
					var value = BreaksCore.GetDebugInfoByName(breakpoint.info_type, breakpoint.info_entry.category, breakpoint.info_entry.name);
					bool triggered = false;

					switch (breakpoint.trigger)
					{
						case BreakpontTrigger.High:
							triggered = value == 1;
							break;

						case BreakpontTrigger.Low:
							triggered = value == 0;
							break;

						case BreakpontTrigger.Z:
							triggered = value == 0xff;
							break;

						case BreakpontTrigger.Posedge:
							triggered = (breakpoint.prev_value == 0 && value == 1);
							breakpoint.prev_value = value;
							break;

						case BreakpontTrigger.Negedge:
							triggered = (breakpoint.prev_value == 1 && value == 0);
							breakpoint.prev_value = value;
							break;

						case BreakpontTrigger.VectorEqual:
							triggered = value == breakpoint.vector_value;
							break;
					}

					if (triggered)
					{
						onBreakpointTrigger?.Invoke(breakpoint);
					}

					if (triggered && breakpoint.autoclear)
					{
						breakpoint.enabled = false;
					}
				}
			}

			foreach (var breakpoint in breakpoints)
			{
				if (breakpoint.enabled)
				{
					any_enabled = true;
					break;
				}
			}
		}

		private void toolStripButton1_Click(object sender, EventArgs e)
		{

		}

		private void toolStripButton2_Click(object sender, EventArgs e)
		{

		}

		private void listView1_Click(object sender, EventArgs e)
		{

		}
	}
}
