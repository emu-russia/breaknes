using SharpTools;

namespace Breaknes
{
	public partial class FormDebugger : Form
	{
		Dictionary<string, List<LogicValue>> waves = new();

		/// <summary>
		/// Snatch Waves
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void toolStripButton4_Click(object sender, EventArgs e)
		{
			if (board_ctrl.Paused && board_ctrl.SimulationStarted)
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
		}

		public void ResetWaves()
		{
			ValueChangeData[] vcd = new ValueChangeData[0];
			wavesControl1.PlotWaves(vcd, 0);
			waves = new();
		}

		public void UpdateWaves()
		{
			// Update dump

			var info = BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_PPU);
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

			info = BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_Core);
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

			info = BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_APU);
			foreach (var entry in info)
			{
				if (entry.name == "/CLK")		// Skip APU /Clk
					continue;

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

		private void toolStripButton5_Click(object sender, EventArgs e)
		{
			ResetWaves();
		}

		bool SignalFilter(BreaksCore.DebugInfoEntry entry)
		{
			if (entry.bits != 1)
				return false;

			if (entry.category == BreaksCore.CORE_BRK_CATEGORY && bRKInterruptsToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.CORE_DISP_CATEGORY && dispatcherToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.CORE_ALU_CATEGORY && aLURelatedToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.CORE_BOPS_CATEGORY && bottomOpsToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.CORE_FOPS_CATEGORY && flagOpsToolStripMenuItem.Checked) return true;

			if (entry.category == BreaksCore.APU_CLKS_CATEGORY && clocksToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.APU_CORE_CATEGORY && coreToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.APU_DMA_CATEGORY && dMAToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.APU_REGOPS_CATEGORY && regOpsToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.APU_LC_CATEGORY && lengthCountersToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.APU_WIRES_CATEGORY && othersToolStripMenuItem.Checked) return true;

			if (entry.category == BreaksCore.PPU_CLKS_CATEGORY && clocksToolStripMenuItem1.Checked) return true;
			if (entry.category == BreaksCore.PPU_CPU_CATEGORY && cPUIFToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.PPU_CTRL_CATEGORY && cTRLRegsToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.PPU_HV_CATEGORY && hVToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.PPU_MUX_CATEGORY && mUXToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.PPU_SPG_CATEGORY && dataReaderToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.PPU_CRAM_CATEGORY && cRAMToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.PPU_VRAM_CATEGORY && vRAMControlToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.PPU_FSM_CATEGORY && fSMToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.PPU_EVAL_CATEGORY && oAMEvalToolStripMenuItem.Checked) return true;
			if (entry.category == BreaksCore.PPU_WIRES_CATEGORY && othersToolStripMenuItem1.Checked) return true;

			return false;
		}

		private void bRKInterruptsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			bRKInterruptsToolStripMenuItem.Checked = !bRKInterruptsToolStripMenuItem.Checked;
		}

		private void dispatcherToolStripMenuItem_Click(object sender, EventArgs e)
		{
			dispatcherToolStripMenuItem.Checked = !dispatcherToolStripMenuItem.Checked;
		}

		private void aLURelatedToolStripMenuItem_Click(object sender, EventArgs e)
		{
			aLURelatedToolStripMenuItem.Checked = !aLURelatedToolStripMenuItem.Checked;
		}

		private void bottomOpsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			bottomOpsToolStripMenuItem.Checked = !bottomOpsToolStripMenuItem.Checked;
		}

		private void flagOpsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			flagOpsToolStripMenuItem.Checked = !flagOpsToolStripMenuItem.Checked;
		}

		private void clocksToolStripMenuItem_Click(object sender, EventArgs e)
		{
			clocksToolStripMenuItem.Checked = !clocksToolStripMenuItem.Checked;
		}

		private void coreToolStripMenuItem_Click(object sender, EventArgs e)
		{
			coreToolStripMenuItem.Checked = !coreToolStripMenuItem.Checked;
		}

		private void dMAToolStripMenuItem_Click(object sender, EventArgs e)
		{
			dMAToolStripMenuItem.Checked = !dMAToolStripMenuItem.Checked;
		}

		private void regOpsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			regOpsToolStripMenuItem.Checked = !regOpsToolStripMenuItem.Checked;
		}

		private void lengthCountersToolStripMenuItem_Click(object sender, EventArgs e)
		{
			lengthCountersToolStripMenuItem.Checked = !lengthCountersToolStripMenuItem.Checked;
		}

		private void othersToolStripMenuItem_Click(object sender, EventArgs e)
		{
			othersToolStripMenuItem.Checked = !othersToolStripMenuItem.Checked;
		}

		private void clocksToolStripMenuItem1_Click(object sender, EventArgs e)
		{
			clocksToolStripMenuItem1.Checked = !clocksToolStripMenuItem1.Checked;
		}

		private void cPUIFToolStripMenuItem_Click(object sender, EventArgs e)
		{
			cPUIFToolStripMenuItem.Checked = !cPUIFToolStripMenuItem.Checked;
		}

		private void cTRLRegsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			cTRLRegsToolStripMenuItem.Checked = !cTRLRegsToolStripMenuItem.Checked;
		}

		private void hVToolStripMenuItem_Click(object sender, EventArgs e)
		{
			hVToolStripMenuItem.Checked = !hVToolStripMenuItem.Checked;
		}

		private void mUXToolStripMenuItem_Click(object sender, EventArgs e)
		{
			mUXToolStripMenuItem.Checked = !mUXToolStripMenuItem.Checked;
		}

		private void dataReaderToolStripMenuItem_Click(object sender, EventArgs e)
		{
			dataReaderToolStripMenuItem.Checked = !dataReaderToolStripMenuItem.Checked;
		}

		private void cRAMToolStripMenuItem_Click(object sender, EventArgs e)
		{
			cRAMToolStripMenuItem.Checked = !cRAMToolStripMenuItem.Checked;
		}

		private void vRAMControlToolStripMenuItem_Click(object sender, EventArgs e)
		{
			vRAMControlToolStripMenuItem.Checked = !vRAMControlToolStripMenuItem.Checked;
		}

		private void fSMToolStripMenuItem_Click(object sender, EventArgs e)
		{
			fSMToolStripMenuItem.Checked = !fSMToolStripMenuItem.Checked;
		}

		private void oAMEvalToolStripMenuItem_Click(object sender, EventArgs e)
		{
			oAMEvalToolStripMenuItem.Checked = !oAMEvalToolStripMenuItem.Checked;
		}

		private void othersToolStripMenuItem1_Click(object sender, EventArgs e)
		{
			othersToolStripMenuItem1.Checked = !othersToolStripMenuItem1.Checked;
		}
	}
}
