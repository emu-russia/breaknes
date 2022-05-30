// A class for creating an empty cartridge on the NROM mapper. Used in Free Mode when the user has not selected a .nes file.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PPUPlayer
{
	internal class DummyNrom
	{
		byte[] nes;

		public DummyNrom(bool V_Mirroring)
		{
			byte[] nes_header = new byte[16] { 0x4e, 0x45, 0x53, 0x1a, 0x01, 0x01, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			byte[] prg = new byte[0x4000];
			byte[] chr = new byte[0x2000];

			if (V_Mirroring)
			{
				nes_header[6] |= 1;
			}

			nes = new byte[nes_header.Length + chr.Length + prg.Length];
			nes_header.CopyTo(nes, 0);
			prg.CopyTo(nes, nes_header.Length);
			chr.CopyTo(nes, nes_header.Length + prg.Length);
		}

		public byte [] GetNesImage()
		{
			return nes;
		}
	}
}
