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
using System.IO;

// TBD: The entire implementation is NTSC PPU oriented. As things settle down, you need to add PPU selection settings and parameterize this module.

// Note the peculiarity that the signal generation in the PPU starts immediately from the visible part (PICTURE). Therefore in the scans - HSync and Burst are on the right.

namespace PPUPlayer
{
	public partial class Form1 : Form
	{
		static int SamplesPerPCLK = 8;
		static int PixelsPerScan = 341;
		static int ScansPerField = 262;
		static int SamplesPerScan = PixelsPerScan * SamplesPerPCLK;
		static int SamplesPerField = ScansPerField * SamplesPerScan;

		float[] Scan = new float[SamplesPerScan];
		float[] Field = new float[SamplesPerField];
		float[] LastScan = new float[SamplesPerScan];
		float[] LastField = new float[SamplesPerField];

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

				for (int i = 0; i < SamplesPerScan; i++)
				{
					LastScan[i] = Scan[i];
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

				for (int i = 0; i < SamplesPerField; i++)
				{
					LastField[i] = Field[i];
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
			int w = PixelsPerScan;
			int h = ScansPerField;

			Bitmap pic = new(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
			Graphics gr = Graphics.FromImage(pic);

			int samplePtr = 0;

			for (int y = 0; y < ScansPerField; y++)
			{
				for (int x =0; x< PixelsPerScan; x++)
				{
					// Integrate sample batch

					float avg = 0.0f;
					for (int i = 0; i < SamplesPerPCLK; i++)
					{
						avg += Field[samplePtr++];
					}
					avg /= SamplesPerPCLK;

					// Normalization

					float ire = ((avg - BlankLevel) / IRE);
					int gs = (int)Math.Max(0.0f, Math.Min(255.0f, ire));

					gr.FillRectangle(new SolidBrush(Color.FromArgb(gs, gs, gs)), x, y, 1, 1);
				}
			}

			pictureBoxField.Image = pic;
		}

		void ResetVisualize()
		{
			ScanSampleCounter = 0;
			FieldSampleCounter = 0;
		}

		private void SaveFloatArray(string filename, float [] data)
		{
			// https://stackoverflow.com/questions/30628833/write-float-array-into-a-binary-file-c-sharp

			using (FileStream file = File.Create(filename))
			{
				using (BinaryWriter writer = new BinaryWriter(file))
				{
					foreach (float value in data)
					{
						writer.Write(value);
					}
				}
			}
		}

		private void LoadFloatArray(string filename, float [] data, int maxSize)
		{
			int size = Math.Min(data.Length, maxSize);

			using (FileStream file = File.OpenRead(filename))
			{
				using (BinaryReader reader = new BinaryReader(file))
				{
					for (int i=0; i<size; i++)
					{
						data[i] = reader.ReadSingle();
					}
				}
			}
		}

		private void saveToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DialogResult res = saveFileDialog1.ShowDialog();
			if (res == DialogResult.OK)
			{
				SaveFloatArray(saveFileDialog1.FileName, LastField);
			}
		}

		private void saveTheLastScanToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DialogResult res = saveFileDialog1.ShowDialog();
			if (res == DialogResult.OK)
			{
				SaveFloatArray(saveFileDialog1.FileName, LastScan);
			}
		}

		private void visualizeFieldFromDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DialogResult res = openFileDialog1.ShowDialog();
			if (res == DialogResult.OK)
			{
				for (int i = 0; i < SamplesPerField; i++)
				{
					LastField[i] = BlankLevel;
				}

				LoadFloatArray(openFileDialog1.FileName, LastField, SamplesPerField);

				for (int i = 0; i < SamplesPerField; i++)
				{
					Field[i] = LastField[i];
				}

				var PrevFieldSampleCounter = FieldSampleCounter;
				FieldSampleCounter = SamplesPerField;
				VisualizeField();
				FieldSampleCounter = PrevFieldSampleCounter;
			}
		}

		private void visualizeScanFromDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DialogResult res = openFileDialog1.ShowDialog();
			if (res == DialogResult.OK)
			{
				for (int i = 0; i < SamplesPerScan; i++)
				{
					LastScan[i] = BlankLevel;
				}

				LoadFloatArray(openFileDialog1.FileName, LastScan, SamplesPerScan);

				for (int i = 0; i < SamplesPerScan; i++)
				{
					Scan[i] = LastScan[i];
				}

				var PrevScanSampleCounter = ScanSampleCounter;
				ScanSampleCounter = SamplesPerScan;
				VisualizeScan();
				ScanSampleCounter = PrevScanSampleCounter;
			}
		}
	}
}
