using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.InteropServices;

namespace PPUPlayer
{
	internal class PPUPlayerInterop
	{

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
		public static extern int GetPCLKCounter();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int InsertCartridge(byte[] nesImage, int nesImageSize);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void EjectCartridge();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SampleVideoSignal(out float sample);

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

		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		unsafe struct VideoSignalFeaturesRaw
		{
			public int SamplesPerPCLK;
			public int PixelsPerScan;       // Excluding Dot Crawl
			public int ScansPerField;
			public int Composite;           // 1: Composite, 0: RGB
			public float BlankLevel;       // IRE = 0
			public float V_pk_pk;
		}

		public struct VideoSignalFeatures
		{
			public int SamplesPerPCLK;
			public int PixelsPerScan;       // Excluding Dot Crawl
			public int ScansPerField;
			public int Composite;           // 1: Composite, 0: RGB
			public float BlankLevel;       // IRE = 0
			public float V_pk_pk;
		}

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void GetSignalFeatures(IntPtr features);

		public static VideoSignalFeatures GetSignalFeatures()
		{
			VideoSignalFeatures features = new();

			IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf<VideoSignalFeaturesRaw>());
			if (ptr == IntPtr.Zero)
			{
				throw new Exception("AllocHGlobal failed!");
			}

			GetSignalFeatures(ptr);

#pragma warning disable CS8605 // Unboxing a possibly null value.
			VideoSignalFeaturesRaw raw = (VideoSignalFeaturesRaw)Marshal.PtrToStructure(ptr, typeof(VideoSignalFeaturesRaw));
#pragma warning restore CS8605 // Unboxing a possibly null value.

			features.SamplesPerPCLK = raw.SamplesPerPCLK;
			features.PixelsPerScan = raw.PixelsPerScan;
			features.ScansPerField = raw.ScansPerField;
			features.Composite = raw.Composite;
			features.BlankLevel = raw.BlankLevel;
			features.V_pk_pk = raw.V_pk_pk;

			Marshal.FreeHGlobal(ptr);

			return features;
		}
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
		static extern int GetMemLayout();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetMemDescriptor(int descrID, IntPtr descr);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DumpMem(int descrID, [In, Out] [MarshalAs(UnmanagedType.LPArray)] byte[] ptr);


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
	}

}
