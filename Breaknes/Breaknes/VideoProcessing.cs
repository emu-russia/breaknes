using SharpTools;

namespace Breaknes
{
	public class VideoRender
	{
		private BreaksCore.VideoSignalFeatures ppu_features;
		private int SamplesPerScan;

		private BreaksCore.VideoOutSample[] ScanBuffer;
		private int WritePtr = 0;
		private bool SyncFound = false;
		private int SyncPos = -1;

		private Color[] field = new Color[256 * 240];
		private byte[] raw_field = new byte[256 * 240 * 2];
		private int CurrentScan = 0;

		private Bitmap? field_pic = null;
		private PictureBox? output_picture_box = null;

		private OnRenderField? onRenderField = null;
		public delegate void OnRenderField();

		private long field_counter = 0;

		private bool dump_video = false;
		private string dump_video_dir = "";
		private string dump_rom_name;

		public VideoRender(OnRenderField _onRender, bool dump, string dump_dir, string rom_name)
		{
			onRenderField = _onRender;
			BreaksCore.GetPpuSignalFeatures(out ppu_features);

			dump_video = dump;
			dump_video_dir = dump_dir;
			dump_rom_name = rom_name;

			SamplesPerScan = ppu_features.PixelsPerScan * ppu_features.SamplesPerPCLK;
			ScanBuffer = new BreaksCore.VideoOutSample[2 * SamplesPerScan];
			WritePtr = 0;

			SyncFound = false;
			SyncPos = -1;
			CurrentScan = 0;

			field_pic = null;
			GC.Collect();
		}

		public void SetOutputPictureBox (PictureBox pictureBox)
		{
			output_picture_box = pictureBox;
		}

		public void ProcessSample (BreaksCore.VideoOutSample sample)
		{
			ScanBuffer[WritePtr] = sample;

			// Check that the sample is HSync.

			bool sync = false;
			sync = (sample.raw & 0b1000000000) != 0;

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
				ProcessScanRAW();

				SyncFound = false;
				WritePtr = 0;
			}

			if (WritePtr >= 2 * SamplesPerScan)
			{
				SyncFound = false;
				WritePtr = 0;
			}
		}

		private void ProcessScanRAW()
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
					BreaksCore.ConvertRAWToRGB(ScanBuffer[ReadPtr].raw, out r, out g, out b);

					field[CurrentScan * 256 + i] = Color.FromArgb(r, g, b);

					if (dump_video)
					{
						UInt16 raw_color = ScanBuffer[ReadPtr].raw;
						raw_field[2 * CurrentScan * 256 + i] = (byte)((raw_color >> 0) & 0xff);
						raw_field[2 * CurrentScan * 256 + i + 1] = (byte)((raw_color >> 0) & 0xff);
					}
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

		private void VisualizeField()
		{
			int w = 256;
			int h = 240;

			//if (field_pic == null)
			{
				field_pic = new(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
			}

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

			DumpVideo();

			if (output_picture_box != null)
			{
				output_picture_box.Image = field_pic;
			}
			gr.Dispose();

			field_counter++;

			onRenderField?.Invoke();
		}

		private void DumpVideo()
		{
			if (dump_video)
			{
				string raw_name = dump_video_dir + "/" + dump_rom_name + "_" + field_counter.ToString("D5") + ".bin";
				string bmp_name = dump_video_dir + "/" + dump_rom_name + "_" + field_counter.ToString("D5") + ".bmp";

				File.WriteAllBytes(raw_name, raw_field);
				if (field_pic != null)
				{
					field_pic.Save(bmp_name, System.Drawing.Imaging.ImageFormat.Bmp);
				}

				Console.WriteLine("field: {0}", field_counter);
			}
		}
	}
}
