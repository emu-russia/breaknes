// Interface for communicating with the native part.

using System.Text;
using System.Globalization;
using System.ComponentModel;
using System.Collections.Generic;
using System.Collections;

using System.Runtime.InteropServices;

namespace Breaknes
{
	public class BreaksCore
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


        [DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void CreateBoard(string boardName, string apu, string ppu, string p1);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DestroyBoard();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void InsertCartridge(byte [] nesImage, int size);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void EjectCartridge();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void Sim(int numHalfCycles);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetDebugInfoEntryCount(DebugInfoType type);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetDebugInfo(DebugInfoType type, IntPtr entries);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int GetMemLayout();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetMemDescriptor(int descrID, ref MemDesciptorRaw descr);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern void DumpMem(int descrID, byte [] ptr);


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
					for (int n=0; n<32; n++)
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

	}

}
