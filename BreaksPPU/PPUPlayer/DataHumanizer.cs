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

namespace PPUPlayer
{
    public class DataHumanizer
    {
        const string VRAM_NAME = "VRAM";
        const string CRAM_NAME = "Color RAM";
        const string OAM_NAME = "OAM";
        const string OAM2_NAME = "Temp OAM";
        const string CHR_ROM_NAME = "CHR-ROM";

        public Bitmap ConvertHexToImage(string descrName)
        {
            List<BreaksCore.MemDesciptor> mem = BreaksCore.GetMemoryLayout();

            // CHR-ROM (Pattern Table)

            if (descrName == CHR_ROM_NAME)
            {
                int descrID = 0;

                foreach (var descr in mem)
                {
                    if (descr.name == CHR_ROM_NAME)
                    {
                        byte[] data = new byte[descr.size];
                        BreaksCore.DumpMem(descrID, data);
                        return GetCHRImage(data);
                    }

                    descrID++;
                }
            }

            // VRAM (Name Table + Pattern Table)

            if (descrName == VRAM_NAME)
            {
                int descrVRAM = -1;
                int VRAMSize = 0;
                int descrCHR = -1;
                int CHRSize = 0;
                int descrID = 0;

                foreach (var descr in mem)
                {
                    if (descr.name == VRAM_NAME)
                    {
                        descrVRAM = descrID;
                        VRAMSize = descr.size;
                    }
                    if (descr.name == CHR_ROM_NAME)
                    {
                        descrCHR = descrID;
                        CHRSize = descr.size;
                    }
                    descrID++;
                }

                if (descrVRAM > 0 && descrCHR > 0)
                {
                    byte[] vram_data = new byte[VRAMSize];
                    BreaksCore.DumpMem(descrVRAM, vram_data);

                    byte[] chr_data = new byte[CHRSize];
                    BreaksCore.DumpMem(descrCHR, chr_data);

                    return GetVRAMImage(vram_data, chr_data);
                }
            }

            // OAM (OAM + Pattern Table)

            if (descrName == OAM_NAME)
            {
                int descrOAM = -1;
                int OAMSize = 0;
                int descrCHR = -1;
                int CHRSize = 0;
                int descrID = 0;

                foreach (var descr in mem)
                {
                    if (descr.name == OAM_NAME)
                    {
                        descrOAM = descrID;
                        OAMSize = descr.size;
                    }
                    if (descr.name == CHR_ROM_NAME)
                    {
                        descrCHR = descrID;
                        CHRSize = descr.size;
                    }
                    descrID++;
                }

                if (descrOAM > 0 && descrCHR > 0)
                {
                    byte[] oam_data = new byte[OAMSize];
                    BreaksCore.DumpMem(descrOAM, oam_data);

                    byte[] chr_data = new byte[CHRSize];
                    BreaksCore.DumpMem(descrCHR, chr_data);

                    return GetMainOAMImage(oam_data, chr_data);
                }
            }

            // Temp OAM (Temp OAM + Pattern Table)

            if (descrName == OAM2_NAME)
            {
                int descrOAM2 = -1;
                int OAM2Size = 0;
                int descrCHR = -1;
                int CHRSize = 0;
                int descrID = 0;

                foreach (var descr in mem)
                {
                    if (descr.name == OAM2_NAME)
                    {
                        descrOAM2 = descrID;
                        OAM2Size = descr.size;
                    }
                    if (descr.name == CHR_ROM_NAME)
                    {
                        descrCHR = descrID;
                        CHRSize = descr.size;
                    }
                    descrID++;
                }

                if (descrOAM2 > 0 && descrCHR > 0)
                {
                    byte[] oam2_data = new byte[OAM2Size];
                    BreaksCore.DumpMem(descrOAM2, oam2_data);

                    byte[] chr_data = new byte[CHRSize];
                    BreaksCore.DumpMem(descrCHR, chr_data);

                    return GetTempOAMImage(oam2_data, chr_data);
                }
            }

            // Color RAM (CRAM)

            if (descrName == CRAM_NAME)
            {
                int descrID = 0;

                foreach (var descr in mem)
                {
                    if (descr.name == CRAM_NAME)
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
                            int c;
                            Color p = Color.Transparent;
                            c = (((hi >> b) & 1) << 1) | ((lo >> b) & 1);
                            switch (c)
                            {
                                case 0:
                                    p = Color.FromArgb(0, 0, 0);
                                    break;
                                case 1:
                                    p = Color.FromArgb(0x3f, 0x3f, 0x3f);
                                    break;
                                case 2:
                                    p = Color.FromArgb(0x7f, 0x7f, 0x7f);
                                    break;
                                case 3:
                                    p = Color.FromArgb(0xff, 0xff, 0xff);
                                    break;
                            }
                            pic.SetPixel(8 * x + (7 - b), 8 * y + r, p);
                        }
                    }
                }
            }

            return pic;
        }

        Bitmap GetVRAMImage(byte [] nameTab, byte [] patternTab)
        {
            return null;
        }

        Bitmap GetMainOAMImage(byte[] oam, byte[] patternTab)
        {
            return null;
        }

        Bitmap GetTempOAMImage(byte[] oam2, byte[] patternTab)
        {
            return null;
        }

        Bitmap GetColorRAMImage(byte [] cram)
        {
            return null;
        }
    }
}
