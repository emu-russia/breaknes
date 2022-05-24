// Working with PPU memory dumps

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

using System.IO;

// I don't think any maniac would want to load and save Temp OAM. But if we have to, we will.

namespace PPUPlayer
{
	public partial class Form1 : Form
	{
		internal const string VRAM_NAME = "VRAM";
		internal const string CHR_ROM_NAME = "CHR-ROM";
		internal const string OAM_NAME = "OAM";
		internal const string CRAM_NAME = "Color RAM";

		/// <summary>
		/// File -> PPU Mem
		/// </summary>
		void LoadMemoryDump(string filename, string memDescr)
		{
			List<BreaksCore.MemDesciptor> mem = BreaksCore.GetMemoryLayout();

			int descrID = 0;

			foreach (var descr in mem)
			{
				if (descr.name == memDescr)
				{
					byte[] data = File.ReadAllBytes(filename);
					if (data.Length < descr.size)
					{
						throw new Exception("The dump size does not match the size of the PPU memory region. You can specify a larger file, only the desired part will be loaded. But smaller files cannot be used.");
					}
					BreaksCore.WriteMem(descrID, data);
				}

				descrID++;
			}
		}

		/// <summary>
		/// PPU Mem -> File
		/// </summary>
		void SaveMemoryDump(string filename, string memDescr)
		{
			List<BreaksCore.MemDesciptor> mem = BreaksCore.GetMemoryLayout();

			int descrID = 0;

			foreach (var descr in mem)
			{
				if (descr.name == memDescr)
				{
					byte[] data = new byte[descr.size];
					BreaksCore.DumpMem(descrID, data);
					File.WriteAllBytes(filename, data);
				}

				descrID++;
			}
		}

		private void loadVRAMToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
				return;

			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				LoadMemoryDump(openFileDialog1.FileName, VRAM_NAME);
			}
		}

		private void loadCHRToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
				return;

			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				LoadMemoryDump(openFileDialog1.FileName, CHR_ROM_NAME);
			}
		}

		private void loadOAMToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
				return;

			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				LoadMemoryDump(openFileDialog1.FileName, OAM_NAME);
			}
		}

		private void loadCRAMToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
				return;

			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				LoadMemoryDump(openFileDialog1.FileName, CRAM_NAME);
			}
		}

		private void saveVRAMToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
				return;

			if (saveFileDialog1.ShowDialog() == DialogResult.OK)
			{
				SaveMemoryDump(saveFileDialog1.FileName, VRAM_NAME);
			}
		}

		private void saveCHRToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
				return;

			if (saveFileDialog1.ShowDialog() == DialogResult.OK)
			{
				SaveMemoryDump(saveFileDialog1.FileName, CHR_ROM_NAME);
			}
		}

		private void saveOAMToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
				return;

			if (saveFileDialog1.ShowDialog() == DialogResult.OK)
			{
				SaveMemoryDump(saveFileDialog1.FileName, OAM_NAME);
			}
		}

		private void saveCRAMToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
				return;

			if (saveFileDialog1.ShowDialog() == DialogResult.OK)
			{
				SaveMemoryDump(saveFileDialog1.FileName, CRAM_NAME);
			}
		}

		private void loadNintendulatorPPUDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (!SimulationStarted)
				return;

			// TBD.
		}
	}
}
