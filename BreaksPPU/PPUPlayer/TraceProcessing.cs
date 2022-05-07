// Display the signal trace in the DataGridView.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections;

// The data model is straightforward.
// There is a Field list. Each Field contains a list of Scans. Each Scan contains a list of PPU signal values obtained after each half-step of the simulation.

// The main problem is to find the right code from StackOverflow to work with DataGridView.

namespace PPUPlayer
{
	public partial class Form1 : Form
	{
		List<FieldTrace> fields = new();

		int CurrentTraceField = 0;
		int CurrentTraceScan = 0;

		bool TraceResetInProgress = false;

		class FieldTrace
		{
			public List<ScanTrace> scans = new();
		}

		class ScanTrace
		{
			public List<List<BreaksCore.DebugInfoEntry>> entries = new();
		}

		private void testTraceToolStripMenuItem_Click(object sender, EventArgs e)
		{
			ResetTrace(2);

			var entry = BreaksCore.GetTestDebugInfo();
			fields[0].scans[0].entries.Add(entry);
			fields[0].scans[0].entries.Add(entry);
			fields[0].scans[0].entries.Add(entry);

			VisualizeTrace(0, 0);
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
				for (int n = 0; n < ScansPerField; n++)
				{
					fields[i].scans.Add(new());
				}
			}

			comboBoxTraceScan.Items.Clear();
			comboBoxTraceField.Items.Clear();

			for (int i=0; i<ScansPerField; i++)
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
		void ProcessTrace()
		{
			var entry = BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_PPU);
			fields[CurrentTraceField].scans[CurrentTraceScan].entries.Add(entry);

			if (fields[CurrentTraceField].scans[CurrentTraceScan].entries.Count >= SamplesPerScan)
			{
				CurrentTraceScan++;
			}

			if (CurrentTraceScan >= ScansPerField)
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

			foreach (var row in fields[field].scans[scan].entries)
			{
				string[] rowSignals = new string[row.Count];
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

				dataGridView1.Rows.Add(rowSignals);
			}
		}

	}
}
