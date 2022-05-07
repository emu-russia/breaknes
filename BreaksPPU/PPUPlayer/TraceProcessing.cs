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

		}

		void ResetTrace(int maxFields)
		{
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

	}
}
