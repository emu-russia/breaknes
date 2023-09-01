using System.Windows.Forms;

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
			InitGridColumns();
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
				dataGridView1.DataSource = null;
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
					dataGridView1.DataSource = null;
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
					PopulateGrid(item.Tag as IOConfigDevice);
			}
		}

		/*
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
		*/

		private void FormIOConfig_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}

		private void InitGridColumns()
		{
			dataGridView1.AutoGenerateColumns = false;
			dataGridView1.AutoSize = true;

			// Initialize and add a check box column.
			DataGridViewCheckBoxColumn check_box_column = new DataGridViewCheckBoxColumn();
			check_box_column.AutoSizeMode = DataGridViewAutoSizeColumnMode.AllCells;
			check_box_column.DataPropertyName = "attached";
			check_box_column.Name = "Attached";
			dataGridView1.Columns.Add(check_box_column);

			DataGridViewTextBoxColumn text_column = new DataGridViewTextBoxColumn();
			text_column.AutoSizeMode = DataGridViewAutoSizeColumnMode.AllCells;
			text_column.DataPropertyName = "board_name";
			text_column.Name = "Board";
			text_column.SortMode = DataGridViewColumnSortMode.NotSortable;
			text_column.ReadOnly = true;
			dataGridView1.Columns.Add(text_column);

			DataGridViewComboBoxColumn cboBoxColumn = new DataGridViewComboBoxColumn();
			//cboBoxColumn.AutoSizeMode = DataGridViewAutoSizeColumnMode.AllCells;
			cboBoxColumn.DataSource = DataPortBinding.GetPorts();
			cboBoxColumn.DisplayMember = "port_name";
			cboBoxColumn.ValueMember = "port_id";
			cboBoxColumn.Name = "Port";
			dataGridView1.Columns.Add(cboBoxColumn);
		}

		private void PopulateGrid(IOConfigDevice device)
		{
			// Fill in the list of compatible motherboards

			bindingSource1.Clear();
			BoardDescription boards = BoardDescriptionLoader.Load();
			foreach (var board in boards.boards)
			{
				for (int i = 0; i < board.io.Length; i++)
				{
					if (board.io[i].devices.Contains(device.device_id))
					{
						DataBinding dataBinding = new DataBinding();

						// Select the Attached status check box
						dataBinding.attached = false;
						foreach (var port in device.attached)
						{
							if (port.board == board.name)
							{
								dataBinding.attached = true;
								break;
							}
						}

						dataBinding.board_name = board.name;
						dataBinding.device = device;
						bindingSource1.Add(dataBinding);
						break;
					}
				}
			}

			dataGridView1.DataSource = bindingSource1;
		}

		class DataBinding
		{
			public IOConfigDevice device;
			public bool attached { get; set; }
			public string board_name { get; set; }
			public List<string> port { get; set; }
		}

		class DataPortBinding
		{
			public string port_name { get; private set; }
			public int port_id { get; private set; }
			public DataPortBinding(string name, int id)
			{
				port_name = name;
				port_id = id;
			}

			public static List<DataPortBinding> GetPorts()
			{
				List<DataPortBinding> ports = new();
				ports.Add(new DataPortBinding("Port1", 0));
				ports.Add(new DataPortBinding("Port2", 0));
				return ports;
			}
		}


	}
}
