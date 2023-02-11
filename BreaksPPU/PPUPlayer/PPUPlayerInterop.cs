using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.InteropServices;

namespace PPUPlayer
{
	public class PPUPlayerInterop
	{

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
			public UInt16 raw;		// sBGRLLCCCC (Sync || Tint Blue || Tint Green || Tint Red || Luma[2] || Chroma[4])
		}

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void CreateBoard(string boardName, string apu, string ppu, string p1);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DestroyBoard();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void Step();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void CPUWrite(int ppuReg, byte val);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void CPURead(int ppuReg);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern long GetPCLKCounter();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int InsertCartridge(byte[] nesImage, int nesImageSize);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void EjectCartridge();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SampleVideoSignal(out VideoOutSample sample);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetHCounter();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetVCounter();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void ResetPPU();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int PPUInResetState();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
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
			public int BackPorchSize;		// BackPorch size in pixels.
			[FieldOffset(16)]
			public int Composite;           // 1: Composite, 0: RGB
			[FieldOffset(20)]
			public float BurstLevel;		// IRE = 0
			[FieldOffset(24)]
			public float WhiteLevel;        // IRE = 100
			[FieldOffset(28)]
			public float SyncLevel;
			[FieldOffset(32)]
			public int PhaseAlteration;		// 1: PAL
		}

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void GetSignalFeatures(out VideoSignalFeatures features);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void ConvertRAWToRGB(UInt16 raw, out byte r, out byte g, out byte b);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
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

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SetOamDecayBehavior(OAMDecayBehavior behavior);
		
		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SetNoiseLevel(float volts);
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


		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetDebugInfoEntryCount(DebugInfoType type);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetDebugInfo(DebugInfoType type, IntPtr entries);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetDebugInfoByName(DebugInfoType type, ref DebugInfoEntryRaw entry);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int SetDebugInfoByName(DebugInfoType type, ref DebugInfoEntryRaw entry);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetMemLayout();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetMemDescriptor(int descrID, IntPtr descr);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DumpMem(int descrID, [In, Out] [MarshalAs(UnmanagedType.LPArray)] byte[] ptr);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
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

		public static UInt32 GetDebugInfoByName(DebugInfoType type, string category, string name)
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
