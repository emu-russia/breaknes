
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
		}

		public void ResetControls()
		{
			comboBox1.Items.Clear();
			comboBox2.Items.Clear();
			comboBox3.Items.Clear();

			checkBox1.Checked = false;
			checkBox2.Checked = false;
			comboBox4.Items.Clear();
			textBox1.Text = "";
		}

		public void ShowBreakpointProps(Breakpoint bp)
		{
			ResetControls();

			comboBox1.Items.Add(bp.info_type.ToString().Replace("DebugInfoType_", ""));
			comboBox1.SelectedIndex = 0;
			comboBox2.Items.Add(bp.info_entry.category);
			comboBox2.SelectedIndex = 0;
			comboBox3.Items.Add(bp.info_entry.name);
			comboBox3.SelectedIndex = 0;

			checkBox1.Checked = bp.enabled;
			checkBox2.Checked = bp.autoclear;
			comboBox4.Items.Add(bp.trigger.ToString());
			comboBox4.SelectedIndex = 0;

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

		BreakpontTrigger ComboBoxToTrigger()
		{
			switch (comboBox4.SelectedIndex)
			{
				case 0:
					return BreakpontTrigger.High;
				case 1:
					return BreakpontTrigger.Low;
				case 2:
					return BreakpontTrigger.Z;
				case 3:
					return BreakpontTrigger.Posedge;
				case 4:
					return BreakpontTrigger.Negedge;
				case 5:
					return BreakpontTrigger.VectorEqual;
			}

			return BreakpontTrigger.Low;
		}

		/// <summary>
		/// type changed
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (!comboBox1.Enabled)
				return;

			BreaksCore.DebugInfoType info_type = ComboBoxToDebugInfoType();
			var list = BreaksCore.GetDebugInfo(info_type);
			var cats = list.Select(x => x.category).Distinct();

			comboBox2.Items.Clear();
			foreach (var item in cats)
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
			if (!comboBox2.Enabled)
				return;

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

		public Breakpoint ControlsToBreakpoint()
		{
			Breakpoint bp = new();

			bp.info_type = ComboBoxToDebugInfoType();
			bp.info_entry = new BreaksCore.DebugInfoEntry();
			bp.info_entry.category = comboBox2.Text;
			bp.info_entry.name = comboBox3.Text;

			bp.prev_value = BreaksCore.GetDebugInfoByName(bp.info_type, bp.info_entry.category, bp.info_entry.name);

			bp.trigger = ComboBoxToTrigger();

			bp.enabled = checkBox1.Checked;
			bp.autoclear = checkBox2.Checked;
			bp.vector_value = StrToUInt32(textBox1.Text);

			return bp;
		}

		UInt32 StrToUInt32(string str)
		{
			str = str.Trim();

			if (str == "")
				return 0;

			return (str.Contains("0x") || str.Contains("0X")) ?
				Convert.ToUInt32(str, 16) :
					str[0] == '0' ? Convert.ToUInt32(str, 8) : Convert.ToUInt32(str, 10);
		}

		public void SetEnabled()
		{
			checkBox1.Checked = true;
		}
	}
}
