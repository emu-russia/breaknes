namespace Breaknes
{
	public partial class FormIOConfig : Form
	{
		IOConfig config = new();
		IOProcessor parent_io;

		public FormIOConfig(IOProcessor io)
		{
			InitializeComponent();
			parent_io = io;
		}

		private void FormIOConfig_Load(object sender, EventArgs e)
		{
			config = IOConfigManager.LoadIOConfig();
			PopulateDeviceList();
		}

		private void PopulateDeviceList()
		{
			// Registered device pool

			listView1.Clear();
			foreach (var device in config.devices)
			{
				ListViewItem item = new ListViewItem(IOConfigManager.DeviceIDToString(device.device_id) + " " + device.name);
				item.Tag = device;
				listView1.Items.Add(item);
			}
		}

		// Bindings
		private void button1_Click(object sender, EventArgs e)
		{
			if (listView1.SelectedItems.Count != 0)
			{
				var item = listView1.SelectedItems[0];
				if (item.Tag != null)
				{
					IOConfigDevice device = item.Tag as IOConfigDevice;

					switch (device.device_id)
					{
						case 0x00000001:
							FormBindingFamiController1 fami1 = new FormBindingFamiController1(device, parent_io);
							fami1.ShowDialog();
							break;
						case 0x00000002:
							FormBindingFamiController2 fami2 = new FormBindingFamiController2(device, parent_io);
							fami2.ShowDialog();
							break;
						case 0x00000003:
							FormBindingNESController nes = new FormBindingNESController(device, parent_io);
							nes.ShowDialog();
							break;
						case 0x00000004:
							FormBindingDendyController dendy = new FormBindingDendyController(device, parent_io);
							dendy.ShowDialog();
							break;
						case 0x00010001:
							FormVirtualFamiController1 virt_fami1 = new FormVirtualFamiController1(device, parent_io);
							virt_fami1.ShowDialog();
							break;
						case 0x00010002:
							FormVirtualFamiController2 virt_fami2 = new FormVirtualFamiController2(device, parent_io);
							virt_fami2.ShowDialog();
							break;
						case 0x00010003:
							FormVirtualNESController virt_nes = new FormVirtualNESController(device, parent_io);
							virt_nes.ShowDialog();
							break;
						case 0x00010004:
							FormVirtualDendyController virt_dendy = new FormVirtualDendyController(device, parent_io);
							virt_dendy.ShowDialog();
							break;
					}
				}
			}
		}

		// Add Device
		private void button2_Click(object sender, EventArgs e)
		{
			FormIOAddDevice addDevice = new FormIOAddDevice();
			addDevice.FormClosed += AddDevice_FormClosed;
			addDevice.ShowDialog();
		}

		private void AddDevice_FormClosed(object? sender, FormClosedEventArgs e)
		{
			FormIOAddDevice addDevice = (FormIOAddDevice)sender;

			if (addDevice.device_to_add != null)
			{
				var list = config.devices.ToList();
				list.Add(addDevice.device_to_add);
				config.devices = list.ToArray();
				PopulateDeviceList();
				checkedListBox1.Items.Clear();
			}
		}

		// Remove Device
		private void button3_Click(object sender, EventArgs e)
		{
			if (listView1.SelectedItems.Count != 0)
			{
				var item = listView1.SelectedItems[0];
				if (item.Tag != null)
				{
					var devices_list = new List<IOConfigDevice>();
					for (int i = 0; i < config.devices.Length; i++)
					{
						if (config.devices[i] != (IOConfigDevice)item.Tag)
						{
							devices_list.Add(config.devices[i]);
						}
					}

					config.devices = devices_list.ToArray();
					PopulateDeviceList();
					checkedListBox1.Items.Clear();
				}
			}
		}

		// Save
		private void button4_Click(object sender, EventArgs e)
		{
			IOConfigManager.SaveIOConfig(config);
			Close();
		}

		// Device list click
		private void listView1_Click(object sender, EventArgs e)
		{
			if (listView1.SelectedItems.Count != 0)
			{
				var item = listView1.SelectedItems[0];
				if (item.Tag != null)
					PopulateConnectionStatus(item.Tag as IOConfigDevice);
			}
		}

		private void PopulateConnectionStatus(IOConfigDevice device)
		{
			// Fill in the list of compatible motherboards

			checkedListBox1.Items.Clear();
			checkedListBox1.Tag = null;
			BoardDescription boards = BoardDescriptionLoader.Load();
			foreach (var board in boards.boards)
			{
				for (int i = 0; i < board.io.Length; i++)
				{
					if (board.io[i].devices.Contains(device.device_id))
					{
						checkedListBox1.Items.Add(board.name);
						break;
					}
				}
			}

			// Select the Attached status check box

			if (device != null)
			{
				for (int i = 0; i < checkedListBox1.Items.Count; i++)
				{
					bool attached = false;

					foreach (var port in device.attached)
					{
						if (port.board == checkedListBox1.Items[i].ToString())
						{
							attached = true;
							break;
						}
					}

					checkedListBox1.SetItemCheckState(i, attached ? CheckState.Checked : CheckState.Unchecked);
				}
				checkedListBox1.Tag = device;
			}
		}

		private void checkedListBox1_ItemCheck(object sender, ItemCheckEventArgs e)
		{
			IOConfigDevice device = checkedListBox1.Tag as IOConfigDevice;
			if (device == null)
				return;

			List<IOConfigPort> attached = device.attached.ToList();

			string board_name = checkedListBox1.Items[e.Index].ToString();

			if (e.CurrentValue == CheckState.Checked)
			{
				List<IOConfigPort> new_board_list = new();

				foreach (var port in attached)
				{
					if (port.board != board_name)
						new_board_list.Add(port);
				}

				attached = new_board_list;
			}
			else
			{
				IOConfigPort port = new();
				port.board = board_name;
				port.port = 0;  // TODO
				attached.Add(port);
			}

			device.attached = attached.ToArray();
		}

		private void FormIOConfig_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
