
namespace SharpTools
{
	public partial class BreakpointProps : UserControl
	{
		public BreakpointProps()
		{
			InitializeComponent();
		}

		public void DisableProps ()
		{
			comboBox1.Enabled = false;
			comboBox2.Enabled = false;
			comboBox3.Enabled = false;

			checkBox1.Enabled = false;
			checkBox2.Enabled = false;
			comboBox4.Enabled = false;
			textBox1.Enabled = false;
		}

		public void ShowBreakpointProps(Breakpoint bp)
		{
			comboBox1.Text = bp.info_type.ToString();
			comboBox2.Text = bp.info_entry.category.ToString();
			comboBox3.Text = bp.info_entry.name.ToString();

			checkBox1.Checked = bp.enabled;
			checkBox2.Checked = bp.autoclear;
			comboBox4.Text = bp.trigger.ToString();

			if (bp.trigger == BreakpontTrigger.VectorEqual)
			{
				textBox1.Text = bp.vector_value.ToString();
			}
			else
			{
				textBox1.Text = "";
			}
		}
	}
}
