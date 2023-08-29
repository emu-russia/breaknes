
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
						bindings.Add(new IOConfigBinding(0, "Up"));
						bindings.Add(new IOConfigBinding(1, "Down"));
						bindings.Add(new IOConfigBinding(2, "Left"));
						bindings.Add(new IOConfigBinding(3, "Right"));
						bindings.Add(new IOConfigBinding(4, "Select"));
						bindings.Add(new IOConfigBinding(5, "Start"));
						bindings.Add(new IOConfigBinding(6, "B"));
						bindings.Add(new IOConfigBinding(7, "A"));
						device_to_add.bindings = bindings.ToArray();
						break;
					case 0x00010002:
						bindings.Add(new IOConfigBinding(0, "Up"));
						bindings.Add(new IOConfigBinding(1, "Down"));
						bindings.Add(new IOConfigBinding(2, "Left"));
						bindings.Add(new IOConfigBinding(3, "Right"));
						bindings.Add(new IOConfigBinding(4, "B"));
						bindings.Add(new IOConfigBinding(5, "A"));
						bindings.Add(new IOConfigBinding(6, "Volume"));
						bindings.Add(new IOConfigBinding(7, "MicLevel"));
						device_to_add.bindings = bindings.ToArray();
						break;
					case 0x00010003:
						bindings.Add(new IOConfigBinding(0, "Up"));
						bindings.Add(new IOConfigBinding(1, "Down"));
						bindings.Add(new IOConfigBinding(2, "Left"));
						bindings.Add(new IOConfigBinding(3, "Right"));
						bindings.Add(new IOConfigBinding(4, "Select"));
						bindings.Add(new IOConfigBinding(5, "Start"));
						bindings.Add(new IOConfigBinding(6, "B"));
						bindings.Add(new IOConfigBinding(7, "A"));
						device_to_add.bindings = bindings.ToArray();
						break;
					case 0x00010004:
						bindings.Add(new IOConfigBinding(0, "Up"));
						bindings.Add(new IOConfigBinding(1, "Down"));
						bindings.Add(new IOConfigBinding(2, "Left"));
						bindings.Add(new IOConfigBinding(3, "Right"));
						bindings.Add(new IOConfigBinding(4, "Select"));
						bindings.Add(new IOConfigBinding(5, "Start"));
						bindings.Add(new IOConfigBinding(6, "TurboB"));
						bindings.Add(new IOConfigBinding(7, "TurboA"));
						bindings.Add(new IOConfigBinding(8, "B"));
						bindings.Add(new IOConfigBinding(9, "A"));
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
