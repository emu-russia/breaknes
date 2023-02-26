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
		private Pen? selection_pen;
		private Pen? dotted_pen;

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

				for (long i = 0; i < data.Length; i++)
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
				grid_pen = new Pen(Color.FromArgb(GridOpacity, GridColor));
				signal_pen = new Pen(new SolidBrush(SignalColor));
				label_brush = new SolidBrush(LabelsColor);
				zero_pen = new Pen(new SolidBrush(ZeroColor));
				selection_pen = new Pen(SelectionColor, 2);
				dotted_pen = new Pen(Color.FromArgb(DottedOpacity, DottedColor));
				dotted_pen.DashPattern = new float[] { 1, 4 };
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
			long horizontalStepping = data.Length / 10;
			float verticalStepping = (maxVal - minVal) / 20;

			if (grid_pen != null)
			{
				for (long i = 0; i < data.Length; i += horizontalStepping)
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
				for (long i = 1; i < data.Length; i++)
				{
					PointF pos = TransformCoord(i, data[i]);
					gr.DrawLine(signal_pen, prev, pos);
					prev = pos;
				}
			}
		}

		private PointF TransformCoord (long i, float val)
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

		private long InvTransform (int x)
		{
			return Math.Max(Math.Min((x * data.Length) / Width, data.Length-1), 0);
		}

		private void DrawLabels (Graphics gr)
		{
			long horizontalStepping = data.Length / 10;
			float verticalStepping = (maxVal - minVal) / 20;

			if (label_brush != null)
			{
				for (long i = horizontalStepping; i < data.Length; i += horizontalStepping)
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

			if (data.Length <= 10 || Width == 0 || Height == 0)
				return;

			DrawDotted(gr);
			DrawGrid(gr);
			DrawLabels(gr);
			DrawSignal(gr);
			DrawSelection(gr);
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


		#region "Selection/Snatch support"

		private bool selection_enabled = false;
		private int SelectStartMouseX;
		private int SelectStartMouseY;
		private int LastMouseX;
		private int LastMouseY;
		private bool selection_box_active = false;
		private int min_selection_square = 64;
		private bool selection_in_progress = false;
		private Point last_mouse_pos = new();

		public void EnableSelection (bool enable)
		{
			selection_enabled = enable;
			ClearSelection();
		}

		public void ClearSelection()
		{
			selection_box_active = false;
			selection_in_progress = false;
			Invalidate();
		}

		public bool IsSelectedSomething()
		{
			if (data.Length <= 10 || Width == 0 || Height == 0)
				return false;

			return selection_enabled && selection_box_active;
		}

		public float[] SnatchSelection ()
		{
			if (!IsSelectedSomething())
				return Array.Empty<float>();

			List<float> res = new();

			int dist = Math.Abs(SelectStartMouseX - LastMouseX);
			int start_x = Math.Min(SelectStartMouseX, LastMouseX);
			long sp = InvTransform(start_x);
			long ep = InvTransform(start_x + dist);

			for (long i=sp; i<ep; i++)
			{
				res.Add(data[i]);
			}

			return res.ToArray();
		}

		protected override void OnMouseDown(MouseEventArgs e)
		{
			if (e.Button == MouseButtons.Left && selection_enabled && !selection_in_progress)
			{
				Point pos = GetMousePos(e);
				LastMouseX = SelectStartMouseX = pos.X;
				LastMouseY = SelectStartMouseY = pos.Y;
				selection_box_active = true;
				selection_in_progress = true;
			}

			base.OnMouseDown(e);
		}

		protected override void OnMouseUp(MouseEventArgs e)
		{
			if (selection_enabled && selection_in_progress)
			{
				int square = Math.Abs(SelectStartMouseX - LastMouseX) * Math.Abs(SelectStartMouseY - LastMouseY);

				selection_box_active = (square > min_selection_square);
				selection_in_progress = false;

				Invalidate();
			}

			base.OnMouseUp(e);
		}

		protected override void OnMouseMove(MouseEventArgs e)
		{
			if (selection_enabled && selection_in_progress)
			{
				Point pos = GetMousePos(e);
				LastMouseX = Math.Max(Math.Min(pos.X, Width-1), 0);
				LastMouseY = Math.Max(Math.Min(pos.Y, Height-1), 0);
				Invalidate();
			}

			base.OnMouseMove(e);
		}

		/// <summary>
		/// Snap to every nth grid if dotted mapping is enabled.
		/// </summary>
		/// <param name="e"></param>
		/// <returns></returns>
		private Point GetMousePos (MouseEventArgs e)
		{
			if (dotted_enabled)
			{
				var sample_num = InvTransform(e.X);
				sample_num = (long)Math.Round((double)sample_num / dotted_every_nth) * dotted_every_nth;	// Snap to grid
				var mp = TransformCoord(sample_num, 0);
				last_mouse_pos.X = (int)mp.X;
				last_mouse_pos.Y = e.Y;
			}
			else
			{
				last_mouse_pos.X = e.X;
				last_mouse_pos.Y = e.Y;
			}

			return last_mouse_pos;
		}

		private void DrawSelection(Graphics gr)
		{
			if (selection_enabled && selection_box_active && selection_pen != null)
			{
				Point[] points = new Point[5];

				points[0] = new Point(SelectStartMouseX, SelectStartMouseY);
				points[1] = new Point(SelectStartMouseX, LastMouseY);
				points[2] = new Point(LastMouseX, LastMouseY);
				points[3] = new Point(LastMouseX, SelectStartMouseY);
				points[4] = new Point(SelectStartMouseX, SelectStartMouseY);

				gr.DrawLines(selection_pen, points);
			}
		}

		#endregion "Selection/Snatch support"


		#region "Dotted Every Nth support"

		private int dotted_every_nth = 0;
		private bool dotted_enabled = false;

		public void EnabledDottedEveryNth (int every_nth_sample, bool enable)
		{
			dotted_every_nth = every_nth_sample;
			dotted_enabled = enable;
			Invalidate();
		}

		private void DrawDotted(Graphics gr)
		{
			if (dotted_enabled && dotted_pen != null)
			{
				for (long i = 0; i < data.Length; i+=dotted_every_nth)
				{
					PointF pos = TransformCoord(i, 0);
					gr.DrawLine(dotted_pen, pos.X, 0, pos.X, Height);
				}
			}
		}

		#endregion "Dotted Every Nth support"


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

		[Category("Plot Appearance")]
		public Color SelectionColor { get; set; }

		[Category("Plot Appearance")]
		public Color DottedColor { get; set; }

		[Category("Plot Appearance")]
		public int GridOpacity { get; set; }

		[Category("Plot Appearance")]
		public int DottedOpacity { get; set; }
	}
}
