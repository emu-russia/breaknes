// https://www.nesdev.org/wiki/NSF

using System.ComponentModel;

namespace NSFPlayer
{
	public class NSFHeader
	{
		[Description("denotes an NES sound format file")]
		public byte[] Signature { get; set; } = new byte[5];
		[Description("Version number $01 (or $02 for NSF2)")]
		public byte Version { get; set; }
		[Description("Total songs   (1=1 song, 2=2 songs, etc)")]
		public byte TotalSongs { get; set; }
		[Description("Starting song (1=1st song, 2=2nd song, etc)")]
		public byte StartingSong { get; set; }
		[Description("(lo, hi) load address of data ($8000-FFFF)")]
		public UInt16 LoadAddress { get; set; }
		[Description("(lo, hi) init address of data ($8000-FFFF)")]
		public UInt16 InitAddress { get; set; }
		[Description("(lo, hi) play address of data ($8000-FFFF)")]
		public UInt16 PlayAddress { get; set; }
		[Description("The name of the song, null terminated")]
		public string Name { get; set; } = "";
		[Description("The artist, if known, null terminated")]
		public string Artist { get; set; } = "";
		[Description("The copyright holder, null terminated")]
		public string Copyright { get; set; } = "";
		[Description("(lo, hi) Play speed, in 1/1000000th sec ticks, NTSC")]
		public UInt16 PlaySpeedNtsc { get; set; }
		[Description("Bankswitch init values")]
		public byte[] Bankswitch { get; set; } = new byte[8];
		[Description("(lo, hi) Play speed, in 1/1000000th sec ticks, PAL")]
		public UInt16 PlaySpeedPal { get; set; }
		[Description("PAL/NTSC bits; bit 0: if clear, this is an NTSC tune; bit 0: if set, this is a PAL tune; bit 1: if set, this is a dual PAL/NTSC tune; bits 2-7: reserved")]
		public byte PalNtscBits { get; set; }
		[Description("Extra Sound Chip Support")]
		public byte ExtraChips { get; set; }
		[Description("Reserved for NSF2")]
		public byte ReservedNSF2 { get; set; }
		[Description("24-bit length of contained program data")]
		public int DataLength { get; set; }
	}

	public class NSFSupport
	{
		private readonly NSFHeader head = new();
		private byte[] nsf_data = Array.Empty<byte>();
		private const int data_offset = 0x80;       // The music program/data follows

		private const byte rts = 0x60;
		private bool RDY2_Shadow = true;

		public NSFHeader GetHead()
		{
			return head;
		}

		public byte[] GetData()
		{
			return nsf_data;
		}

		public void LoadNSF(byte[] data)
		{
			LoadHeader(data);
			nsf_data = new byte[head.DataLength];
			Array.Copy(data, data_offset, nsf_data, 0, head.DataLength);
			CoreReady(false);
		}

		private void LoadHeader(byte[] data)
		{
			for (int i=0; i<5; i++)
			{
				head.Signature[i] = data[i];
			}

			head.Version = data[5];
			head.TotalSongs = data[6];
			head.StartingSong = data[7];
			head.LoadAddress = (UInt16)((data[9] << 8) | data[8]);
			head.InitAddress = (UInt16)((data[11] << 8) | data[10]);
			head.PlayAddress = (UInt16)((data[13] << 8) | data[12]);

			head.Name = "";
			for (int i = 0; i < 32; i++)
			{
				if (data[i + 0xe] == 0)
					break;
				head.Name += (char)data[i + 0xe];
			}

			head.Artist = "";
			for (int i = 0; i < 32; i++)
			{
				if (data[i + 0x2e] == 0)
					break;
				head.Artist += (char)data[i + 0x2e];
			}

			head.Copyright = "";
			for (int i = 0; i < 32; i++)
			{
				if (data[i + 0x4e] == 0)
					break;
				head.Copyright += (char)data[i + 0x4e];
			}

			head.PlaySpeedNtsc = (UInt16)((data[0x6f] << 8) | data[0x6e]);
			head.PlaySpeedPal = (UInt16)((data[0x79] << 8) | data[0x78]);

			for (int i = 0; i < 8; i++)
			{
				head.Bankswitch[i] = data[i + 0x70];
			}

			head.PalNtscBits = data[0x7a];
			head.ExtraChips = data[0x7b];
			head.ReservedNSF2 = data[0x7c];

			head.DataLength = (int)((data[0x7f] << 16) | (data[0x7e] << 8) | data[0x7d]);

			// If 0, all data until end of file is part of the program. If used, can be used to provide NSF2 metadata in a backward compatible way.
			if (head.DataLength == 0)
			{
				head.DataLength = data.Length - data_offset;
			}
		}

		/// <summary>
		/// Return the period for calling the PLAY procedure.
		/// </summary>
		/// <param name="PreferPal">If the NSF header specifies that you can use hybrid PAL/NTSC - specify explicitly that we want PAL</param>
		/// <returns></returns>
		public int GetPeriod(bool PreferPal)
		{
			bool pal;
			if ((head.PalNtscBits & 2) != 0)
			{
				pal = PreferPal;
			}
			else
			{
				pal = (head.PalNtscBits & 1) != 0;
			}
			return (pal ? head.PlaySpeedPal : head.PlaySpeedNtsc) / 100;
		}

		/// <summary>
		/// Start the 6502 core to execute from the specified address, prior to RTS instruction.
		/// </summary>
		/// <param name="address">PC address</param>
		/// <param name="a">A register value (optional)</param>
		/// <param name="x">X register value (optional)</param>
		/// <param name="y">Y register value (optional)</param>
		/// <param name="reset_apu_also">Also /RES=0 whole APU chip</param>
		public void ExecuteUntilRTS (UInt16 address, byte? a, byte? x, byte? y, bool reset_apu_also)
		{
			//if (IsCoreReady())
			//	return;

			NSFPlayerInterop.ResetAPU(address, reset_apu_also);
			// Set the A register for the desired song.
			if (a != null)
			{
				BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_CoreRegs, BreaksCore.CORE_REGS_CATEGORY, "A", (byte)a);
			}
			// Set the X register for PAL(1) or NTSC(0).
			if (x != null)
			{
				BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_CoreRegs, BreaksCore.CORE_REGS_CATEGORY, "X", (byte)x);
			}
			// While the NSF1 specification never guaranteed anything for Y on entry to INIT, for better forward compatibility with NSF2's non-returning INIT feature, it is recommended that the player set Y to 0
			if (y != null)
			{
				BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_CoreRegs, BreaksCore.CORE_REGS_CATEGORY, "Y", (byte)y);
			}
			CoreReady(true);
			Console.WriteLine("Exec: 0x" + address.ToString("X4"));
		}

		/// <summary>
		/// Check that 6502 is executed prior to the RTS instruction.
		/// </summary>
		public void SyncExec ()
		{
			bool sync = BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Board, BreaksCore.BOARD_CATEGORY, "SYNC") != 0;
			byte ir = (byte)BreaksCore.GetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_Core, BreaksCore.CORE_WIRES_CATEGORY, "IR");
			if (sync && (ir == rts) && RDY2_Shadow)
			{
				Console.WriteLine("Synced to RTS");
				CoreReady(false);
			}
		}

		/// <summary>
		/// The debug signal `RDY2`, which is present in all APU revisions, is used to start and stop.
		/// </summary>
		/// <param name="ready"></param>
		private void CoreReady (bool ready)
		{
			BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_APU, BreaksCore.APU_WIRES_CATEGORY, "RDY2", (UInt32)(ready ? 1 : 0));
			RDY2_Shadow = ready;
		}

		/// <summary>
		/// Used to check that the NSF procedure has finished its execution and the 6502 core is resting.
		/// </summary>
		/// <returns></returns>
		public bool IsCoreReady()
		{
			return RDY2_Shadow;
		}
	}
}
