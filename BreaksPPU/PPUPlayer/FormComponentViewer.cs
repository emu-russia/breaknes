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
	public partial class FormComponentViewer : Form
	{
		PPUPlayerInterop.VideoSignalFeatures ppu_features;
		int SamplesPerScan;

		PPUPlayerInterop.VideoOutSample[]? ScanBuffer;
		int WritePtr = 0;
		bool SyncFound = false;
		int SyncPos = -1;

		Color[] field = new Color[256 * 240];
		int CurrentScan = 0;
		List<Bitmap> fields = new List<Bitmap>();

		public FormComponentViewer(string filename)
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

				sample.RED = (byte)(dump[i] & 0xff);
				sample.GREEN = (byte)((dump[i] >> 8) & 0xff);
				sample.BLUE = (byte)((dump[i] >> 16) & 0xff);
				sample.nSYNC = (byte)((dump[i] >> 24) & 0xff);

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

		void ResetVisualize()
		{
			PPUPlayerInterop.GetSignalFeatures(out ppu_features);

			SamplesPerScan = ppu_features.PixelsPerScan * ppu_features.SamplesPerPCLK;
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
			sync = sample.nSYNC == 0;

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
				ProcessScanRGB();

				SyncFound = false;
				WritePtr = 0;
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
				AddField();
				Console.WriteLine("Added Field");
				CurrentScan = 0;
			}
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

		private void FormComponentViewer_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
