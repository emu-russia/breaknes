using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.InteropServices;

namespace NSFPlayer
{
	public class NSFPlayerInterop
	{
		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void CreateBoard(string boardName, string apu, string ppu, string p1);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DestroyBoard();

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void Step();

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void ResetAPU(UInt16 addr, bool reset_apu_also);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool APUInResetState();

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern long GetACLKCounter();
		
		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern long GetPHICounter();

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SampleAudioSignal(out float sample);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void LoadNSFData([In, Out][MarshalAs(UnmanagedType.LPArray)] byte[] data, int data_size, UInt16 load_address);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void EnableNSFBanking(bool enable);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void LoadRegDump([In, Out][MarshalAs(UnmanagedType.LPArray)] byte[] data, int data_size);

		[StructLayout(LayoutKind.Explicit)]
		public struct AudioSignalFeatures
		{
			[FieldOffset(0)]
			public int SampleRate;      // The sampling frequency of the audio signal (samples per "virtual" second). The audio is actually sampled every PHI (core clock) cycle.
			[FieldOffset(4)]
			public int AclkPerSecond;   // Number of ACLKs per virtual second. Can be used by NSFPlayer to get frequency of PLAY = AclkPerSecond / period
			[FieldOffset(8)]
			public float AUXA_LowLevel;		// Lower signal level for AUX_A (mV)
			[FieldOffset(12)]
			public float AUXA_HighLevel;	// Upper signal level for AUX_A (mV)
			[FieldOffset(16)]
			public float AUXB_LowLevel;		// Lower signal level for AUX_B (mV)
			[FieldOffset(20)]
			public float AUXB_HighLevel;    // Upper signal level for AUX_B (mV)
		}

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void GetSignalFeatures(out AudioSignalFeatures features);
	}


	internal class BreaksCore
	{
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

		public const string BANKED_SRAM_NAME = "BankedSRAM";
		public const string WRAM_NAME = "WRAM";
		public const string CORE_WIRES_CATEGORY = "Core Wires";
		public const string CORE_REGS_CATEGORY = "Core Regs";
		public const string APU_WIRES_CATEGORY = "APU Wires";
		public const string APU_REGS_CATEGORY = "APU Regs";
		public const string BOARD_CATEGORY = "NSFPlayer Board";

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


		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetDebugInfoEntryCount(DebugInfoType type);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetDebugInfo(DebugInfoType type, IntPtr entries);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetDebugInfoByName(DebugInfoType type, ref DebugInfoEntryRaw entry);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int SetDebugInfoByName(DebugInfoType type, ref DebugInfoEntryRaw entry);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetMemLayout();

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetMemDescriptor(int descrID, IntPtr descr);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DumpMem(int descrID, [In, Out] [MarshalAs(UnmanagedType.LPArray)] byte[] ptr);

		[DllImport("NSFPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
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
	}
}
