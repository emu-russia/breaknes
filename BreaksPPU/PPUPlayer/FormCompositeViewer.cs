using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.IO;
using System.Runtime.InteropServices;

namespace PPUPlayer
{
	public partial class FormCompositeViewer : Form
	{
		PPUPlayerInterop.VideoSignalFeatures ppu_features;
		int SamplesPerScan;

		PPUPlayerInterop.VideoOutSample[] ScanBuffer;
		int WritePtr = 0;
		bool SyncFound = false;
		int SyncPos = -1;

		Color[] field = new Color[256 * 240];
		int CurrentScan = 0;
		List<Bitmap> fields = new List<Bitmap>();

		// https://www.codeproject.com/Messages/5517608/conversion-from-Uint32-to-float.aspx
		[StructLayout(LayoutKind.Explicit)]
		public struct Uifl
		{
			[FieldOffset(0)]
			public float fl;
			[FieldOffset(0)]
			public uint ui;
		};

		public FormCompositeViewer(string filename)
		{
			InitializeComponent();

			// Load the Logisim dump

			string text = File.ReadAllText(filename);
			UInt32[] dump = LogisimHEXConv.HEXToUIntArray(text);

			// Process the dump field by field and fill the ComboBox and PictureView.

			ResetVisualize();

			comboBox1.Items.Clear();

			for (int i = 0; i < dump.Length; i++)
			{
				PPUPlayerInterop.VideoOutSample sample = new();

				Uifl uifl = new Uifl();
				uifl.ui = dump[i];
				sample.composite = uifl.fl;

				ProcessSample(sample);
			}

			if (fields.Count != 0)
			{
				ShowField(0);
			}
			else
			{
				AddField();
				Console.WriteLine("Added partial field.");
				ShowField(0);
			}
		}

		private void FormCompositeViewer_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}

		void ResetVisualize()
		{
			PPUPlayerInterop.GetSignalFeatures(out ppu_features);

			// In Logisim the samples are sampled every CLK, not every half-cycle as in combat. Therefore, you must divide by 2.
			SamplesPerScan = ppu_features.PixelsPerScan * ppu_features.SamplesPerPCLK / 2;
			ScanBuffer = new PPUPlayerInterop.VideoOutSample[2 * SamplesPerScan];
			WritePtr = 0;

			SyncFound = false;
			SyncPos = -1;
			CurrentScan = 0;
		}

		void ProcessSample(PPUPlayerInterop.VideoOutSample sample)
		{
			ScanBuffer[WritePtr] = sample;

			// Check that the sample is HSync.

			bool sync = false;
			sync = sample.composite <= ppu_features.SyncLevel;

			// If the beginning of HSync is found - remember its offset in the input buffer.

			if (sync && !SyncFound)
			{
				SyncPos = WritePtr;
				SyncFound = true;
			}

			// Advance write pointer

			WritePtr++;

			// If HSync is found and the number of samples is more than enough, process the scan.

			if (SyncFound && (SyncPos + WritePtr) >= SamplesPerScan)
			{
				ProcessScanComposite();

				SyncFound = false;
				WritePtr = 0;
			}
		}

		void ProcessScanComposite()
		{
			int ReadPtr = SyncPos;
			int num_phases = 12;
			float normalize_factor = 1.0f / ppu_features.WhiteLevel;
			PPUPlayerInterop.VideoOutSample[] batch = new PPUPlayerInterop.VideoOutSample[num_phases];

			// In Logisim the samples are sampled every CLK, not every half-cycle as in combat. Therefore, you must divide by 2.
			int SamplesPerPixelInLogisimDump = ppu_features.SamplesPerPCLK / 2;

			// Skip HSync

			while (ScanBuffer[ReadPtr].composite <= ppu_features.SyncLevel)
			{
				ReadPtr++;
			}

			// Detect Color Burst Phase (simulate TV PLL)

			int cb_phase = PLL();

			ReadPtr += ppu_features.BackPorchSize * SamplesPerPixelInLogisimDump;

			// Output the visible part of the signal

			for (int i = 0; i < 256; i++)
			{
				if (CurrentScan < 240)
				{
					// Extract the median sample batch

					int MidPtr = ReadPtr + i * SamplesPerPixelInLogisimDump - num_phases / 2;

					for (int n = 0; n < num_phases; n++)
					{
						batch[n] = ScanBuffer[MidPtr++];
					}

					// YIQ

					float Y = 0, I = 0, Q = 0;

					float ofs = i * SamplesPerPixelInLogisimDump;

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
				AddField();
				Console.WriteLine("Added Field");
				CurrentScan = 0;
			}
		}

		int PLL()
		{
			// In Logisim the samples are sampled every CLK, not every half-cycle as in combat. Therefore, you must divide by 2.
			int SamplesPerPixelInLogisimDump = ppu_features.SamplesPerPCLK / 2;

			int ReadPtr = SyncPos;
			int num_phases = 12;
			int samples = ppu_features.BackPorchSize * SamplesPerPixelInLogisimDump;

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

		void AddField()
		{
			int w = 256;
			int h = 240;

			comboBox1.Items.Add(fields.Count.ToString());

			Bitmap? field_pic = new(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

			Bitmap pic = field_pic;
			Graphics gr = Graphics.FromImage(pic);

			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					Color p = field[w * y + x];
					gr.FillRectangle(new SolidBrush(p), x, y, 1, 1);
				}
			}

			fields.Add(field_pic);
			gr.Dispose();
		}

		void ShowField(int n)
		{
			if (n < fields.Count)
			{
				pictureBox1.Image = fields[n];
				Console.WriteLine("Show Field: " + n.ToString());
			}
		}

		private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
		{
			int n = comboBox1.SelectedIndex;
			ShowField(n);
		}
	}
}
