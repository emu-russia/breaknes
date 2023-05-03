using SharpTools;
using System.Collections.Generic;
using System.Windows.Forms;
using System;

namespace PPUPlayer
{
	public partial class FormMain : Form
	{
		Dictionary<string, List<LogicValue>> waves = new();

		/// <summary>
		/// Snatch Waves
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void toolStripButton5_Click(object sender, EventArgs e)
		{
			if (Paused && SimulationStarted)
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

			var info = BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_PPU);
			foreach (var entry in info)
			{
				if (entry.bits == 1 && entry.category == BreaksCore.PPU_FSM_CATEGORY)
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

		private void toolStripButton6_Click(object sender, EventArgs e)
		{
			ResetWaves();
		}
	}
}