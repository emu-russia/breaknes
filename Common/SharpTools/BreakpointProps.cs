
namespace SharpTools
{
	public partial class BreakpointProps : UserControl
	{
		public BreakpointProps()
		{
			InitializeComponent();
		}

		public void DisableProps()
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

		BreaksCore.DebugInfoType ComboBoxToDebugInfoType()
		{
			switch (comboBox1.SelectedIndex)
			{
				case 0:
					return BreaksCore.DebugInfoType.DebugInfoType_Core;
				case 1:
					return BreaksCore.DebugInfoType.DebugInfoType_CoreRegs;
				case 2:
					return BreaksCore.DebugInfoType.DebugInfoType_APU;
				case 3:
					return BreaksCore.DebugInfoType.DebugInfoType_APURegs;
				case 4:
					return BreaksCore.DebugInfoType.DebugInfoType_PPU;
				case 5:
					return BreaksCore.DebugInfoType.DebugInfoType_PPURegs;
				case 6:
					return BreaksCore.DebugInfoType.DebugInfoType_Board;
				case 7:
					return BreaksCore.DebugInfoType.DebugInfoType_Cart;
			}

			return BreaksCore.DebugInfoType.DebugInfoType_Test;
		}

		/// <summary>
		/// type changed
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
		{
			BreaksCore.DebugInfoType info_type = ComboBoxToDebugInfoType();
			var list = BreaksCore.GetDebugInfo(info_type);
			var cats = list.Select(x => x.category).Distinct();

			comboBox2.Items.Clear();
			foreach ( var item in cats)
			{
				comboBox2.Items.Add(item);
			}
			comboBox3.Items.Clear();
		}

		/// <summary>
		/// category changed
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
		{
			BreaksCore.DebugInfoType info_type = ComboBoxToDebugInfoType();
			var list = BreaksCore.GetDebugInfo(info_type);
			var cat = comboBox2.Text;

			comboBox3.Items.Clear();
			foreach (var item in list)
			{
				if (item.category == cat)
				{
					comboBox3.Items.Add(item.name);
				}
			}
		}
	}
}
