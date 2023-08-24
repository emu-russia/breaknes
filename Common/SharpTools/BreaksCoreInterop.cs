using System;
using System.Runtime.InteropServices;
using static SharpTools.CoreDebug;

namespace SharpTools
{
	public class BreaksCore
	{
		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int Assemble(string text, byte[] buffer);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void CreateBoard(string boardName, string apu, string ppu, string p1);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DestroyBoard();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int InsertCartridge(byte[] nesImage, int nesImageSize);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void EjectCartridge();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void Step();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void Reset();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool InResetState();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern long GetACLKCounter();
		
		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern long GetPHICounter();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SampleAudioSignal(out float sample);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void LoadRegDump([In, Out][MarshalAs(UnmanagedType.LPArray)] byte[] data, int data_size);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void EnablePpuRegDump(bool enable, string regdump_dir);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void EnableApuRegDump(bool enable, string regdump_dir);

		[StructLayout(LayoutKind.Explicit)]
		public struct AudioSignalFeatures
		{
			[FieldOffset(0)]
			public int SampleRate;      // The sampling frequency of the audio signal (samples per "virtual" second). The audio is actually sampled every PHI (core clock) cycle.
			[FieldOffset(4)]
			public float AUXA_LowLevel;		// Lower signal level for AUX_A (mV)
			[FieldOffset(8)]
			public float AUXA_HighLevel;	// Upper signal level for AUX_A (mV)
			[FieldOffset(12)]
			public float AUXB_LowLevel;		// Lower signal level for AUX_B (mV)
			[FieldOffset(16)]
			public float AUXB_HighLevel;    // Upper signal level for AUX_B (mV)
		}

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void GetApuSignalFeatures(out AudioSignalFeatures features);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern long GetPCLKCounter();

		[StructLayout(LayoutKind.Explicit)]
		public struct VideoOutSample
		{
			[FieldOffset(0)]
			public float composite;
			[FieldOffset(0)]
			public byte RED;
			[FieldOffset(1)]
			public byte GREEN;
			[FieldOffset(2)]
			public byte BLUE;
			[FieldOffset(3)]
			public byte nSYNC;
			[FieldOffset(0)]
			public UInt16 raw;      // sBGRLLCCCC (Sync || Tint Blue || Tint Green || Tint Red || Luma[2] || Chroma[4])
		}

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SampleVideoSignal(out VideoOutSample sample);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetHCounter();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetVCounter();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void RenderAlwaysEnabled(bool enable);

		[StructLayout(LayoutKind.Explicit)]
		public struct VideoSignalFeatures
		{
			[FieldOffset(0)]
			public int SamplesPerPCLK;
			[FieldOffset(4)]
			public int PixelsPerScan;       // Excluding Dot Crawl
			[FieldOffset(8)]
			public int ScansPerField;
			[FieldOffset(12)]
			public int BackPorchSize;       // BackPorch size in pixels.
			[FieldOffset(16)]
			public int Composite;           // 1: Composite, 0: RGB
			[FieldOffset(20)]
			public float BlackLevel;        // IRE = 0
			[FieldOffset(24)]
			public float WhiteLevel;        // IRE = 110
			[FieldOffset(28)]
			public float SyncLevel;         // SYNC low level
			[FieldOffset(32)]
			public int PhaseAlteration;     // 1: PAL
		}

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void GetPpuSignalFeatures(out VideoSignalFeatures features);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void ConvertRAWToRGB(UInt16 raw, out byte r, out byte g, out byte b);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SetRAWColorMode(bool enable);

		/// <summary>
		/// How to handle the OAM Corruption effect.
		/// </summary>
		public enum OAMDecayBehavior
		{
			Keep = 0,
			Evaporate,
			ToZero,
			ToOne,
			Randomize,
		};

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SetOamDecayBehavior(OAMDecayBehavior behavior);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SetNoiseLevel(float volts);

		#region "Core IO Api"

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int IOCreateInstance(UInt32 device_id);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void IOAttach(int port, int handle);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void IODetach(int port, int handle);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void IOSetState(int handle, int io_state, UInt32 value);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern UInt32 IOGetState(int handle, int io_state);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int IOGetNumStates(int handle);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void IOGetStateName(int handle, int io_state, [In, Out][MarshalAs(UnmanagedType.LPStr)] char[] name, int name_size);

		#endregion "Core IO Api"


		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		unsafe public struct CpuDebugInfoRaw
		{
			// Regs & Buses

			public byte SB;
			public byte DB;
			public byte ADL;
			public byte ADH;

			public byte IR;
			public byte PD;
			public byte Y;
			public byte X;
			public byte S;
			public byte AI;
			public byte BI;
			public byte ADD;
			public byte AC;
			public byte PCL;
			public byte PCH;
			public byte PCLS;
			public byte PCHS;
			public byte ABL;
			public byte ABH;
			public byte DL;
			public byte DOR;

			public byte C_OUT;
			public byte Z_OUT;
			public byte I_OUT;
			public byte D_OUT;
			public byte B_OUT;
			public byte V_OUT;
			public byte N_OUT;

			// Internals

			public byte n_PRDY;
			public byte n_NMIP;
			public byte n_IRQP;
			public byte RESP;
			public byte BRK6E;
			public byte BRK7;
			public byte DORES;
			public byte n_DONMI;
			public byte n_T2;
			public byte n_T3;
			public byte n_T4;
			public byte n_T5;
			public byte T0;
			public byte n_T0;
			public byte n_T1X;
			public byte Z_IR;
			public byte FETCH;
			public byte n_ready;
			public byte WR;
			public byte TRES2;
			public byte ACRL1;
			public byte ACRL2;
			public byte T1;
			public byte RMW_T6;
			public byte RMW_T7;
			public byte ENDS;
			public byte ENDX;
			public byte TRES1;
			public byte n_TRESX;
			public byte BRFW;
			public byte n_BRTAKEN;
			public byte ACR;
			public byte AVR;

			// Decoder

			public fixed byte decoder_out[130];

			// Control commands

			public fixed byte cmd[(int)ControlCommand.Max];
			public byte n_ACIN;
			public byte n_DAA;
			public byte n_DSA;
			public byte n_1PC;          // From Dispatcher
		}

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void GetAllCoreDebugInfo(out CpuDebugInfoRaw info);

		// Here goes DebugHub

		public enum DebugInfoType : int
		{
			DebugInfoType_Unknown = 0,
			DebugInfoType_Test,
			DebugInfoType_Core,
			DebugInfoType_CoreRegs,
			DebugInfoType_APU,
			DebugInfoType_APURegs,
			DebugInfoType_PPU,
			DebugInfoType_PPURegs,
			DebugInfoType_Board,
			DebugInfoType_Cart,
		};

		public const string CORE_BRK_CATEGORY = "Core BRK";
		public const string CORE_DISP_CATEGORY = "Core Dispatcher";
		public const string CORE_ALU_CATEGORY = "Core ALU Related";
		public const string CORE_BOPS_CATEGORY = "Core Bops";
		public const string CORE_FOPS_CATEGORY = "Core Fops";
		public const string CORE_REGS_CATEGORY = "Core Regs";
		public const string APU_CLKS_CATEGORY = "APU Clocks";
		public const string APU_CORE_CATEGORY = "APU Core";
		public const string APU_DMA_CATEGORY = "APU DMA";
		public const string APU_REGOPS_CATEGORY = "APU RegOps";
		public const string APU_LC_CATEGORY = "APU LC";
		public const string APU_WIRES_CATEGORY = "APU Wires";
		public const string APU_REGS_CATEGORY = "APU Regs";
		public const string CRAM_NAME = "Color RAM";
		public const string OAM_NAME = "OAM";
		public const string OAM2_NAME = "Temp OAM";
		public const string PPU_CLKS_CATEGORY = "PPU Clocks";
		public const string PPU_CPU_CATEGORY = "PPU CPU I/F";
		public const string PPU_CTRL_CATEGORY = "PPU CTRL";
		public const string PPU_HV_CATEGORY = "PPU H/V";
		public const string PPU_MUX_CATEGORY = "PPU MUX";
		public const string PPU_SPG_CATEGORY = "PPU DataReader";    // aka Still Picture Generator
		public const string PPU_CRAM_CATEGORY = "PPU CRAM";
		public const string PPU_VRAM_CATEGORY = "PPU VRAM";
		public const string PPU_FSM_CATEGORY = "PPU FSM";
		public const string PPU_EVAL_CATEGORY = "PPU Eval";
		public const string PPU_WIRES_CATEGORY = "PPU Wires";       // Uncategorized PPU tentacles
		public const string PPU_REGS_CATEGORY = "PPU Regs";
		
		public const string BOARD_CATEGORY = "Board";
		public const string BANKED_SRAM_NAME = "BankedSRAM";
		public const string WRAM_NAME = "WRAM";
		public const string VRAM_NAME = "VRAM";				// aka CIRAM
		public const string CHR_ROM_NAME = "CHR-ROM";
		public const string NROM_CATEGORY = "NROM";

		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		unsafe struct DebugInfoEntryRaw
		{
			public fixed byte category[32];
			public fixed byte name[32];
			public byte bits;
			public UInt32 value;
		}

		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		unsafe struct MemDesciptorRaw
		{
			public fixed byte name[32];
			public int size;
		};

		public class DebugInfoEntry
		{
			public string category = "";
			public string name = "";
			public byte bits = 0;
			public UInt32 value = 0;
		}

		public class MemDesciptor
		{
			public string name = "";
			public int size;
		}


		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetDebugInfoEntryCount(DebugInfoType type);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetDebugInfo(DebugInfoType type, IntPtr entries);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetDebugInfoByName(DebugInfoType type, ref DebugInfoEntryRaw entry);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int SetDebugInfoByName(DebugInfoType type, ref DebugInfoEntryRaw entry);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetMemLayout();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetMemDescriptor(int descrID, IntPtr descr);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DumpMem(int descrID, [In, Out] [MarshalAs(UnmanagedType.LPArray)] byte[] ptr);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void WriteMem(int descrID, [In, Out][MarshalAs(UnmanagedType.LPArray)] byte[] ptr);


		public static List<MemDesciptor> GetMemoryLayout()
		{
			List<MemDesciptor> list = new();

			int count = GetMemLayout();

			IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf<BreaksCore.MemDesciptorRaw>());
			if (ptr == IntPtr.Zero)
			{
				throw new Exception("AllocHGlobal failed!");
			}

			for (int i=0; i<count; i++)
			{
				GetMemDescriptor(i, ptr);

#pragma warning disable CS8605 // Unboxing a possibly null value.
				BreaksCore.MemDesciptorRaw raw = (BreaksCore.MemDesciptorRaw)Marshal.PtrToStructure(ptr, typeof(BreaksCore.MemDesciptorRaw));
#pragma warning restore CS8605 // Unboxing a possibly null value.

				MemDesciptor descr = new();

				unsafe
				{
					for (int n = 0; n < 32; n++)
					{
						if (raw.name[n] != 0)
						{
							descr.name += (char)raw.name[n];
						}
						else
						{
							break;
						}
					}

					descr.size = raw.size;
				}

				list.Add(descr);
			}

			Marshal.FreeHGlobal(ptr);

			return list;
		}


		public static List<DebugInfoEntry> GetDebugInfo(DebugInfoType type)
		{
			List<DebugInfoEntry> list = new();

			var count = BreaksCore.GetDebugInfoEntryCount(type);

			//Console.WriteLine("Size of entry: " + Marshal.SizeOf<BreaksCore.DebugInfoEntryRaw>().ToString());

			IntPtr ptr = Marshal.AllocHGlobal(count * Marshal.SizeOf<BreaksCore.DebugInfoEntryRaw>());
			if (ptr == IntPtr.Zero)
			{
				throw new Exception("AllocHGlobal failed!");
			}

			BreaksCore.GetDebugInfo(type, ptr);

			IntPtr p = ptr;

			for (int i = 0; i < count; ++i)
			{
#pragma warning disable CS8605 // Unboxing a possibly null value.
				BreaksCore.DebugInfoEntryRaw raw = (BreaksCore.DebugInfoEntryRaw)Marshal.PtrToStructure(p, typeof(BreaksCore.DebugInfoEntryRaw));
#pragma warning restore CS8605 // Unboxing a possibly null value.

				DebugInfoEntry entry = new();

				unsafe
				{
					for (int n = 0; n < 32; n++)
					{
						if (raw.category[n] != 0)
						{
							entry.category += (char)raw.category[n];
						}
						else
						{
							break;
						}
					}

					for (int n = 0; n < 32; n++)
					{
						if (raw.name[n] != 0)
						{
							entry.name += (char)raw.name[n];
						}
						else
						{
							break;
						}
					}

					if (type == DebugInfoType.DebugInfoType_PPU)
					{
						if (Visual2C02Mapping) entry.name = Visual2C02.ToVisual2C02(entry.name);
					}

					if (type == DebugInfoType.DebugInfoType_APU)
					{
						if (Visual2A03Mapping) entry.name = Visual2A03.ToVisual2A03(entry.name);
					}

					entry.bits = raw.bits;
					entry.value = raw.value;
				}

				list.Add(entry);

				p += Marshal.SizeOf<BreaksCore.DebugInfoEntryRaw>();
			}

			Marshal.FreeHGlobal(ptr);

			return list;
		}

		public static List<DebugInfoEntry> GetTestDebugInfo()
		{
			List<DebugInfoEntry> list = new();

			DebugInfoEntry testEntry = new();

			testEntry.category = "Test Category";
			testEntry.name = "Test Entry";
			testEntry.bits = 8;
			testEntry.value = 123;

			list.Add(testEntry);

			return list;
		}

		public static UInt32 GetDebugInfoByName (DebugInfoType type, string category, string name)
		{
			DebugInfoEntryRaw entry = new();

			if (type == DebugInfoType.DebugInfoType_PPU)
			{
				if (Visual2C02Mapping) name = Visual2C02.FromVisual2C02(name);
			}

			if (type == DebugInfoType.DebugInfoType_APU)
			{
				if (Visual2A03Mapping) name = Visual2A03.FromVisual2A03(name);
			}

			unsafe
			{
				for (int i = 0; i < 32; i++)
				{
					if (i < category.Length)
						entry.category[i] = (byte)category[i];
					else
						entry.category[i] = 0;

					if (i < name.Length)
						entry.name[i] = (byte)name[i];
					else
						entry.name[i] = 0;
				}
			}

			GetDebugInfoByName(type, ref entry);

			return entry.value;
		}

		public static void SetDebugInfoByName(DebugInfoType type, string category, string name, UInt32 value)
		{
			DebugInfoEntryRaw entry = new();

			if (type == DebugInfoType.DebugInfoType_PPU)
			{
				if (Visual2C02Mapping) name = Visual2C02.FromVisual2C02(name);
			}

			if (type == DebugInfoType.DebugInfoType_APU)
			{
				if (Visual2A03Mapping) name = Visual2A03.FromVisual2A03(name);
			}

			unsafe
			{
				for (int i = 0; i < 32; i++)
				{
					if (i < category.Length)
						entry.category[i] = (byte)category[i];
					else
						entry.category[i] = 0;

					if (i < name.Length)
						entry.name[i] = (byte)name[i];
					else
						entry.name[i] = 0;
				}
			}

			entry.value = value;

			SetDebugInfoByName(type, ref entry);
		}

		public static bool Visual2A03Mapping = false;
		public static bool Visual2C02Mapping = false;
	}
}
