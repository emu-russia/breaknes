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

// TBD: The entire implementation is NTSC PPU oriented. As things settle down, you need to add PPU selection settings and parameterize this module.

namespace PPUPlayer
{
    public partial class Form1 : Form
    {
        static int SamplesPerPCLK = 8;
        static int SamplesPerScan = 341 * SamplesPerPCLK;
        static int SamplesPerField = 262 * SamplesPerScan;

        float[] Scan = new float[SamplesPerScan];
        float[] Field = new float[SamplesPerField];

        int ScanSampleCounter = 0;
        int FieldSampleCounter = 0;

        static float BlackLevel = 1.3000f;
        static float IRE = 0.02f;
        static int PixelsPerSample = 1;
        static int ScaleY = 4;

        Color scanBgColor = Color.Gray;
        Color scanColor = Color.AliceBlue;

        void ProcessSample(float sample)
        {
            Scan[ScanSampleCounter] = sample;
            Field[FieldSampleCounter] = sample;

            ScanSampleCounter++;
            if (ScanSampleCounter >= SamplesPerScan)
            {
                if (pictureBoxScan.Visible)
                {
                    HumanizeScan();
                }
                ScanSampleCounter = 0;
            }

            FieldSampleCounter++;
            if (FieldSampleCounter >= SamplesPerField)
            {
                if (pictureBoxField.Visible)
                {
                    HumanizeField();
                }
                FieldSampleCounter = 0;
            }
        }

        private void testScanToolStripMenuItem_Click(object sender, EventArgs e)
        {
            float delta = 0.001f;

            for (int i = 0; i < SamplesPerScan; i++)
            {
                Scan[i] = BlackLevel + delta * i;
            }

            var PrevScanSampleCounter = ScanSampleCounter;
            ScanSampleCounter = SamplesPerScan;
            HumanizeScan();
            ScanSampleCounter = PrevScanSampleCounter;
        }

        private void testFieldToolStripMenuItem_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < SamplesPerField; i++)
            {
                Field[i] = BlackLevel;
            }

            var PrevFieldSampleCounter = FieldSampleCounter;
            FieldSampleCounter = SamplesPerField;
            HumanizeField();
            FieldSampleCounter = PrevFieldSampleCounter;
        }

        void HumanizeScan()
        {
            int w = SamplesPerScan * PixelsPerSample;
            int h = 250 * ScaleY;

            Bitmap pic = new(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            Graphics gr = Graphics.FromImage(pic);
            gr.Clear(scanBgColor);
            Pen pen = new(scanColor);
            Point prevPt = new(0, 0);

            for (int n=0; n<SamplesPerScan; n++)
            {
                float sample = Scan[n];
                float ires = (sample - BlackLevel) / IRE;

                Point pt = new(PixelsPerSample * n, h - ((int)ires + 200));
                gr.DrawLine(pen, prevPt, pt);
                prevPt = pt;
            }

            pictureBoxScan.Image = pic;
        }

        void HumanizeField()
        {
            // TBD
        }
    }
}
