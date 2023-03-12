using Be.Windows.Forms;
using SharpTools;
using SharpToolsCustomClass;
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
	public partial class FormDebugger : Form
	{
		private DataHumanizer humanizer = new();
		private List<BreaksCore.MemDesciptor> mem = new();
		private bool UpdateMemLayoutInProgress = false;
		private BoardControl board_ctrl;

		public FormDebugger(BoardControl _board_ctrl)
		{
			board_ctrl = _board_ctrl;
			InitializeComponent();
		}

		private void FormDebugger_Load(object sender, EventArgs e)
		{
			UpdateMemLayout();
			humanizer.SetColorDebugOutput(true);
		}

		private void FormDebugger_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
			else if (e.KeyCode == Keys.F11)
			{
				Step();
			}
			else if (e.KeyCode == Keys.F5)
			{
				board_ctrl.Paused = false;
			}
		}

		private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
		{
			Button2Click();
		}

		private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (UpdateMemLayoutInProgress)
				return;

			Button1Click();
		}

		/// <summary>
		/// Update DebugInfo
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void button2_Click(object sender, EventArgs e)
		{
			Button2Click();
		}

		/// <summary>
		/// Dump Mem
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void button1_Click(object sender, EventArgs e)
		{
			Button1Click();
		}

		void Button2Click()
		{
			List<BreaksCore.DebugInfoEntry> entries = BreaksCore.GetDebugInfo(ComboBoxToDebugInfoType());
			UpdateDebugInfo(entries);
		}

		BreaksCore.DebugInfoType ComboBoxToDebugInfoType()
		{
			switch (comboBox2.SelectedIndex)
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

		void UpdateDebugInfo(List<BreaksCore.DebugInfoEntry> entries)
		{
			// Construct an object to show in PropertyGrid

			CustomClass myProperties = new CustomClass();

			foreach (BreaksCore.DebugInfoEntry entry in entries)
			{
				CustomProperty myProp = new CustomProperty();
				myProp.Name = entry.name;
				myProp.Category = entry.category;
				myProp.Value = entry.value;
				myProperties.Add(myProp);
			}

			propertyGrid1.SelectedObject = myProperties;
		}

		void Button1Click()
		{
			if (mem.Count == 0)
			{
				hexBox1.ByteProvider = new DynamicByteProvider(new byte[0]);
				hexBox1.Refresh();
				return;
			}

			int descrID = comboBox1.SelectedIndex;

			if (pictureBoxForHuman.Visible)
			{
				string descrName = mem[descrID].name;

				Bitmap bitmap = humanizer.ConvertHexToImage(descrName);
				pictureBoxForHuman.Image = bitmap;
				pictureBoxForHuman.Invalidate();
			}
			else
			{
				byte[] buf = new byte[mem[descrID].size];

				BreaksCore.DumpMem(descrID, buf);
				hexBox1.ByteProvider = new DynamicByteProvider(buf);
				hexBox1.Refresh();
			}
		}

		/// <summary>
		/// Get a set of memory regions from the debugger and fill the ComboBox.
		/// </summary>
		void UpdateMemLayout()
		{
			UpdateMemLayoutInProgress = true;

			comboBox1.Items.Clear();

			mem = BreaksCore.GetMemoryLayout();

			foreach (var descr in mem)
			{
				comboBox1.Items.Add(descr.name);
			}

			if (mem.Count != 0)
			{
				comboBox1.SelectedIndex = 0;
			}

			UpdateMemLayoutInProgress = false;
		}

		private void TraceCore()
		{
			string text = "";
			List<BreaksCore.DebugInfoEntry> entries = BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_CoreRegs);
			int max = 4;
			int cnt = 0;
			foreach (var entry in entries)
			{
				text += entry.name + " = " + entry.value.ToString("X2") + "; ";
				cnt++;
				if (cnt > max)
					break;
			}
			Console.WriteLine(text);
		}

		// Pause
		private void toolStripButton1_Click(object sender, EventArgs e)
		{
			board_ctrl.Paused = true;
		}

		/// A tool for short-range debugging.
		/// Step-by-step execution of the simulation. Available only if the worker is stopped.
		private void Step()
		{
			if (board_ctrl.Paused)
			{
				if (BreaksCore.InResetState())
				{
					// Skip the reset condition by following all steps until it resets

					while (BreaksCore.InResetState())
					{
						BreaksCore.Step();
					}
				}
				else
				{
					BreaksCore.Step();
				}

				TraceCore();
				Button2Click();
			}
		}


		private void toolStripButton2_Click(object sender, EventArgs e)
		{
			Step();
		}

		// Click
		private void toolStripButton3_Click(object sender, EventArgs e)
		{
			board_ctrl.Paused = false;
		}

		/// <summary>
		/// Used to change the value of PPU registers on the fly. Use for Free Mode.
		/// </summary>
		private void propertyGrid1_PropertyValueChanged(object s, PropertyValueChangedEventArgs e)
		{
			if (e.ChangedItem != null)
			{
				BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_PPURegs,
					"PPU Regs", e.ChangedItem.Label, (UInt32)e.ChangedItem.Value);
			}
		}
	}
}
