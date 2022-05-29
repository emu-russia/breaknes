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
using System.Numerics;

// TBD: A ScanRing buffer is used to store the sample stream, which can hold up to 3 scans.
// When samples are added, the drop from Blank level to Sync is counted. If there are 2 such drops, it means that there are enough samples in the circular buffer to output one line.

namespace PPUPlayer
{
	public partial class FormMain : Form
	{
		// TBD: The VideoSignalFeatures structure from PPUSim will be used instead.

		static int SamplesPerPCLK = 8;
		static int PixelsPerScan = 341;
		static int ScansPerField = 262;
		static int SamplesPerScan = PixelsPerScan * SamplesPerPCLK;
		static int SamplesPerField = ScansPerField * SamplesPerScan;
		// Picture output is set to unloaded video signal (Vpk/pk = 2.0)
		static float BlankLevel = 1.3000f;  // IRE = 0
		static float V_pk_pk = 2.0f;

		// TBD: A circular buffer will be used (ScanRing)

		float[] Scan = new float[SamplesPerScan];
		float[] Field = new float[SamplesPerField];
		float[] LastScan = new float[SamplesPerScan];
		float[] LastField = new float[SamplesPerField];
		int ScanSampleCounter = 0;
		int FieldSampleCounter = 0;

		// Visualization settings

		static int PixelsPerSample = 1;     // Discrete pixels (Bitmap)
		static int ScaleY = 5;
		static int PPUPicturePortion = 256 * SamplesPerPCLK;
		Color scanBgColor = Color.Gray;
		Color scanColor = Color.AliceBlue;
		Color pictureDelimiterColor = Color.Tomato;

		Bitmap? scan_pic = null;
		Bitmap? field_pic = null;

		bool RawMode = false;

		void ProcessSample(PPUPlayerInterop.VideoOutSample sample)
		{
			Scan[ScanSampleCounter] = sample.composite;
			Field[FieldSampleCounter] = sample.composite;

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
			float IRE = V_pk_pk / 100.0f;

			//if (scan_pic == null)
			{
				scan_pic = new(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
			}

			Bitmap pic = scan_pic;
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

			pictureBoxScan.Image = scan_pic;
			gr.Dispose();
		}

		void VisualizeField()
		{
			int w = PixelsPerScan;
			int h = ScansPerField;
			float WhiteLevel = BlankLevel + V_pk_pk;

			float[] packet = new float[SamplesPerPCLK];

			//if (field_pic == null)
			{
				field_pic = new(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
			}

			Bitmap pic = field_pic;
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
						packet[i] = Field[samplePtr] - BlankLevel;
						avg += packet[i];
						samplePtr++;
					}
					avg /= SamplesPerPCLK;

					float lum = Clamp(avg / WhiteLevel, 0, 1);

					// Saturation/Hue

					var c = Goertzel(packet, SamplesPerPCLK, 1.0f);

					float hue = Clamp((float)c.Phase, 0, 1);
					float sat = Clamp((float)c.Magnitude, 0, 1);

					// HSL -> RGB

					gr.FillRectangle(new SolidBrush(hsl2rgb(hue, sat, lum)), x, y, 1, 1);
				}
			}

			pictureBoxField.Image = field_pic;
			gr.Dispose();
		}

		// https://dsp.stackexchange.com/questions/23944/using-goertzel-to-find-phase-shift

		Complex Goertzel (float[] data, int size, float omega)
		{
			float sine, cosine, coeff, q1 = 0, q2 = 0;

			sine = (float)Math.Sin(omega);
			cosine = (float)Math.Cos(omega);
			coeff = 2.0f * cosine;

			for (int t = 0; t < size; t++)
			{
				float q0 = coeff * q1 - q2 + data[t];
				q2 = q1;
				q1 = q0;
			}

			float real = (q1 - q2 * cosine);
			float imag = (q2 * sine);

			return new Complex(real, imag);
		}

		// https://gist.github.com/ciembor/1494530

		float hue2rgb(float p, float q, float t)
		{
			if (t < 0)
				t += 1;
			if (t > 1)
				t -= 1;
			if (t < 1.0f / 6)
				return p + (q - p) * 6 * t;
			if (t < 1.0f / 2)
				return q;
			if (t < 2.0f / 3)
				return p + (q - p) * (2.0f / 3 - t) * 6;

			return p;
		}

		Color hsl2rgb(float h, float s, float l)
		{
			Color result;

			if (s == 0)
			{
				// achromatic
				int z = (int)Clamp(l * 255, 0, 255);
				result = Color.FromArgb (z, z, z);
			}
			else
			{
				float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
				float p = 2 * l - q;
				float r = Clamp(hue2rgb(p, q, h + 1.0f / 3) * 255, 0, 255);
				float g = Clamp(hue2rgb(p, q, h) * 255, 0, 255);
				float b = Clamp(hue2rgb(p, q, h - 1.0f / 3) * 255, 0, 255);
				result = Color.FromArgb((int)r, (int)g, (int)b);
			}

			return result;
		}

		float Clamp(float val, float min, float max)
		{
			return Math.Min(max, Math.Max(val, min));
		}

		void ResetVisualize(bool RAWMode)
		{
			RawMode = RAWMode;
			PPUPlayerInterop.SetRAWColorMode(RAWMode);

			ScanSampleCounter = 0;
			FieldSampleCounter = 0;
			scan_pic = null;
			field_pic = null;
			GC.Collect();
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
