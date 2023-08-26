using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Breaknes
{
	public partial class FormIOConfig : Form
	{
		IOConfig config = new();

		public FormIOConfig()
		{
			InitializeComponent();
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
					for (int i=0; i<config.devices.Length; i++)
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
			string text = IOConfigManager.SerializeIOConfig(config);
			Console.Write(text);
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
			BoardDescription boards = BoardDescriptionLoader.Load();
			foreach (var board in boards.boards)
			{
				for (int i=0; i<board.io.Length; i++)
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
				foreach (var attached in device.attached)
				{
					for (int i =0; i<checkedListBox1.Items.Count; i++)
					{
						checkedListBox1.SetItemCheckState(i, attached == checkedListBox1.Items[i].ToString() ? CheckState.Checked : CheckState.Unchecked);
					}
				}
			}
		}
	}
}
