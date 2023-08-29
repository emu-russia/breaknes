
namespace Breaknes
{
	public partial class FormIOAddDevice : Form
	{
		public IOConfigDevice? device_to_add = null;

		public FormIOAddDevice()
		{
			InitializeComponent();

			comboBox1.SelectedIndex = 0;
		}

		private void button1_Click(object sender, EventArgs e)
		{
			UInt32 device_id = ComboIndexToDeviceID();
			string name = textBox1.Text;

			if (device_id != 0 && name != "")
			{
				device_to_add = new IOConfigDevice();
				device_to_add.name = name;
				device_to_add.device_id = device_id;

				// Assign bindings automatically for virtual devices
				List<IOConfigBinding> bindings = new();

				switch (device_id)
				{
					case 0x00010001:
						bindings.Add(new IOConfigBinding(0, "Fami1_Up"));
						bindings.Add(new IOConfigBinding(1, "Fami1_Down"));
						bindings.Add(new IOConfigBinding(2, "Fami1_Left"));
						bindings.Add(new IOConfigBinding(3, "Fami1_Right"));
						bindings.Add(new IOConfigBinding(4, "Fami1_Select"));
						bindings.Add(new IOConfigBinding(5, "Fami1_Start"));
						bindings.Add(new IOConfigBinding(6, "Fami1_B"));
						bindings.Add(new IOConfigBinding(7, "Fami1_A"));
						device_to_add.bindings = bindings.ToArray();
						break;
					case 0x00010002:
						bindings.Add(new IOConfigBinding(0, "Fami2_Up"));
						bindings.Add(new IOConfigBinding(1, "Fami2_Down"));
						bindings.Add(new IOConfigBinding(2, "Fami2_Left"));
						bindings.Add(new IOConfigBinding(3, "Fami2_Right"));
						bindings.Add(new IOConfigBinding(4, "Fami2_B"));
						bindings.Add(new IOConfigBinding(5, "Fami2_A"));
						bindings.Add(new IOConfigBinding(6, "Fami2_Volume"));
						bindings.Add(new IOConfigBinding(7, "Fami2_MicLevel"));
						device_to_add.bindings = bindings.ToArray();
						break;
					case 0x00010003:
						bindings.Add(new IOConfigBinding(0, "NES_Up"));
						bindings.Add(new IOConfigBinding(1, "NES_Down"));
						bindings.Add(new IOConfigBinding(2, "NES_Left"));
						bindings.Add(new IOConfigBinding(3, "NES_Right"));
						bindings.Add(new IOConfigBinding(4, "NES_Select"));
						bindings.Add(new IOConfigBinding(5, "NES_Start"));
						bindings.Add(new IOConfigBinding(6, "NES_B"));
						bindings.Add(new IOConfigBinding(7, "NES_A"));
						device_to_add.bindings = bindings.ToArray();
						break;
					case 0x00010004:
						bindings.Add(new IOConfigBinding(0, "Dendy_Up"));
						bindings.Add(new IOConfigBinding(1, "Dendy_Down"));
						bindings.Add(new IOConfigBinding(2, "Dendy_Left"));
						bindings.Add(new IOConfigBinding(3, "Dendy_Right"));
						bindings.Add(new IOConfigBinding(4, "Dendy_Select"));
						bindings.Add(new IOConfigBinding(5, "Dendy_Start"));
						bindings.Add(new IOConfigBinding(6, "Dendy_TurboB"));
						bindings.Add(new IOConfigBinding(7, "Dendy_TurboA"));
						bindings.Add(new IOConfigBinding(8, "Dendy_B"));
						bindings.Add(new IOConfigBinding(9, "Dendy_A"));
						device_to_add.bindings = bindings.ToArray();
						break;
				}
			}
			Close();
		}

		private UInt32 ComboIndexToDeviceID()
		{
			switch (comboBox1.SelectedIndex)
			{
				case 0: return 0x00000001;
				case 1: return 0x00000002;
				case 2: return 0x00000003;
				case 3: return 0x00000004;
				case 4: return 0x00010001;
				case 5: return 0x00010002;
				case 6: return 0x00010003;
				case 7: return 0x00010004;
			}
			return 0;
		}

		private void FormIOAddDevice_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
