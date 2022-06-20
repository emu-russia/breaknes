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

// Samples are stored in ScanBuffer in raw form. After processing, the Scan enters the Field in a ready for output form (RGB).

namespace PPUPlayer
{
	public partial class FormMain : Form
	{
		PPUPlayerInterop.VideoSignalFeatures ppu_features;
		int SamplesPerScan;
		bool RawMode = false;

		PPUPlayerInterop.VideoOutSample[] ScanBuffer;
		int WritePtr = 0;
		bool SyncFound = false;
		int SyncPos = -1;

		Color[] field = new Color[256 * 240];
		int CurrentScan = 0;

		// Visualization settings

		static int PixelsPerSample = 1;     // Discrete pixels (Bitmap)
		static int ScaleY = 1;
		Color scanBgColor = Color.Gray;
		Color scanColor = Color.AliceBlue;
		Color pictureDelimiterColor = Color.Tomato;

		Bitmap? scan_pic = null;
		Bitmap? field_pic = null;

		void ResetVisualize(bool RAWMode)
		{
			PPUPlayerInterop.GetSignalFeatures(out ppu_features);

			SamplesPerScan = ppu_features.PixelsPerScan * ppu_features.SamplesPerPCLK;
			ScanBuffer = new PPUPlayerInterop.VideoOutSample[2 * SamplesPerScan];
			WritePtr = 0;

			RawMode = RAWMode;
			PPUPlayerInterop.SetRAWColorMode(RAWMode);

			SyncFound = false;
			SyncPos = -1;
			CurrentScan = 0;

			scan_pic = null;
			field_pic = null;
			GC.Collect();
		}

		void ProcessSample(PPUPlayerInterop.VideoOutSample sample)
		{
			ScanBuffer[WritePtr] = sample;

			// Check that the sample is HSync.

			bool sync = false;

			if (RawMode)
			{
				sync = (sample.raw & 0b1000000000) != 0;
			}
			else
			{
				if (sample.composite <= ppu_features.SyncLevel && ppu_features.Composite != 0)
				{
					sync = true;
				}
				else if (sample.nSYNC == 0 && ppu_features.Composite == 0)
				{
					sync = true;
				}
			}

			// If the beginning of HSync is found - remember its offset in the input buffer.

			if (sync && !SyncFound)
			{
				SyncPos = WritePtr;
				SyncFound = true;
			}

			// Advance write pointer

			WritePtr++;

			// If HSync is found and the number of samples is more than enough, process the scan.

			if ( SyncFound && (SyncPos + WritePtr) >= SamplesPerScan )
			{
				if (RawMode)
				{
					ProcessScanRAW();
				}
				else
				{
					if (ppu_features.Composite != 0)
					{
						ProcessScanComposite();

						if (pictureBoxScan.Visible)
						{
							VisualizeScanComposite();
						}
					}
					else
					{
						ProcessScanRGB();
					}
				}

				SyncFound = false;
				WritePtr = 0;
			}
		}

		void ProcessScanRAW()
		{
			int ReadPtr = SyncPos;

			// Skip HSync and Back Porch

			while ((ScanBuffer[ReadPtr].raw & 0b1000000000) != 0)
			{
				ReadPtr++;
			}

			ReadPtr += ppu_features.BackPorchSize * ppu_features.SamplesPerPCLK;

			// Output the visible part of the signal

			for (int i = 0; i < 256; i++)
			{
				if (CurrentScan < 240)
				{
					byte r, g, b;
					PPUPlayerInterop.ConvertRAWToRGB(ScanBuffer[ReadPtr].raw, out r, out g, out b);

					field[CurrentScan * 256 + i] = Color.FromArgb(r, g, b);
				}

				ReadPtr += ppu_features.SamplesPerPCLK;
			}

			CurrentScan++;
			if (CurrentScan >= ppu_features.ScansPerField)
			{
				VisualizeField();
				CurrentScan = 0;
			}
		}

		void ProcessScanRGB()
		{
			int ReadPtr = SyncPos;

			// Skip HSync and Back Porch

			while (ScanBuffer[ReadPtr].nSYNC == 0)
			{
				ReadPtr++;
			}

			ReadPtr += ppu_features.BackPorchSize * ppu_features.SamplesPerPCLK;

			// Output the visible part of the signal

			for (int i = 0; i < 256; i++)
			{
				if (CurrentScan < 240)
				{
					var sample = ScanBuffer[ReadPtr];

					byte r = sample.RED;
					byte g = sample.GREEN;
					byte b = sample.BLUE;

					field[CurrentScan * 256 + i] = Color.FromArgb(r, g, b);
				}

				ReadPtr += ppu_features.SamplesPerPCLK;
			}

			CurrentScan++;
			if (CurrentScan >= ppu_features.ScansPerField)
			{
				VisualizeField();
				CurrentScan = 0;
			}
		}

		void ProcessScanComposite()
		{
			int ReadPtr = SyncPos;
			int num_phases = 12;
			float normalize_factor = 1.0f / ppu_features.WhiteLevel;
			PPUPlayerInterop.VideoOutSample[] batch = new PPUPlayerInterop.VideoOutSample[num_phases];

			// Skip HSync

			while (ScanBuffer[ReadPtr].composite <= ppu_features.SyncLevel)
			{
				ReadPtr++;
			}

			// Detect Color Burst Phase (simulate TV PLL)

			int cb_phase = PLL();

			ReadPtr += ppu_features.BackPorchSize * ppu_features.SamplesPerPCLK;

			// Output the visible part of the signal

			for (int i = 0; i < 256; i++)
			{
				if (CurrentScan < 240)
				{
					// Extract the median sample batch

					int MidPtr = ReadPtr + i * ppu_features.SamplesPerPCLK - num_phases / 2;

					for (int n = 0; n < num_phases; n++)
					{
						batch[n] = ScanBuffer[MidPtr++];
					}

					// YIQ

					float Y = 0, I = 0, Q = 0;

					float ofs = i * ppu_features.SamplesPerPCLK;

					// TBD: For now, a hack to tweak Hue. Interestingly, PPUSim does not require this tweaking.
					float hue_adj = 0.35f;

					for (int n = 0; n < num_phases; n++)
					{
						float level = ((batch[n].composite - ppu_features.BurstLevel) * normalize_factor) / num_phases;
						Y += level;
						I += level * (float)Math.Cos((cb_phase + n + ofs) * (2 * Math.PI / num_phases) + hue_adj);
						Q += level * (float)Math.Sin((cb_phase + n + ofs) * (2 * Math.PI / num_phases) + hue_adj);
					}

					// 6500K color temperature

					float R = Y + (0.956f * I) + (0.623f * Q);
					float G = Y - (0.272f * I) - (0.648f * Q);
					float B = Y - (1.105f * I) + (1.705f * Q);

					byte r = (byte)(Clamp(R, 0.0f, 1.0f) * 255);
					byte g = (byte)(Clamp(G, 0.0f, 1.0f) * 255);
					byte b = (byte)(Clamp(B, 0.0f, 1.0f) * 255);

					field[CurrentScan * 256 + i] = Color.FromArgb(r, g, b);
				}
			}

			CurrentScan++;
			if (CurrentScan >= ppu_features.ScansPerField)
			{
				VisualizeField();
				CurrentScan = 0;
			}
		}

		int PLL()
		{
			int ReadPtr = SyncPos;
			int num_phases = 12;
			int samples = ppu_features.BackPorchSize * ppu_features.SamplesPerPCLK;

			// Skip HSync

			while (ScanBuffer[ReadPtr].composite <= ppu_features.SyncLevel)
			{
				ReadPtr++;
			}

			// Lock phase of color burst

			while (ScanBuffer[ReadPtr++].composite == ppu_features.BurstLevel && samples != 0)
			{
				samples--;
			}

			// Get phase shift

			int cb_shift = 0;

			if (ScanBuffer[ReadPtr].composite < ppu_features.BurstLevel)
			{
				while (ScanBuffer[ReadPtr].composite < ppu_features.BurstLevel && samples != 0)
				{
					samples--;
					cb_shift--;
					ReadPtr++;
				}
			}
			else
			{
				while (ScanBuffer[ReadPtr].composite > ppu_features.BurstLevel && samples != 0)
				{
					samples--;
					cb_shift++;
					ReadPtr++;
				}

			}

			return cb_shift % num_phases;
		}

		float Clamp(float val, float min, float max)
		{
			return Math.Min(max, Math.Max(val, min));
		}

		/// <summary>
		/// The appearance of the signal repeats the familiar pictures from Wikipedia. Upper level IRE = 200 (with reserve), 0 = Blank Level, IRE = -50 lower level.
		/// Therefore, the height of the picture is 250 IRE.
		/// When converting the sample to the IRE level, it is additionally shifted to make it look nice.
		/// </summary>
		void VisualizeScanComposite()
		{
			int ReadPtr = SyncPos;
			int w = SamplesPerScan * PixelsPerSample;
			int h = 2000;
			float IRE = (ppu_features.WhiteLevel - ppu_features.BurstLevel) / 100.0f;
			int max_samples = SamplesPerScan;

			//if (scan_pic == null)
			{
				scan_pic = new(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
			}

			while (ScanBuffer[ReadPtr].composite <= ppu_features.SyncLevel)
			{
				ReadPtr++;
				max_samples--;
			}

			// TBD: Think about the best way to make HSync visible, too.

			int hsync_size = 12;

			ReadPtr -= hsync_size * ppu_features.SamplesPerPCLK;
			max_samples += hsync_size * ppu_features.SamplesPerPCLK;

			//int PPUPicturePortion = (hsync_size + ppu_features.BackPorchSize) * ppu_features.SamplesPerPCLK * PixelsPerSample;

			Bitmap pic = scan_pic;
			Graphics gr = Graphics.FromImage(pic);
			gr.Clear(scanBgColor);
			Pen pen = new(scanColor);
			Point prevPt = new(-SamplesPerScan, 0);

			for (int n = 0; n < max_samples; n++)
			{
				float sample = ScanBuffer[ReadPtr + n].composite;
				float ires = ((sample - ppu_features.BurstLevel) / IRE) * ScaleY;

				Point pt = new(PixelsPerSample * n, h - ((int)ires + 300));
				gr.DrawLine(pen, prevPt, pt);
				prevPt = pt;
			}

			// TBD: Draw the separator for the visible part of the signal.

			//gr.DrawLine(new(pictureDelimiterColor),
			//	new(PPUPicturePortion * PixelsPerSample, 0),
			//	new(PPUPicturePortion * PixelsPerSample, h - 1));

			pictureBoxScan.Image = scan_pic;
			gr.Dispose();
		}

		void VisualizeField()
		{
			int w = 256;
			int h = 240;

			//if (field_pic == null)
			{
				field_pic = new(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
			}

			Bitmap pic = field_pic;
			Graphics gr = Graphics.FromImage(pic);

			for (int y=0; y<h; y++)
			{
				for (int x=0; x<w; x++)
				{
					Color p = field[w * y + x];
					gr.FillRectangle(new SolidBrush(p), x, y, 1, 1);
				}
			}

			pictureBoxField.Image = field_pic;
			gr.Dispose();
		}
	}
}
