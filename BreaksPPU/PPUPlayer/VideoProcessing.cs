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

// Note the peculiarity that the signal generation in the PPU starts immediately from the visible part (PICTURE). Therefore in the scans - HSync and Burst are on the right.

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

		// Picture output is set to unloaded video signal (Vpk/pk = 2.0)

		static float BlankLevel = 1.3000f;  // IRE = 0
		static float V_pk_pk = 2.0f;
		static float IRE = V_pk_pk / 100.0f;
		static int PixelsPerSample = 1;
		static int ScaleY = 5;
		static int PPUPicturePortion = 256 * SamplesPerPCLK;

		Color scanBgColor = Color.Gray;
		Color scanColor = Color.AliceBlue;
		Color pictureDelimiterColor = Color.Tomato;

		void ProcessSample(float sample)
		{
			Scan[ScanSampleCounter] = sample;
			Field[FieldSampleCounter] = sample;

			ScanSampleCounter++;
			if (ScanSampleCounter >= SamplesPerScan)
			{
				if (pictureBoxScan.Visible)
				{
					VisualizeScan();
				}
				ScanSampleCounter = 0;
			}

			FieldSampleCounter++;
			if (FieldSampleCounter >= SamplesPerField)
			{
				if (pictureBoxField.Visible)
				{
					VisualizeField();
				}
				FieldSampleCounter = 0;
			}
		}

		private void testScanToolStripMenuItem_Click(object sender, EventArgs e)
		{
			float delta = 0.001f;

			float level = BlankLevel;
			int subCounter = 0;

			for (int i = 0; i < SamplesPerScan; i++)
			{
				Scan[i] = subCounter < 4 ? BlankLevel + delta * i : BlankLevel - delta * i;

				subCounter++;
				if (subCounter >= 8)
				{
					subCounter = 0;
				}
			}

			var PrevScanSampleCounter = ScanSampleCounter;
			ScanSampleCounter = SamplesPerScan;
			VisualizeScan();
			ScanSampleCounter = PrevScanSampleCounter;
		}

		private void testFieldToolStripMenuItem_Click(object sender, EventArgs e)
		{
			for (int i = 0; i < SamplesPerField; i++)
			{
				Field[i] = BlankLevel;
			}

			var PrevFieldSampleCounter = FieldSampleCounter;
			FieldSampleCounter = SamplesPerField;
			VisualizeField();
			FieldSampleCounter = PrevFieldSampleCounter;
		}

		/// <summary>
		/// The appearance of the signal repeats the familiar pictures from Wikipedia. Upper level IRE = 200 (with reserve), 0 = Blank Level, IRE = -50 lower level.
		/// Therefore, the height of the picture is 250 IRE.
		/// When converting the sample to the IRE level, it is additionally shifted to make it look nice.
		/// </summary>
		void VisualizeScan()
		{
			int w = SamplesPerScan * PixelsPerSample;
			int h = 250 * ScaleY;

			Bitmap pic = new(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
			Graphics gr = Graphics.FromImage(pic);
			gr.Clear(scanBgColor);
			Pen pen = new(scanColor);
			Point prevPt = new(-SamplesPerScan, 0);

			for (int n=0; n<SamplesPerScan; n++)
			{
				float sample = Scan[n];
				float ires = ((sample - BlankLevel) / IRE) * ScaleY;

				Point pt = new(PixelsPerSample * n, h - ((int)ires + 200));
				gr.DrawLine(pen, prevPt, pt);
				prevPt = pt;
			}

			// Draw the separator for the visible part of the signal.
			gr.DrawLine(new(pictureDelimiterColor),
				new(PPUPicturePortion * PixelsPerSample, 0),
				new(PPUPicturePortion * PixelsPerSample, h - 1));

			pictureBoxScan.Image = pic;
		}

		void VisualizeField()
		{
			// TBD
		}

		void ResetVisualize()
		{
			ScanSampleCounter = 0;
			FieldSampleCounter = 0;
		}
	}
}
