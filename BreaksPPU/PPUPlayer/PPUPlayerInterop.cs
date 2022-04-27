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
		public static extern void InsertCartridge(byte[] nesImage, int nesImageSize);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void EjectCartridge();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void SampleVideoSignal(out float sample);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetHCounter();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetVCounter();
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
			public UInt32 value;
		}

		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		unsafe struct MemDesciptorRaw
		{
			public fixed byte name[32];
			int size;
		};

		public class DebugInfoEntry
		{
			public string category = "";
			public string name = "";
			public UInt32 value = 0;
		}


		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetDebugInfoEntryCount(DebugInfoType type);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetDebugInfo(DebugInfoType type, IntPtr entries);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetMemLayout();

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetMemDescriptor(int descrID, ref MemDesciptorRaw descr);

		[DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DumpMem(int descrID, byte[] ptr);


		public static List<DebugInfoEntry> GetDebugInfo(DebugInfoType type)
		{
			List<DebugInfoEntry> list = new List<DebugInfoEntry>();

			var count = BreaksCore.GetDebugInfoEntryCount(type);

			//Console.WriteLine("Size of entry: " + Marshal.SizeOf<BreaksCore.DebugInfoEntryRaw>().ToString());

			IntPtr ptr = Marshal.AllocHGlobal(count * Marshal.SizeOf<BreaksCore.DebugInfoEntryRaw>());

			BreaksCore.GetDebugInfo(type, ptr);

			IntPtr p = ptr;

			for (int i = 0; i < count; ++i)
			{
				BreaksCore.DebugInfoEntryRaw raw = (BreaksCore.DebugInfoEntryRaw)Marshal.PtrToStructure(p, typeof(BreaksCore.DebugInfoEntryRaw));

				DebugInfoEntry entry = new DebugInfoEntry();

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
			List<DebugInfoEntry> list = new List<DebugInfoEntry>();

			DebugInfoEntry testEntry = new DebugInfoEntry();

			testEntry.category = "Test Category";
			testEntry.name = "Test Entry";
			testEntry.value = 123;

			list.Add(testEntry);

			return list;
		}
	}

}
