// Display the signal trace in the DataGridView.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.IO;

using SharpTools;

// The data model is straightforward.
// There is a Field list. Each Field contains a list of Scans. Each Scan contains a list of PPU signal values obtained after each half-step of the simulation.

// The main problem is to find the right code from StackOverflow to work with DataGridView.

namespace PPUPlayer
{
	public partial class FormMain : Form
	{
		List<FieldTrace> fields = new();

		int CurrentTraceField = 0;
		int CurrentTraceScan = 0;

		bool TraceResetInProgress = false;
		internal int TraceTimeResolutionNanos = 23;
		internal int VCDMax = 90;

		class FieldTrace
		{
			public List<ScanTrace> scans = new();
		}

		class ScanTrace
		{
			public List<List<BreaksCore.DebugInfoEntry>> entries = new();
		}

		void ResetTrace(int maxFields)
		{
			TraceResetInProgress = true;

			CurrentTraceField = 0;
			CurrentTraceScan = 0;

			fields = new();

			for (int i=0; i<maxFields; i++)
			{
				fields.Add(new());
			}

			for (int i = 0; i < maxFields; i++)
			{
				for (int n = 0; n < ppu_features.ScansPerField; n++)
				{
					fields[i].scans.Add(new());
				}
			}

			comboBoxTraceScan.Items.Clear();
			comboBoxTraceField.Items.Clear();

			for (int i=0; i< ppu_features.ScansPerField; i++)
			{
				comboBoxTraceScan.Items.Add(i.ToString());
			}

			comboBoxTraceScan.SelectedIndex = 0;

			for (int i=0; i<maxFields; i++)
			{
				comboBoxTraceField.Items.Add(i.ToString());
			}

			comboBoxTraceField.SelectedIndex = 0;

			TraceResetInProgress = false;

			Console.WriteLine("ResetTrace. MaxFields: " + maxFields.ToString());
		}

		/// <summary>
		/// Tracing is a very slow process.
		/// </summary>
		void ProcessTrace(int currentScan)
		{
			var entry = BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_PPU);
			entry.AddRange(BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_PPURegs));
			entry.AddRange(BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_Board));

			// Add the scan number to the very end

			BreaksCore.DebugInfoEntry scan = new();
			scan.category = "";
			scan.name = "Scan";
			scan.value = (uint)currentScan;
			entry.Add(scan);

			fields[CurrentTraceField].scans[CurrentTraceScan].entries.Add(entry);

			if (fields[CurrentTraceField].scans[CurrentTraceScan].entries.Count >= SamplesPerScan)
			{
				CurrentTraceScan++;
			}

			if (CurrentTraceScan >= ppu_features.ScansPerField)
			{
				CurrentTraceScan = 0;
				CurrentTraceField++;
			}

			if (CurrentTraceField >= TraceMaxFields)
			{
				ResetTrace(TraceMaxFields);
			}
		}

		private void comboBoxTraceScan_SelectedIndexChanged(object sender, EventArgs e)
		{
			VisualizeTrace(comboBoxTraceField.SelectedIndex, comboBoxTraceScan.SelectedIndex);
		}

		private void comboBoxTraceField_SelectedIndexChanged(object sender, EventArgs e)
		{
			VisualizeTrace(comboBoxTraceField.SelectedIndex, comboBoxTraceScan.SelectedIndex);
		}

		void VisualizeTrace(int field, int scan)
		{
			if (TraceResetInProgress)
				return;

			Console.WriteLine("VisualizeTrace: scan: " + scan.ToString() + ", field: " + field.ToString());

			dataGridView1.Columns.Clear();

			List<string> filter = new();
			
			if (TraceFilter != "")
			{
				filter = TraceFilter.Split(';').ToList();
			}

			//dataGridView1.AutoSizeColumnsMode =	DataGridViewAutoSizeColumnsMode.AllCells;
			//dataGridView1.AutoSizeRowsMode = DataGridViewAutoSizeRowsMode.AllCells;

			if (fields[field].scans[scan].entries.Count == 0)
				return;

			List<BreaksCore.DebugInfoEntry> entry0 = fields[field].scans[scan].entries[0];

			int colCount = entry0.Count;

			foreach (var info in entry0)
			{
				if (filter.Count != 0)
				{
					if (!filter.Contains(info.name))
						continue;
				}

				DataGridViewColumn col = new DataGridViewTextBoxColumn();
				col.Name = info.name;
				col.DataPropertyName = "value";

				dataGridView1.Columns.Add(col);
			}

			string[] prevRowSignals = new string[colCount];

			foreach (var row in fields[field].scans[scan].entries)
			{
				string[] rowSignals = new string[colCount];
				int colIdx = 0;

				foreach (var signal in row)
				{
					if (filter.Count != 0)
					{
						if (!filter.Contains(signal.name))
							continue;
					}

					string text = "";

					if (signal.value == 0) text = "0";
					else if (signal.value == 1) text = "1";
					else if ((Int32)signal.value == -1) text = "z";
					else if ((Int32)signal.value == -2) text = "x";
					else text = "0x" + signal.value.ToString("x");

					rowSignals[colIdx] = text;
					colIdx++;
				}

				if (TraceCollapseSameRows && rowSignals.SequenceEqual(prevRowSignals) )
				{
					continue;
				}

				dataGridView1.Rows.Add(rowSignals);
				rowSignals.CopyTo(prevRowSignals, 0);
			}
		}

		private void saveTraceInLogisimFormatToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (saveFileDialog2.ShowDialog() == DialogResult.OK)
			{
				SaveTrace(saveFileDialog2.FileName);
			}
		}

		void SaveTrace (string filename)
		{
			string text = "";

			int scan = comboBoxTraceScan.SelectedIndex;
			int field = comboBoxTraceField.SelectedIndex;

			if (!(scan >= 0 && field >= 0))
				return;

			if (fields[field].scans[scan].entries.Count == 0)
				return;

			List<string> filter = new();

			if (TraceFilter != "")
			{
				filter = TraceFilter.Split(';').ToList();
			}

			List<BreaksCore.DebugInfoEntry> entry0 = fields[field].scans[scan].entries[0];

			bool first_col = true;

			foreach (var info in entry0)
			{
				if (filter.Count != 0)
				{
					if (!filter.Contains(info.name))
						continue;
				}

				if (!first_col)
				{
					text += "\t";
				}

				text += info.name;

				first_col = false;
			}

			text += "\n";

			string prev_row_text = "";

			foreach (var row in fields[field].scans[scan].entries)
			{
				bool first = true;

				string row_text = "";

				foreach (var signal in row)
				{
					if (filter.Count != 0)
					{
						if (!filter.Contains(signal.name))
							continue;
					}

					string val_text = "";

					if (signal.value == 0) val_text = "0";
					else if (signal.value == 1) val_text = "1";
					else if ((Int32)signal.value == -1) val_text = "z";
					else if ((Int32)signal.value == -2) val_text = "x";
					else val_text = "0x" + signal.value.ToString("x");

					if (!first)
					{
						row_text += "\t";
					}

					row_text += val_text;

					first = false;
				}

				if (TraceCollapseSameRows && row_text == prev_row_text)
				{
					continue;
				}

				text += row_text + "\n";
				prev_row_text = row_text;
			}

			File.WriteAllText(filename, text);
		}

		void SetTraceTimeResolutionNanos(int ns)
		{
			TraceTimeResolutionNanos = ns;
		}

		void SaveTraceVCD(string filename)
		{
			// Header

			string text = "";

			int scan = comboBoxTraceScan.SelectedIndex;
			int field = comboBoxTraceField.SelectedIndex;

			if (!(scan >= 0 && field >= 0))
				return;

			if (fields[field].scans[scan].entries.Count == 0)
				return;

			text += "$date\n";
			text += "   1 Jan 2000\n";
			text += "$end\n";

			text += "$version\n";
			text += "   PPUPlayer\n";
			text += "$end\n";

			text += "$comment\n";
			text += "   Comment\n";
			text += "$end\n";

			text += "$timescale 1ns $end\n";

			List<string> filter = new();

			if (TraceFilter != "")
			{
				filter = TraceFilter.Split(';').ToList();
			}

			List<BreaksCore.DebugInfoEntry> entry0 = fields[field].scans[scan].entries[0];

			text += "$scope module logic $end\n";
			char id = '!';
			int signal_count = 0;

			foreach (var info in entry0)
			{
				if (filter.Count != 0)
				{
					if (!filter.Contains(info.name))
						continue;
				}

				text += "$var wire " + info.bits.ToString() + " " + id + " " + info.name + " $end\n";
				id++;

				signal_count++;
				if (signal_count > VCDMax)
				{
					MessageBox.Show(
						"Too many signals to trace to VCD format. Maximum: " + VCDMax.ToString(),
						"Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
					return;
				}
			}

			text += "$upscope $end\n";
			text += "$enddefinitions $end\n";

			// Dumpvars

			text += "$dumpvars\n";
			id = '!';

			foreach (var info in entry0)
			{
				if (filter.Count != 0)
				{
					if (!filter.Contains(info.name))
						continue;
				}

				string val_text = "";

				if (info.bits == 1)
				{
					val_text = "0";
				}
				else
				{
					val_text += "b";
					for (int bit_num=info.bits-1; bit_num>=0; bit_num--)
					{
						uint bitval = (info.value >> bit_num) & 1;
						val_text += bitval.ToString();
					}
					val_text += " ";
				}

				text += val_text + id + "\n";
				id++;
			}

			text += "$end\n";

			// Wave

			int ts = 0;

			foreach (var row in fields[field].scans[scan].entries)
			{
				string row_text = "";
				row_text += "#" + ts.ToString() + "\n";

				id = '!';

				foreach (var signal in row)
				{
					if (filter.Count != 0)
					{
						if (!filter.Contains(signal.name))
							continue;
					}

					string val_text = "";

					if (signal.bits == 1)
					{
						if (signal.value == 0) val_text = "0";
						else if (signal.value == 1) val_text = "1";
						else if ((Int32)signal.value == -1) val_text = "z";
						else if ((Int32)signal.value == -2) val_text = "x";
					}
					else
					{
						val_text += "b";
						for (int bit_num = signal.bits - 1; bit_num >= 0; bit_num--)
						{
							uint bitval = (signal.value >> bit_num) & 1;
							val_text += bitval.ToString();
						}
						val_text += " ";
					}

					row_text += val_text + id + "\n";
					id++;
				}

				text += row_text;
				ts += TraceTimeResolutionNanos;
			}

			File.WriteAllText(filename, text);
		}

		private void saveTraceInVCDFormatToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (saveFileDialogVCD.ShowDialog() == DialogResult.OK)
			{
				SaveTraceVCD(saveFileDialogVCD.FileName);
			}
		}
	}
}
