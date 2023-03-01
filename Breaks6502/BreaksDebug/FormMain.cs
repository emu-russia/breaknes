using System.Runtime.InteropServices;
using Be.Windows.Forms;
using System.IO;
using Newtonsoft.Json;
using SharpTools;

namespace BreaksDebug
{
	public partial class FormMain : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		CoreDebug sys = new CoreDebug();
		string testAsmName = "Test.asm";
		string MarkdownDir = "WikiMarkdown";        // To dump the state as Markdown text, you need to create this directory.
		string MarkdownImgDir = "imgstore/ops";
		string WikiRoot = "/BreakingNESWiki/";
		bool MarkdownOutput = false;

		public FormMain()
		{
			InitializeComponent();
		}

		private void Form1_Load(object sender, EventArgs e)
		{
#if DEBUG
			AllocConsole();
#endif

			BreaksCoreInterop.CreateBoard("Bogus", "None", "None", "None");

			SetStyle(ControlStyles.OptimizedDoubleBuffer, true);

			toolStripButton4.Checked = true;    // /NMI
			toolStripButton5.Checked = true;    // /IRQ
			toolStripButton6.Checked = false;   // /RES
			toolStripButton3.Checked = false;   // SO
			toolStripButton2.Checked = true;    // RES
			ButtonsToPads();

			UpdateAll();

			// Select the operating mode - manual or automatic unit test.

			string[] args = Environment.GetCommandLineArgs();

			UnitTestMode = args.Length > 1;
			if (UnitTestMode)
			{
				TestInputJson = args[1];
				Console.WriteLine("Running UnitTest from " + TestInputJson);

				string json = File.ReadAllText(TestInputJson);

				testParam = JsonConvert.DeserializeObject<UnitTestParam>(json);

				if (InitUnitTest() == 0)
				{
					StartUnitTest();
				}
			}
			else
			{
				if (File.Exists(testAsmName))
				{
					LoadAsm(testAsmName);
					Assemble();
				}
			}

			MarkdownOutput = Directory.Exists(MarkdownDir);
			if (MarkdownOutput && !UnitTestMode)
			{
				Directory.CreateDirectory(MarkdownDir + "/" + MarkdownImgDir);
			}
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormAbout dlg = new FormAbout();
			dlg.ShowDialog();
		}

		private void toolStripButton1_Click(object sender, EventArgs e)
		{
			if (!UnitTestMode)
			{
				Step();
			}
		}

		private void Form1_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.F11 && !UnitTestMode)
			{
				Step();
			}
		}

		void Step()
		{
			bool trace = true;

			if (UnitTestMode)
			{
				trace = testParam.TraceMemOps;
			}

			BreaksCoreInterop.Step();

			if (!UnitTestMode)
			{
				UpdateAll();
			}
		}

		void UpdateCycleStats()
		{
			toolStripStatusLabel1.Text = "Cycle: " + BreaksCoreInterop.GetPHICounter().ToString();
		}

		void UpdateMemoryDump()
		{
			hexBox1.Refresh();
		}

		void UpdateCpuPads()
		{
			sys.cpu_pads.n_NMI = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "/NMI");
			sys.cpu_pads.n_IRQ = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "/IRQ");
			sys.cpu_pads.n_RES = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "/RES");
			sys.cpu_pads.PHI0 = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "PHI0");
			sys.cpu_pads.RDY = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "RDY");
			sys.cpu_pads.SO = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "SO");
			sys.cpu_pads.PHI1 = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "PHI1");
			sys.cpu_pads.PHI2 = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "PHI2");
			sys.cpu_pads.RnW = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "R/W");
			sys.cpu_pads.SYNC = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "SYNC");
			sys.cpu_pads.A = (UInt16)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "A");
			sys.cpu_pads.D = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "D");

			propertyGrid1.SelectedObject = sys.cpu_pads;
		}

		void UpdateState()
		{
			if (sys.cpu_pads.PHI1 != 0)
			{
				label3.Text = "PHI1 (Set Address + R/W Mode)";
			}

			if (sys.cpu_pads.PHI2 != 0)
			{
				label3.Text = "PHI2 (Read/Write Mem)";
			}
		}

		void UpdateCpuDebugInfo()
		{
			var regsBuses = sys.GetRegsBuses();
			propertyGrid2.SelectedObject = regsBuses;
			var internals = sys.GetInternals();
			propertyGrid3.SelectedObject = internals;
			var decoderOut = sys.GetDecoder();
			propertyGrid4.SelectedObject = decoderOut;
			propertyGrid4.ExpandAllGridItems();
			var commands = sys.GetCommands();
			propertyGrid5.SelectedObject = commands;
			propertyGrid5.ExpandAllGridItems();

			dataPathView1.ShowCpuCommands(commands, sys.cpu_pads.PHI1 != 0);

			UpdateDisasm(regsBuses.IRForDisasm, regsBuses.PDForDisasm, internals.FETCH != 0);

			// Dump the state of the processor in a representative form for the wiki.

			if (MarkdownOutput)
			{
				MarkdownDump.ExportStepMarkdown(regsBuses, internals, decoderOut, commands,
					sys.cpu_pads.PHI1, sys.cpu_pads.PHI2, dataPathView1, MarkdownDir, MarkdownImgDir, WikiRoot);
			}
		}

		void UpdateDisasm(byte ir, byte pd, bool fetch)
		{
			label9.Text = QuickDisasm.Disasm(ir) + " (current)";
			if (fetch)
			{
				label9.Text += ", " + QuickDisasm.Disasm(pd) + " (fetched)";
			}
		}

		void UpdateAll()
		{
			UpdateCycleStats();
			UpdateMemoryDump();
			UpdateCpuPads();
			UpdateState();
			UpdateCpuDebugInfo();
		}

		void LoadMemDump(byte[] dump)
		{
			List<BreaksCore.MemDesciptor> mem = BreaksCore.GetMemoryLayout();

			int descrID = -1;

			for (int i = 0; i < mem.Count; i++)
			{
				if (mem[i].name == "WRAM")
				{
					descrID = i;
					break;
				}
			}

			if (descrID >= 0)
			{
				BreaksCore.WriteMem(descrID, dump);
				if (!UnitTestMode)
				{
					hexBox1.ByteProvider = new DynamicByteProvider(dump);
					hexBox1.Refresh();
				}
			}
		}

		byte[] SaveMemDump()
		{
			List<BreaksCore.MemDesciptor> mem = BreaksCore.GetMemoryLayout();

			int descrID = -1;

			for (int i = 0; i < mem.Count; i++)
			{
				if (mem[i].name == "WRAM")
				{
					descrID = i;
					break;
				}
			}

			if (descrID >= 0)
			{
				byte[] buf = new byte[mem[descrID].size];
				BreaksCore.DumpMem(descrID, buf);
				return buf;
			}

			return null;
		}

		private void loadMemoryDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DialogResult res = openFileDialog2.ShowDialog();
			if (res == DialogResult.OK)
			{
				byte[] dump = File.ReadAllBytes(openFileDialog2.FileName);
				LoadMemDump(dump);
				Console.WriteLine("Loaded memory dump: " + openFileDialog2.FileName);
			}
		}

		private void saveTheMemoryDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DialogResult res = saveFileDialog1.ShowDialog();
			if (res == DialogResult.OK)
			{
				var dump = SaveMemDump();
				File.WriteAllBytes(saveFileDialog1.FileName, dump);
				Console.WriteLine("Saved memory dump: " + saveFileDialog1.FileName);
			}
		}

		private void loadAssemblySourceToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DialogResult res = openFileDialog1.ShowDialog();
			if (res == DialogResult.OK)
			{
				LoadAsm(openFileDialog1.FileName);
				tabControl2.SelectTab(1);
			}
		}

		void LoadAsm(string filename)
		{
			richTextBox1.Text = File.ReadAllText(filename);
			Console.WriteLine("Loaded " + filename);
		}

		private void button2_Click(object sender, EventArgs e)
		{
			Assemble();
		}

		void Assemble()
		{
			Console.WriteLine("Assemble");

			byte[] buffer = new byte[0x10000];

			int num_err = BreaksCoreInterop.Assemble(richTextBox1.Text, buffer);
			if (num_err != 0)
			{
				MessageBox.Show(
					"Errors occurred during the assembling process. Num erros: " + num_err.ToString(),
					"Assembling error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				return;
			}

			LoadMemDump(buffer);
		}

		void ButtonsToPads()
		{
			BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "/NMI", (UInt32)(toolStripButton4.Checked ? 1 : 0));
			BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "/IRQ", (UInt32)(toolStripButton5.Checked ? 1 : 0));
			BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "/RES", (UInt32)(toolStripButton6.Checked ? 1 : 0));
			BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "SO", (UInt32)(toolStripButton3.Checked ? 1 : 0));
			BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "RDY", (UInt32)(toolStripButton2.Checked ? 1 : 0));
		}

		private void toolStripButton4_CheckedChanged(object sender, EventArgs e)
		{
			Console.WriteLine("/NMI changed");
			ButtonsToPads();
			UpdateCpuPads();
		}

		private void toolStripButton5_CheckedChanged(object sender, EventArgs e)
		{
			Console.WriteLine("/IRQ changed");
			ButtonsToPads();
			UpdateCpuPads();
		}

		private void toolStripButton6_CheckedChanged(object sender, EventArgs e)
		{
			Console.WriteLine("/RES changed");
			ButtonsToPads();
			UpdateCpuPads();
		}

		private void toolStripButton3_CheckedChanged(object sender, EventArgs e)
		{
			Console.WriteLine("SO changed");
			ButtonsToPads();
			UpdateCpuPads();
		}

		private void toolStripButton2_CheckedChanged(object sender, EventArgs e)
		{
			Console.WriteLine("RDY changed");
			ButtonsToPads();
			UpdateCpuPads();
		}
	}
}
