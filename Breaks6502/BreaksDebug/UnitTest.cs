using Newtonsoft.Json;
using SharpTools;

namespace BreaksDebug
{
	public partial class FormMain : Form
	{

		class UnitTestParam
		{
			public bool CompileFromSource = true;
			public string MemDumpInput = "mem.bin";
			public string AsmSource = "Test.asm";
			public bool RunUntilBrk = true;
			public bool RunCycleAmount = true;
			public long CycleMax = 10000;
			public bool RunUntilPC = false;
			public string PC = "0x0";
			public bool TraceMemOps = true;
			public bool TraceCLK = true;
			public bool DumpMem = true;
			public string JsonResult = "res.json";
			public string MemDumpOutput = "mem2.bin";
		}

		class UnitTestResult
		{
			public string A { get; set; } = "0x00";
			public string X { get; set; } = "0x00";
			public string Y { get; set; } = "0x00";
			public string S { get; set; } = "0x00";
			public string PC { get; set; } = "0x0000";
			public int C { get; set; } = 0;
			public int Z { get; set; } = 0;
			public int I { get; set; } = 0;
			public int D { get; set; } = 0;
			public int V { get; set; } = 0;
			public int N { get; set; } = 0;
			public UInt64 CLK { get; set; } = 0;
		}

		bool UnitTestMode = false;
		string TestInputJson;
		UnitTestParam testParam;
		UInt64 UnitTestPhiCounter = 0;
		UInt64 UnitTestPhiTraceCounter = 0;

		int Strtol(string str)
		{
			str = str.Trim();

			return (str.Contains("0x") || str.Contains("0X")) ?
				Convert.ToInt32(str, 16) :
					str[0] == '0' ? Convert.ToInt32(str, 8) : Convert.ToInt32(str, 10);
		}

		int InitUnitTest()
		{
			if (testParam.CompileFromSource)
			{
				LoadAsm(testParam.AsmSource);
				Assemble();
			}
			else
			{
				byte[] dump = File.ReadAllBytes(testParam.MemDumpInput);
				LoadMemDump(dump);
			}

			// Perform the PreBRK PowerUP sequence

			for (int i = 0; i < 8; i++)
			{
				Step();
			}

			// Cancel Reset and wait for Reset BRK to complete

			toolStripButton6.Checked = true;

			int timeout = 0x100;

			while (timeout-- != 0)
			{
				Step();

				var regsBuses = sys.GetRegsBuses();
				if (regsBuses.IRForDisasm != 0x00)
				{
					break;
				}
			}

			if (timeout == 0)
			{
				Console.WriteLine("The operation of the simulator is broken. BRK-sequence is not completed.");
				return -1;
			}

			return 0;
		}

		void StartUnitTest()
		{
			ushort BreakOnPC = (ushort)Strtol(testParam.PC);

			while (true)
			{
				Step();
				UnitTestPhiCounter++;
				UnitTestPhiTraceCounter++;
				if (UnitTestPhiTraceCounter > 1000000 * 2)
				{
					// Output statistics every million cycles
					if (testParam.TraceCLK)
					{
						Console.WriteLine("CLK: " + (UnitTestPhiCounter / 2).ToString());
					}
					UnitTestPhiTraceCounter = 0;
				}

				if (BreaksCoreInterop.GetPHICounter() >= testParam.CycleMax && testParam.RunCycleAmount)
				{
					break;
				}

				var regsBuses = sys.GetRegsBuses();
				if (regsBuses.IRForDisasm == 0x00 && testParam.RunUntilBrk)
				{
					break;
				}
				if (regsBuses.PCForUnitTest == BreakOnPC && testParam.RunUntilPC)
				{
					break;
				}
			}

			SaveUnitTestResults();
		}

		void SaveUnitTestResults()
		{
			if (testParam.DumpMem)
			{
				byte[] dump = SaveMemDump();
				File.WriteAllBytes(testParam.MemDumpOutput, dump);
			}

			UnitTestResult res = new UnitTestResult();

			var regsBuses = sys.GetRegsBuses();

			res.A = regsBuses.AC;
			res.X = regsBuses.X;
			res.Y = regsBuses.Y;
			res.S = regsBuses.S;
			res.PC = "0x" + regsBuses.PCForUnitTest.ToString("X4");
			res.C = regsBuses.C_OUT;
			res.Z = regsBuses.Z_OUT;
			res.I = regsBuses.I_OUT;
			res.D = regsBuses.D_OUT;
			res.V = regsBuses.V_OUT;
			res.N = regsBuses.N_OUT;
			res.CLK = UnitTestPhiCounter / 2;

			string json = JsonConvert.SerializeObject(res);

			File.WriteAllText(testParam.JsonResult, json);

			Close();
		}

	}
}
