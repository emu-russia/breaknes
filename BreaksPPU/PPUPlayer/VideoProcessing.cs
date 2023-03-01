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

using SharpTools;

// Samples are stored in ScanBuffer in raw form. After processing, the Scan enters the Field in a ready for output form (RGB).

namespace PPUPlayer
{
	public partial class FormMain : Form
	{
		BreaksCoreInterop.VideoSignalFeatures ppu_features;
		int SamplesPerScan;
		bool RawMode = false;

		BreaksCoreInterop.VideoOutSample[] ScanBuffer;
		int WritePtr = 0;
		bool SyncFound = false;
		int SyncPos = -1;

		Color[] field = new Color[256 * 240];
		int CurrentScan = 0;

		float[] composite_samples = Array.Empty<float>();

		Bitmap? field_pic = null;

		bool DumpToFile = false;
		Stream VideoStream;

		void ResetVisualize(bool RAWMode)
		{
			BreaksCoreInterop.GetPpuSignalFeatures(out ppu_features);

			SamplesPerScan = ppu_features.PixelsPerScan * ppu_features.SamplesPerPCLK;
			ScanBuffer = new BreaksCoreInterop.VideoOutSample[2 * SamplesPerScan];
			WritePtr = 0;

			RawMode = RAWMode;
			BreaksCoreInterop.SetRAWColorMode(RAWMode);

			SyncFound = false;
			SyncPos = -1;
			CurrentScan = 0;

			composite_samples = new float[SamplesPerScan];
			signalPlotScan.ForceMinMax(true, -0.5f, ppu_features.WhiteLevel * 2);
			signalPlotScan.EnabledDottedEveryNth(ppu_features.SamplesPerPCLK, !RAWMode);

			field_pic = null;
			GC.Collect();
		}

		void ProcessSample(BreaksCoreInterop.VideoOutSample sample)
		{
			ScanBuffer[WritePtr] = sample;

			// Dump to file (slow)

			if (DumpToFile)
			{
				if (RawMode)
				{
					byte[] buffer = BitConverter.GetBytes(sample.raw);
					VideoStream.Write(buffer, 0, buffer.Length);
				}
				else
				{
					if (ppu_features.Composite != 0)
					{
						byte[] buffer = BitConverter.GetBytes(sample.composite);
						VideoStream.Write(buffer, 0, buffer.Length);
					}
					else
					{
						byte[] buffer = new byte[4];
						buffer[0] = sample.RED;
						buffer[1] = sample.GREEN;
						buffer[2] = sample.BLUE;
						buffer[3] = (byte)(sample.nSYNC == 0 ? 1 : 0);
						VideoStream.Write(buffer, 0, buffer.Length);
					}
				}
			}

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

						if (signalPlotScan.Visible)
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

			if (WritePtr >= 2 * SamplesPerScan)
			{
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
					BreaksCoreInterop.ConvertRAWToRGB(ScanBuffer[ReadPtr].raw, out r, out g, out b);

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

				if (ReadPtr >= ScanBuffer.Length)
					break;
			}

			if (ReadPtr >= ScanBuffer.Length)
			{
				return;
			}

			ReadPtr += ppu_features.BackPorchSize * ppu_features.SamplesPerPCLK;

			// Output the visible part of the signal

			for (int i = 0; i < 256; i++)
			{
				if (CurrentScan < 240)
				{
					int red = 0, green = 0, blue = 0;

					for (int n = 0; n < ppu_features.SamplesPerPCLK; n++)
					{
						var sample = ScanBuffer[ReadPtr];
						red += sample.RED;
						green += sample.GREEN;
						blue += sample.BLUE;
						ReadPtr++;
					}

					byte r = (byte)(red / ppu_features.SamplesPerPCLK);
					byte g = (byte)(green / ppu_features.SamplesPerPCLK);
					byte b = (byte)(blue / ppu_features.SamplesPerPCLK);

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

		void ProcessScanComposite()
		{
			int ReadPtr = SyncPos;
			int num_phases = 12;
			float normalize_factor = 1.1f / ppu_features.WhiteLevel;
			BreaksCoreInterop.VideoOutSample[] batch = new BreaksCoreInterop.VideoOutSample[num_phases];

			// TBD: Make phasing per scanline, not the semi-conservative way (PLL at the beginning of Field, then interpolate)

			// Skip HSync

			while (ScanBuffer[ReadPtr].composite <= ppu_features.SyncLevel && ReadPtr < (ScanBuffer.Length - 1))
			{
				ReadPtr++;
			}

			if (ReadPtr == (ScanBuffer.Length - 1))
				return;

			// Detect Color Burst Phase (simulate TV PLL)

			bool odd = (CurrentScan % 2) != 0 && ppu_features.PhaseAlteration != 0;
			int cb_phase = PLL(odd, ppu_features.PhaseAlteration != 0 ? 10 : 0);

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
						float level = ((batch[n].composite - ppu_features.BlackLevel) * normalize_factor) / num_phases;
						Y += level;
						I += level * (float)Math.Cos((cb_phase + n + ofs) * (2 * Math.PI / num_phases) + hue_adj);
						Q += level * (float)Math.Sin((cb_phase + n + ofs) * (2 * Math.PI / num_phases) + hue_adj) * (odd ? -1.0f : +1.0f);
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

		int prev_cb_shift = 0;

		int PLL(bool odd, int cb)
		{
			int ReadPtr = SyncPos;
			int num_phases = 12;
			int samples = ppu_features.BackPorchSize * ppu_features.SamplesPerPCLK;

			if (odd)
			{
				return Math.Abs((prev_cb_shift + 5) % num_phases);
			}

			// Skip HSync

			while (ScanBuffer[ReadPtr].composite <= ppu_features.SyncLevel && ReadPtr < (ScanBuffer.Length - 1))
			{
				ReadPtr++;
			}

			if (ReadPtr == (ScanBuffer.Length - 1))
				return 0;

			// Lock phase of color burst

			while (ScanBuffer[ReadPtr++].composite == ppu_features.BlackLevel && samples != 0)
			{
				samples--;
			}

			// Get phase shift

			int cb_shift = cb;

			if (ScanBuffer[ReadPtr].composite < ppu_features.BlackLevel)
			{
				while (ScanBuffer[ReadPtr].composite < ppu_features.BlackLevel && samples != 0)
				{
					samples--;
					cb_shift--;
					ReadPtr++;
				}
			}
			else
			{
				while (ScanBuffer[ReadPtr].composite > ppu_features.BlackLevel && samples != 0)
				{
					samples--;
					cb_shift++;
					ReadPtr++;
				}
			}

			int res = cb_shift % num_phases;
			prev_cb_shift = res;
			return res;
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
			int max_samples = SamplesPerScan;

			for (int i=0; i<max_samples; i++)
			{
				composite_samples[i] = ScanBuffer[ReadPtr + i].composite;
			}

			signalPlotScan.PlotSignal(composite_samples);
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

		private void saveFieldAsImageToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (saveFileDialogImage.ShowDialog() == DialogResult.OK)
			{
				string bmp_name = saveFileDialogImage.FileName;
				if (field_pic != null)
				{
					field_pic.Save(bmp_name, System.Drawing.Imaging.ImageFormat.Bmp);
				}
			}
		}

		private void startVideoSignalDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (saveFileDialog1.ShowDialog() == DialogResult.OK)
			{
				VideoStream = File.Create(saveFileDialog1.FileName);
				DumpToFile = true;
			}
		}

		private void stopVideoSignalDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DumpToFile = false;
			VideoStream.Close();
		}

		private void toolStripButton4_Click(object sender, EventArgs e)
		{
			if (Paused && SimulationStarted)
			{
				if (signalPlotScan.IsSelectedSomething())
				{
					float[] data = signalPlotScan.SnatchSelection();
					signalPlotScan.ClearSelection();
					FormSnatch snatch = new FormSnatch(data);
					snatch.Show();
				}
				else
				{
					MessageBox.Show("Select something first with the left mouse button. A box will appear, then click on Snatch.", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				}
			}
			else
			{
				if (backgroundWorker1.IsBusy)
				{
					MessageBox.Show("I can't while the worker is running. Pause it first", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				}
				else
				{
					MessageBox.Show("Simulation not started", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				}
			}
		}
	}
}
