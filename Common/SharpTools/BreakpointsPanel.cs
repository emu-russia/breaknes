
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
			breakpointProps1.DisableProps();
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

			UpdateAnyEnabled();
		}

		private void UpdateAnyEnabled ()
		{
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
			FormAddBreakpoint formAdd = new FormAddBreakpoint();
			formAdd.FormClosed += FormAdd_FormClosed;
			formAdd.ShowDialog();
		}

		private void FormAdd_FormClosed(object? sender, FormClosedEventArgs e)
		{
			FormAddBreakpoint formAdd = (FormAddBreakpoint)sender;

			UpdateAnyEnabled();
		}

		private void toolStripButton2_Click(object sender, EventArgs e)
		{
			if (listView1.SelectedItems.Count != 0)
			{
				var item = listView1.SelectedItems[0];
				Breakpoint bp = item.Tag as Breakpoint;
				breakpoints.Remove(bp);
				listView1.Items.Remove(item);
				UpdateAnyEnabled();
			}
		}

		private void listView1_Click(object sender, EventArgs e)
		{
			if (listView1.SelectedItems.Count != 0)
			{
				var item = listView1.SelectedItems[0];
				Breakpoint bp = item.Tag as Breakpoint;
				breakpointProps1.ShowBreakpointProps(bp);
			}
		}
	}
}
