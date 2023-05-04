
using SharpTools;
using System.Windows.Forms;

namespace NSFPlayer
{
	public partial class FormMain : Form
	{
		Dictionary<string, List<LogicValue>> waves = new();

		/// <summary>
		/// Snatch Waves
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void toolStripButton3_Click(object sender, EventArgs e)
		{
			if (Paused && (nsf_loaded || regdump_loaded))
			{
				if (wavesControl1.IsSelectedSomething())
				{
					long bias;
					ValueChangeData[] data = wavesControl1.SnatchSelection(out bias);
					wavesControl1.ClearSelection();
					FormSnatchWaves snatch = new FormSnatchWaves(data);
					snatch.Show();
				}
				else
				{
					MessageBox.Show("Select something first with the left mouse button. A box will appear, then click on Snatch.", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				}
			}
			else
			{
				if (backgroundWorker1.IsBusy)
				{
					MessageBox.Show("I can't while the worker is running. Pause it first", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				}
				else
				{
					MessageBox.Show("Simulation not started", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				}
			}
		}

		private void ResetWaves()
		{
			ValueChangeData[] vcd = new ValueChangeData[0];
			wavesControl1.PlotWaves(vcd, 0);
			waves = new();
		}

		private void UpdateWaves()
		{
			// Update dump

			var info = BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_APU);
			foreach (var entry in info)
			{
				if (SignalFilter(entry))
				{
					if (!waves.ContainsKey(entry.name))
					{
						waves.Add(entry.name, new List<LogicValue>());
					}
					waves[entry.name].Add(ToLogicValue((byte)entry.value));
				}
			}

			// Update waves control

			ValueChangeData[] vcd = new ValueChangeData[waves.Count];

			int i = 0;
			foreach (var signal in waves.Keys)
			{
				vcd[i] = new ValueChangeData();
				vcd[i].name = signal;
				vcd[i].values = waves[signal].ToArray();
				i++;
			}

			wavesControl1.PlotWaves(vcd, 0);
		}

		private LogicValue ToLogicValue(byte val)
		{
			if (val == 0) return LogicValue.Zero;
			else if (val == 1) return LogicValue.One;
			else if (val == 0xff) return LogicValue.Z;
			else return LogicValue.X;
		}

		bool SignalFilter(BreaksCore.DebugInfoEntry entry)
		{
			if (entry.bits != 1)
				return false;

			if (entry.category == BreaksCore.APU_CLKS_CATEGORY && toolStripButton5.Checked) return true;
			if (entry.category == BreaksCore.APU_CORE_CATEGORY && toolStripButton6.Checked) return true;
			if (entry.category == BreaksCore.APU_DMA_CATEGORY && toolStripButton7.Checked) return true;
			if (entry.category == BreaksCore.APU_REGOPS_CATEGORY && toolStripButton8.Checked) return true;
			if (entry.category == BreaksCore.APU_LC_CATEGORY && toolStripButton9.Checked) return true;
			if (entry.category == BreaksCore.APU_WIRES_CATEGORY && toolStripButton10.Checked) return true;

			return false;
		}

		private void toolStripButton4_Click(object sender, EventArgs e)
		{
			ResetWaves();
		}

		private void toolStripButton5_Click(object sender, EventArgs e)
		{
			toolStripButton5.Checked = !toolStripButton5.Checked;
		}

		private void toolStripButton6_Click(object sender, EventArgs e)
		{
			toolStripButton6.Checked = !toolStripButton6.Checked;
		}

		private void toolStripButton7_Click(object sender, EventArgs e)
		{
			toolStripButton7.Checked = !toolStripButton7.Checked;
		}

		private void toolStripButton8_Click(object sender, EventArgs e)
		{
			toolStripButton8.Checked = !toolStripButton8.Checked;
		}

		private void toolStripButton9_Click(object sender, EventArgs e)
		{
			toolStripButton9.Checked = !toolStripButton9.Checked;
		}

		private void toolStripButton10_Click(object sender, EventArgs e)
		{
			toolStripButton10.Checked = !toolStripButton10.Checked;
		}

	}
}
