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
		unsafe struct DebugInfoEntry
		{
			public fixed char category[32];
			public fixed char name[32];
			public UInt32 value;
		}

		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		unsafe struct MemDesciptor
		{
			public fixed char name[32];
			int size;
		};

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void CreateBoard(string boardName, string apu, string ppu, string p1);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void DestroyBoard();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void InsertCartridge(byte [] nesImage, int size);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void EjectCartridge();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void Sim(int numHalfCycles);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetDebugInfoEntryCount(DebugInfoType type);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetDebugInfo(DebugInfoType type, ref DebugInfoEntry entries);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern int GetMemLayout();

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void GetMemDescriptor(int descrID, ref MemDesciptor descr);

		[DllImport("BreaksCore.dll", CallingConvention = CallingConvention.Cdecl)]
		static extern void DumpMem(int descrID, byte [] ptr);
	}

}
