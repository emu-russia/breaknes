using System;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Drawing.Imaging;

namespace System.Windows.Forms
{
	public partial class SignalPlot : Control
	{
		private BufferedGraphics? gfx;
		private BufferedGraphicsContext? context;

		private float[] data = Array.Empty<float>();
		private float minVal = float.MinValue;
		private float maxVal = float.MaxValue;
		private bool forced_minmax = false;

		private Pen? grid_pen;
		private Pen? signal_pen;
		private Brush? label_brush;
		private Pen? zero_pen;

		private bool gdi_init = false;

		private PointF p1 = new PointF(0, 0);
		private PointF p2 = new PointF(0, 0);
		private PointF tp = new PointF(0, 0);

		public SignalPlot()
		{
			SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
		}

		/// <summary>
		/// Prohibit automatic determination of min/max values for the plot.
		/// </summary>
		/// <param name="force">true: Prohibit automatic min/max detection; false: Allow</param>
		/// <param name="minv">Constant minimum value</param>
		/// <param name="maxv">Constant maximum value</param>
		public void ForceMinMax (bool force, float minv, float maxv)
		{
			minVal = minv;
			maxVal = maxv;
			forced_minmax = force;

			MinMaxPadding(5.0f);
		}

		/// <summary>
		/// Outputs a signal dump. Automatically performs a control update.
		/// </summary>
		/// <param name="samples">Signal dump, float array</param>
		public void PlotSignal(float[] samples)
		{
			if (data.Length != samples.Length)
			{
				data = new float[samples.Length];
			}
			Array.Copy(samples, data, samples.Length);

			if (!forced_minmax)
			{
				minVal = float.MaxValue;
				maxVal = float.MinValue;

				for (int i = 0; i < data.Length; i++)
				{
					if (data[i] < minVal)
						minVal = data[i];
					if (data[i] > maxVal)
						maxVal = data[i];
				}

				MinMaxPadding(5.0f);
			}

			if (!gdi_init)
			{
				grid_pen = new Pen(new SolidBrush(GridColor));
				signal_pen = new Pen(new SolidBrush(SignalColor));
				label_brush = new SolidBrush(LabelsColor);
				zero_pen = new Pen(new SolidBrush(ZeroColor));
				gdi_init = true;
			}

			Invalidate();
		}

		/// <summary>
		/// Make a slight padding of the signal (prc) at the top and bottom
		/// </summary>
		private void MinMaxPadding (float paddingPrc)
		{
			float gap = (paddingPrc * Math.Abs(maxVal - minVal)) / 100;
			maxVal += gap;
			minVal -= gap;
		}

		private void ReallocateGraphics()
		{
			context = BufferedGraphicsManager.Current;
			context.MaximumBuffer = new Size(Width + 1, Height + 1);

			gfx = context.Allocate(CreateGraphics(),
				 new Rectangle(0, 0, Width, Height));
		}

		private void DrawGrid (Graphics gr)
		{
			int horizontalStepping = data.Length / 10;
			float verticalStepping = (maxVal - minVal) / 20;

			if (grid_pen != null)
			{
				for (int i = 0; i < data.Length; i += horizontalStepping)
				{
					PointF pt = TransformCoord(i, 0.0f);
					p1.X = pt.X;
					p1.Y = 0;
					p2.X = pt.X;
					p2.Y = Height;
					gr.DrawLine(grid_pen, p1, p2);
				}

				for (float val = minVal; val < maxVal; val += verticalStepping)
				{
					PointF pt = TransformCoord(0, val);
					p1.X = 0;
					p1.Y = pt.Y;
					p2.X = Width;
					p2.Y = pt.Y;
					gr.DrawLine(grid_pen, p1, p2);
				}
			}

			// Zero
			if (zero_pen != null)
			{
				PointF pt = TransformCoord(0, 0);
				p1.X = 0;
				p1.Y = pt.Y;
				p2.X = Width;
				p2.Y = pt.Y;
				gr.DrawLine(zero_pen, p1, p2);
			}
		}

		private void DrawSignal (Graphics gr)
		{
			PointF prev = TransformCoord(0, data[0]);

			if (signal_pen != null)
			{
				for (int i = 1; i < data.Length; i++)
				{
					PointF pos = TransformCoord(i, data[i]);
					gr.DrawLine(signal_pen, prev, pos);
					prev = pos;
				}
			}
		}

		private PointF TransformCoord (int i, float val)
		{
			float x = (float)(i * Width) / data.Length;
			float delta = maxVal - minVal;
			if (maxVal == minVal)
				delta = 0.0001f;
			float y = Height - ((val - minVal) * Height) / delta;
			tp.X = x;
			tp.Y = y;
			return tp;
		}

		private void DrawLabels (Graphics gr)
		{
			int horizontalStepping = data.Length / 10;
			float verticalStepping = (maxVal - minVal) / 20;

			if (label_brush != null)
			{
				for (int i = horizontalStepping; i < data.Length; i += horizontalStepping)
				{
					PointF pt = TransformCoord(i, 0.0f);
					gr.DrawString(i.ToString(), Font, label_brush, pt);
				}

				for (float val = minVal; val < maxVal; val += verticalStepping)
				{
					PointF pt = TransformCoord(0, val);
					gr.DrawString(val.ToString("0.##"), Font, label_brush, pt);
				}
			}
		}

		private void DrawPlot (Graphics gr)
		{
			gr.Clear(FillColor);

			if (data.Length <= 1 || Width == 0 || Height == 0)
				return;

			DrawGrid(gr);
			DrawLabels(gr);
			DrawSignal(gr);
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			if (gfx == null)
			{
				ReallocateGraphics();
			}

			if (gfx != null)
			{
				DrawPlot(gfx.Graphics);
				gfx.Render(e.Graphics);
			}
		}

		protected override void OnSizeChanged(EventArgs e)
		{
			if (gfx != null)
			{
				gfx.Dispose();
				ReallocateGraphics();
			}

			Invalidate();
			base.OnSizeChanged(e);
		}

		[Category("Plot Appearance")]
		public Color FillColor { get; set; }

		[Category("Plot Appearance")]
		public Color ZeroColor { get; set; }

		[Category("Plot Appearance")]
		public Color SignalColor { get; set; }

		[Category("Plot Appearance")]
		public Color GridColor { get; set; }

		[Category("Plot Appearance")]
		public Color LabelsColor { get; set; }
	}
}
