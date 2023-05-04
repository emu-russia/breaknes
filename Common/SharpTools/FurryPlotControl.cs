using System.Numerics;

namespace System.Windows.Forms
{
	public partial class FurryPlot : Control
	{
		private BufferedGraphics? gfx;
		private BufferedGraphicsContext? context;

		private const int fftSize = 1 << 9;     // The size of a single batch for the FFT. Power of 2
		private const int fftSpans = 512;       // FFT history size

		private Complex[][] spans = new Complex[fftSpans][];        // History of all FFT windows
		private int current_span = 0;

		private Complex[] sample_buffer = new Complex[fftSize];     // Buffer for the current signal batch
		private int sample_ptr = 0;

		private bool draw = false;      // atomic

		private readonly double[] window = new double[fftSize];

		public FurryPlot()
		{
			SetStyle(ControlStyles.OptimizedDoubleBuffer, true);

			for (int i=0; i<fftSpans; i++)
			{
				spans[i] = new Complex[fftSize];
			}

			// Hann
			for (int i = 0; i < fftSize; i++)
			{
				window[i] = 0.5 - 0.5 * Math.Cos(2 * Math.PI * i / (fftSize - 1));
			}
		}

		/// <summary>
		/// Add a sample by performing a window transformation beforehand.
		/// When there are enough samples for the size of the FFT window, the whole batch is processed.
		/// </summary>
		public void AddSample (float sample)
		{
			while (draw) ;

			sample_buffer[sample_ptr] = sample * (float)window[sample_ptr];
			sample_ptr++;
			if (sample_ptr >= fftSize)
			{
				DoFft();
				sample_ptr = 0;
				Invalidate();
			}
		}

		private void DoFft ()
		{
			Complex[] res = fft(sample_buffer);
			for (int i=0; i<fftSize; i++)
			{
				spans[current_span][i] = res[i];
			}
			current_span++;
			if (current_span >= fftSpans)
			{
				current_span = 0;
			}
		}

		private void DrawFft(Graphics gr)
		{
			Bitmap bmp = new Bitmap(fftSpans, fftSize / 2);

			for (int x=0; x<fftSpans; x++)
			{
				int real_x = (current_span + x) % fftSpans;
				for (int y = 0; y < fftSize / 2; y++)
				{
					int real_y = fftSize / 2 + y;
					var val = spans[real_x][real_y].Magnitude;
					bmp.SetPixel(fftSpans - x - 1, y, GetInfernoRGB((byte)val));
				}
			}

			gr.DrawImage(bmp, 0, 0, Width, Height);
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			draw = true;
			if (gfx == null)
			{
				ReallocateGraphics();
			}

			if (Width < 32 || Height < 32)
			{
				draw = false;
				return;
			}

			if (gfx != null)
			{
				DrawFft(gfx.Graphics);
				gfx.Render(e.Graphics);
			}
			draw = false;
		}

		protected override void OnSizeChanged(EventArgs e)
		{
			if (gfx != null)
			{
				gfx.Dispose();
				if (Width > 1 && Height > 1)
					ReallocateGraphics();
			}

			Invalidate();
			base.OnSizeChanged(e);
		}

		private void ReallocateGraphics()
		{
			context = BufferedGraphicsManager.Current;
			context.MaximumBuffer = new Size(Width + 1, Height + 1);

			gfx = context.Allocate(CreateGraphics(),
				 new Rectangle(0, 0, Width, Height));
		}

		// https://ru.wikibooks.org/wiki/Реализации_алгоритмов/Быстрое_преобразование_Фурье

		private Complex w(int k, int N)
		{
			if (k % N == 0) return 1;
			double arg = -2 * Math.PI * k / N;
			return new Complex(Math.Cos(arg), Math.Sin(arg));
		}

		private Complex[] fft(Complex[] x)
		{
			Complex[] X;
			int N = x.Length;
			if (N == 2)
			{
				X = new Complex[2];
				X[0] = x[0] + x[1];
				X[1] = x[0] - x[1];
			}
			else
			{
				Complex[] x_even = new Complex[N / 2];
				Complex[] x_odd = new Complex[N / 2];
				for (int i = 0; i < N / 2; i++)
				{
					x_even[i] = x[2 * i];
					x_odd[i] = x[2 * i + 1];
				}
				Complex[] X_even = fft(x_even);
				Complex[] X_odd = fft(x_odd);
				X = new Complex[N];
				for (int i = 0; i < N / 2; i++)
				{
					X[i] = X_even[i] + w(i, N) * X_odd[i];
					X[i + N / 2] = X_even[i] - w(i, N) * X_odd[i];
				}
			}
			return X;
		}

		// https://github.com/swharden/Spectrogram/blob/main/src/Spectrogram/Colormaps/Inferno.cs

		private Color GetInfernoRGB(byte value)
		{
			byte[] bytes = BitConverter.GetBytes(rgb[value]);
			return Color.FromArgb(bytes[2], bytes[1], bytes[0]);
		}

		private readonly int[] rgb =
		{
			00000003, 00000004, 00000006, 00065543, 00065801, 00065803, 00131342, 00131600,
			00197138, 00262932, 00262934, 00328728, 00394267, 00460061, 00525855, 00591393,
			00657187, 00722726, 00854056, 00919594, 00985389, 01050927, 01182258, 01247796,
			01313590, 01444665, 01510203, 01641278, 01706816, 01838147, 01903685, 02034759,
			02100298, 02231116, 02362190, 02493264, 02558802, 02689876, 02820694, 02951768,
			03017306, 03148380, 03279197, 03410271, 03475808, 03606881, 03737954, 03869028,
			03934565, 04065638, 04196710, 04262247, 04393576, 04524649, 04590185, 04721514,
			04852586, 04918379, 05049451, 05180780, 05246316, 05377644, 05443181, 05574509,
			05705581, 05771373, 05902701, 05968238, 06099566, 06230638, 06296430, 06427758,
			06493294, 06624622, 06690158, 06821486, 06952814, 07018350, 07149678, 07215214,
			07346542, 07477613, 07543405, 07674733, 07740269, 07871597, 08002669, 08068460,
			08199532, 08265324, 08396651, 08462187, 08593515, 08724586, 08790378, 08921450,
			08987241, 09118313, 09249641, 09315432, 09446504, 09512295, 09643367, 09774694,
			09840230, 09971557, 10037348, 10168420, 10234211, 10365283, 10496610, 10562401,
			10693473, 10759264, 10890335, 10956127, 11087454, 11218525, 11284316, 11415643,
			11481435, 11612506, 11678297, 11809624, 11875159, 12006486, 12072278, 12203605,
			12269396, 12400467, 12466258, 12532049, 12663376, 12729167, 12860494, 12926285,
			13057612, 13123147, 13188938, 13320265, 13386056, 13451847, 13583430, 13649220,
			13715011, 13780802, 13912129, 13977920, 14043711, 14109502, 14241085, 14306875,
			14372666, 14438457, 14504504, 14570295, 14636086, 14702132, 14833459, 14899250,
			14965297, 15031088, 15096878, 15097389, 15163180, 15229227, 15295018, 15361064,
			15426855, 15492902, 15558693, 15559203, 15625250, 15691041, 15757087, 15757342,
			15823389, 15889436, 15889690, 15955737, 15956248, 16022038, 16088085, 16088596,
			16154642, 16154897, 16220944, 16221454, 16287501, 16287756, 16288267, 16354313,
			16354824, 16355336, 16421127, 16421638, 16422150, 16422662, 16488710, 16489222,
			16489734, 16489991, 16490503, 16491016, 16491530, 16492043, 16492557, 16493070,
			16493584, 16494098, 16494612, 16494870, 16495384, 16495898, 16496412, 16496926,
			16431905, 16432419, 16432933, 16433448, 16368426, 16368940, 16369455, 16304433,
			16304948, 16305463, 16240442, 16240956, 16175935, 16176450, 16111429, 16111944,
			16046923, 16047183, 15982162, 15982678, 15983193, 15918173, 15918688, 15853668,
			15853928, 15854444, 15854960, 15855220, 15855737, 15856253, 15922049, 15922309,
			15988361, 16054157, 16119953, 16186005, 16251801, 16383133, 16448928, 16580260,
		};

	}
}
