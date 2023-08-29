using SharpTools;

namespace Breaknes
{
	public partial class FormBindingFamiController1 : Form
	{
		IOConfigDevice this_device;
		IOProcessor io;

		public FormBindingFamiController1(IOConfigDevice device, IOProcessor io)
		{
			InitializeComponent();
			this.io = io;
			this_device = device;
		}

		private void FormBindingFamiController1_Load(object sender, EventArgs e)
		{
			PopulateActuatorsList();
			listView2.Clear();
		}

		private void PopulateActuatorsList()
		{
			listView1.Clear();

			int handle = BreaksCore.IOCreateInstance(0x00000001);
			if (handle >= 0)
			{
				int num_actuators = BreaksCore.IOGetNumStates(handle);

				for (int i = 0; i < num_actuators; i++)
				{
					char[] actuator_name = new char[0x100];

					BreaksCore.IOGetStateName(handle, i, actuator_name, actuator_name.Length);

					ListViewItem item = new ListViewItem(new string(actuator_name));
					item.Tag = i;       // Actuator ID

					listView1.Items.Add(item);
				}

				BreaksCore.IODisposeInstance(handle);
			}
		}

		// Clear all
		private void button1_Click(object sender, EventArgs e)
		{
			this_device.bindings = Array.Empty<IOConfigBinding>();
		}

		// Add binding
		private void button2_Click(object sender, EventArgs e)
		{
			if (listView1.SelectedItems.Count != 0)
			{
				var item = listView1.SelectedItems[0];
				int actuator_id = (int)item.Tag;

				FormWaitInputEvent wait_input = new FormWaitInputEvent(actuator_id, io);
				wait_input.FormClosed += Wait_input_FormClosed;
				wait_input.ShowDialog();
			}
		}

		private void AssignBinding(int actuator_id, string bindstr)
		{
			bool found = false;

			for (int i = 0; i < this_device.bindings.Length; i++)
			{
				if (this_device.bindings[i].actuator_id == actuator_id)
				{
					this_device.bindings[i].binding = bindstr;
					found = true;
					break;
				}
			}

			if (!found)
			{
				List<IOConfigBinding> list = this_device.bindings.ToList();
				IOConfigBinding new_binding = new();
				new_binding.actuator_id = actuator_id;
				new_binding.binding = bindstr;
				list.Add(new_binding);
				this_device.bindings = list.ToArray();
			}

			ShowBindings(actuator_id);
		}

		private void Wait_input_FormClosed(object? sender, FormClosedEventArgs e)
		{
			FormWaitInputEvent wait_input = (FormWaitInputEvent)sender;
			if (wait_input.bindstr != null)
			{
				AssignBinding(wait_input.saved_actuator_id, wait_input.bindstr);
			}
		}

		// Clear binding
		private void button3_Click(object sender, EventArgs e)
		{
			if (listView1.SelectedItems.Count != 0)
			{
				var item = listView1.SelectedItems[0];
				int actuator_id = (int)item.Tag;

				List<IOConfigBinding> list = new();
				for (int i = 0; i < this_device.bindings.Length; i++)
				{
					if (this_device.bindings[i].actuator_id != actuator_id)
					{
						list.Add(this_device.bindings[i]);
					}
				}
				this_device.bindings = list.ToArray();

				ShowBindings(actuator_id);
			}
		}

		// Save
		private void button4_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void listView1_Click(object sender, EventArgs e)
		{
			if (listView1.SelectedItems.Count != 0)
			{
				var item = listView1.SelectedItems[0];
				ShowBindings((int)item.Tag);
			}
			else
			{
				listView2.Clear();
			}
		}

		private void ShowBindings(int actuator_id)
		{
			listView2.Clear();
			for (int i=0; i<this_device.bindings.Length; i++)
			{
				if (this_device.bindings[i].actuator_id == actuator_id)
				{
					listView2.Items.Add(this_device.bindings[i].binding);
				}
			}
		}
	}
}
