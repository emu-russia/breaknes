// Module for converting PPU Hex dumps to human form.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections;

using System.Drawing.Drawing2D;
using SharpTools;

namespace PPUPlayer
{
	public class DataHumanizer
	{
		bool color_output = true;

		public Bitmap ConvertHexToImage(string descrName)
		{
			List<BreaksCore.MemDesciptor> mem = BreaksCore.GetMemoryLayout();

			// CHR-ROM (Pattern Table)

			if (descrName == BreaksCore.CHR_ROM_NAME)
			{
				int descrID = 0;

				foreach (var descr in mem)
				{
					if (descr.name == BreaksCore.CHR_ROM_NAME)
					{
						byte[] data = new byte[descr.size];
						BreaksCore.DumpMem(descrID, data);
						return GetCHRImage(data);
					}

					descrID++;
				}
			}

			// VRAM (Name Table + Pattern Table + CRAM)

			if (descrName == BreaksCore.VRAM_NAME)
			{
				int descrVRAM = -1;
				int VRAMSize = 0;
				int descrCHR = -1;
				int CHRSize = 0;
				int descrCRAM = -1;
				int CRAMSize = 0;
				int descrID = 0;

				foreach (var descr in mem)
				{
					if (descr.name == BreaksCore.VRAM_NAME)
					{
						descrVRAM = descrID;
						VRAMSize = descr.size;
					}
					if (descr.name == BreaksCore.CHR_ROM_NAME)
					{
						descrCHR = descrID;
						CHRSize = descr.size;
					}
					if (descr.name == BreaksCore.CRAM_NAME)
					{
						descrCRAM = descrID;
						CRAMSize = descr.size;
					}
					descrID++;
				}

				if (descrVRAM >= 0 && descrCHR >= 0 && descrCRAM >= 0)
				{
					byte[] vram_data = new byte[VRAMSize];
					BreaksCore.DumpMem(descrVRAM, vram_data);

					byte[] chr_data = new byte[CHRSize];
					BreaksCore.DumpMem(descrCHR, chr_data);

					byte[] cram_data = new byte[CRAMSize];
					BreaksCore.DumpMem(descrCRAM, cram_data);

					return GetVRAMImage(vram_data, chr_data, cram_data);
				}
			}

			// OAM (OAM + Pattern Table + CRAM)

			if (descrName == BreaksCore.OAM_NAME)
			{
				int descrOAM = -1;
				int OAMSize = 0;
				int descrCHR = -1;
				int CHRSize = 0;
				int descrCRAM = -1;
				int CRAMSize = 0;
				int descrID = 0;

				foreach (var descr in mem)
				{
					if (descr.name == BreaksCore.OAM_NAME)
					{
						descrOAM = descrID;
						OAMSize = descr.size;
					}
					if (descr.name == BreaksCore.CHR_ROM_NAME)
					{
						descrCHR = descrID;
						CHRSize = descr.size;
					}
					if (descr.name == BreaksCore.CRAM_NAME)
					{
						descrCRAM = descrID;
						CRAMSize = descr.size;
					}
					descrID++;
				}

				if (descrOAM >= 0 && descrCHR >= 0 && descrCRAM >= 0)
				{
					byte[] oam_data = new byte[OAMSize];
					BreaksCore.DumpMem(descrOAM, oam_data);

					byte[] chr_data = new byte[CHRSize];
					BreaksCore.DumpMem(descrCHR, chr_data);

					byte[] cram_data = new byte[CRAMSize];
					BreaksCore.DumpMem(descrCRAM, cram_data);

					return GetMainOAMImage(oam_data, chr_data, cram_data);
				}
			}

			// Temp OAM (Temp OAM + Pattern Table + CRAM)

			if (descrName == BreaksCore.OAM2_NAME)
			{
				int descrOAM2 = -1;
				int OAM2Size = 0;
				int descrCHR = -1;
				int CHRSize = 0;
				int descrCRAM = -1;
				int CRAMSize = 0;
				int descrID = 0;

				foreach (var descr in mem)
				{
					if (descr.name == BreaksCore.OAM2_NAME)
					{
						descrOAM2 = descrID;
						OAM2Size = descr.size;
					}
					if (descr.name == BreaksCore.CHR_ROM_NAME)
					{
						descrCHR = descrID;
						CHRSize = descr.size;
					}
					if (descr.name == BreaksCore.CRAM_NAME)
					{
						descrCRAM = descrID;
						CRAMSize = descr.size;
					}
					descrID++;
				}

				if (descrOAM2 >= 0 && descrCHR >= 0 && descrCRAM >= 0)
				{
					byte[] oam2_data = new byte[OAM2Size];
					BreaksCore.DumpMem(descrOAM2, oam2_data);

					byte[] chr_data = new byte[CHRSize];
					BreaksCore.DumpMem(descrCHR, chr_data);

					byte[] cram_data = new byte[CRAMSize];
					BreaksCore.DumpMem(descrCRAM, cram_data);

					return GetTempOAMImage(oam2_data, chr_data, cram_data);
				}
			}

			// Color RAM (CRAM)

			if (descrName == BreaksCore.CRAM_NAME)
			{
				int descrID = 0;

				foreach (var descr in mem)
				{
					if (descr.name == BreaksCore.CRAM_NAME)
					{
						byte[] data = new byte[descr.size];
						BreaksCore.DumpMem(descrID, data);
						return GetColorRAMImage(data);
					}

					descrID++;
				}
			}

			return null;
		}

		Bitmap GetCHRImage(byte [] chr_data)
		{
			int w = 8 * 16 * 2;
			int h = 8 * 16;

			if (chr_data.Length == 0)
				return new Bitmap(1, 1, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

			// chr2bmp - CHR ROM convertor. v.1.0
			// Created (C) org /2002/ kvzorganic@mail.ru
			// :-P

			Bitmap pic = new Bitmap(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

			// 24-bit image
			for (int y = 0; y < 16; y++)
			{
				for (int r = 0; r < 8; r++)     // ROW
				{
					for (int x = 0; x < 32; x++)
					{
						byte lo, hi;

						// setup pattern address
						int pat = 0;
						if (x >= 16)
							pat += 4096;
						pat = pat + 16 * 16 * y;
						if (x >= 16)
							pat = pat + 16 * (x - 16);
						else
							pat = pat + 16 * x;

						lo = chr_data[pat + r];
						hi = chr_data[pat + r + 8];
						for (int b = 0; b < 8; b++)
						{
							int c = ((((hi >> b) & 1) << 1) | ((lo >> b) & 1)) & 3;
							Color p = Color.FromArgb(Color2ToIntensity(c), Color2ToIntensity(c), Color2ToIntensity(c));
							pic.SetPixel(8 * x + (7 - b), 8 * y + r, p);
						}
					}
				}
			}

			return pic;
		}

		byte Color2ToIntensity(int c)
		{
			if (c == 0)
			{
				return 0;
			}
			else if (c == 3)
			{
				return 0xff;
			}

			return (byte)((c * 0x40) - 1);
		}

		byte Color4ToIntensity(int c)
		{
			if (c == 0)
			{
				return 0;
			}
			else if (c == 15)
			{
				return 0xff;
			}

			return (byte)((c * 0x10) - 1);
		}

		int TwizzleATByte(int offs, byte[] nameTab, int tile_h, int tile_v)
		{
			byte at = nameTab[offs + 0x3c0 + 8 * (tile_v / 4) + (tile_h / 4)];
			if ((tile_v & 3) > 1) at >>= 4;
			return ((tile_h & 3) > 1) ? at >> 2 : at & 3;
		}

		void RenderNameTab(int offs, byte[] nameTab, byte[] patternTab, Bitmap pic, int delta_x, int delta_y, byte[] cram)
		{
			int tile_size = 8;
			int w = 32 * tile_size;
			int h = 30 * tile_size;

			byte[,] nes_pal = GenPalette();

			int tile_num = 0;

			for (int y = 0; y < h; y += tile_size)
			{
				for (int x = 0; x < w; x += tile_size)
				{
					byte tile_id = nameTab[offs + tile_num];

					for (int row=0; row<tile_size; row++)
					{
						int par = 16 * tile_id + 0x1000;
						byte ta = patternTab[par + row];
						byte tb = patternTab[par + row + 8];

						for (int col=0; col<tile_size; col++)
						{
							int c = ((((tb >> col) & 1) << 1) | ((ta >> col) & 1)) & 3;
							var at = TwizzleATByte(offs, nameTab, x / tile_size, y / tile_size);
							c |= ((at & 3) << 2);
							c &= 0xf;
							Color p;
							if (color_output)
							{
								int n = cram[c];
								p = Color.FromArgb(nes_pal[n, 0], nes_pal[n, 1], nes_pal[n, 2]);
							}
							else
							{
								p = Color.FromArgb(Color4ToIntensity(c), Color4ToIntensity(c), Color4ToIntensity(c));
							}
							pic.SetPixel(delta_x + x + (7 - col), delta_y + y + row, p);
						}
					}

					tile_num++;
				}
			}
		}

		Bitmap GetVRAMImage(byte [] nameTab, byte [] patternTab, byte[] cram)
		{
			int tile_size = 8;
			int w = 32 * tile_size;
			int h = 30 * tile_size;

			if (patternTab.Length == 0)
				return new Bitmap(1, 1, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

			Bitmap pic = new Bitmap(w * 2, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

			RenderNameTab(0, nameTab, patternTab, pic, 0, 0, cram);
			RenderNameTab(0x400, nameTab, patternTab, pic, w, 0, cram);

			return pic;
		}

		void RenderObject(byte[] patternTab, Bitmap pic, int delta_x, int delta_y, byte tile_id, int bank, byte attr, bool visible, byte[] cram)
		{
			bool use_colors = color_output;

			if (cram.Length == 0)
				use_colors = false;

			int tile_size = 8;

			bool flip_v = (attr & 0x80) != 0;
			bool flip_h = (attr & 0x40) != 0;
			int pal = attr & 3;

			byte[,] nes_pal = GenPalette();

			for (int row = 0; row < tile_size; row++)
			{
				for (int col = 0; col < tile_size; col++)
				{
					pic.SetPixel(delta_x + col, delta_y + row, Color.Firebrick);
				}
			}

			for (int row = 0; row < tile_size; row++)
			{
				int par = 16 * tile_id + bank * 0x1000;
				byte ta = patternTab[par + row];
				byte tb = patternTab[par + row + 8];

				for (int col = 0; col < tile_size; col++)
				{
					int c = ((((tb >> col) & 1) << 1) | ((ta >> col) & 1)) & 3;
					c |= ((pal & 3) << 2);
					c &= 0xf;
					Color p;
					if (use_colors)
					{
						int n = cram[0x10 + c];
						p = Color.FromArgb(visible ? 0xff : 0x80, nes_pal[n, 0], nes_pal[n, 1], nes_pal[n, 2]);
					}
					else
					{
						p = Color.FromArgb(visible ? 0xff : 0x80, Color2ToIntensity(c & 3), Color2ToIntensity(c & 3), Color2ToIntensity(c & 3));
					}
					pic.SetPixel(delta_x + (flip_h ? col : (7 - col)), delta_y + (flip_v ? (7 - row) : row), p);
				}
			}
		}

		Bitmap GetMainOAMImage(byte[] oam, byte[] patternTab, byte[] cram)
		{
			int tile_size = 8;
			int w = 8 * tile_size;
			int h = 8 * tile_size;

			if (patternTab.Length == 0)
				return new Bitmap(1, 1);

			Bitmap pic = new Bitmap(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

			int obj_num = 0;

			for (int y=0; y<h; y+=tile_size)
			{
				for (int x=0; x<w; x+=tile_size)
				{
					byte obj_y = oam[4 * obj_num + 0];
					byte tile_idx = oam[4 * obj_num + 1];
					byte attr = oam[4 * obj_num + 2];
					byte obj_x = oam[4 * obj_num + 3];

					byte tile_id = (byte)(tile_idx & ~1);
					int bank = tile_idx & 1;
					bool visible = !(obj_y >= 0xef || obj_x >= 0xf9);

					RenderObject(patternTab, pic, x, y, tile_id, bank, attr, visible, cram);

					obj_num++;
				}
			}

			return pic;
		}

		Bitmap GetTempOAMImage(byte[] oam2, byte[] patternTab, byte[] cram)
		{
			int tile_size = 8;
			int w = 8 * tile_size;
			int h = 1 * tile_size;

			if (patternTab.Length == 0)
				return new Bitmap(1, 1);

			Bitmap pic = new Bitmap(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

			int obj_num = 0;

			for (int y = 0; y < h; y += tile_size)
			{
				for (int x = 0; x < w; x += tile_size)
				{
					byte obj_y = oam2[4 * obj_num + 0];
					byte tile_idx = oam2[4 * obj_num + 1];
					byte attr = oam2[4 * obj_num + 2];
					byte obj_x = oam2[4 * obj_num + 3];

					byte tile_id = (byte)(tile_idx & ~1);
					int bank = tile_idx & 1;
					bool visible = !(obj_y >= 0xef || obj_x >= 0xf9);

					RenderObject(patternTab, pic, x, y, tile_id, bank, attr, visible, cram);

					obj_num++;
				}
			}

			return pic;
		}

		/// <summary>
		/// Use PPUSim to generate an RGB palette based on "raw" colors.
		/// </summary>
		/// <returns></returns>
		byte[,] GenPalette()
		{
			byte[,] pal = new byte[64, 3];

			for (UInt16 raw=0; raw<64; raw++)
			{
				byte r, g, b;
				BreaksCore.ConvertRAWToRGB(raw, out r, out g, out b);
				pal[raw, 0] = r;
				pal[raw, 1] = g;
				pal[raw, 2] = b;
			}

			return pal;
		}

		Bitmap GetColorRAMImage(byte [] cram)
		{
			int blockSize = 16;
			int w = blockSize * 16;
			int h = blockSize * 2;

			byte[,] nes_pal = GenPalette();

			Bitmap pic = new Bitmap(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

			// BG

			int y = 0;

			for (int x = 0; x < 16; x++)
			{
				for (int row = 0; row < blockSize; row++)
				{
					var n = cram[x];
					Color p = Color.FromArgb(nes_pal[n, 0], nes_pal[n, 1], nes_pal[n, 2]);
					for (int col = 0; col < blockSize; col++)
					{
						pic.SetPixel(blockSize * x + col, blockSize * y + row, p);
					}
				}
			}

			// OBJ

			y = 1;

			for (int x = 0; x < 16; x++)
			{
				for (int row = 0; row < blockSize; row++)
				{
					var n = cram[x + 16];
					Color p = Color.FromArgb(nes_pal[n, 0], nes_pal[n, 1], nes_pal[n, 2]);
					for (int col = 0; col < blockSize; col++)
					{
						pic.SetPixel(blockSize * x + col, blockSize * y + row, p);
					}
				}
			}

			return pic;
		}

		public void SetColorDebugOutput(bool enable)
		{
			color_output = enable;
		}
	}
}
