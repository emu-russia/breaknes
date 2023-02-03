using System.ComponentModel;

namespace System.Windows.Forms
{
    public partial class SignalPlot : Control
    {
        private BufferedGraphics? gfx;
        private BufferedGraphicsContext? context;

        private float[] data = Array.Empty<float>();
        private float minVal = float.MinValue;
        private float maxVal = float.MaxValue;

		public SignalPlot() => SetStyle(ControlStyles.OptimizedDoubleBuffer, true);

        public void PlotSignal(float[] samples)
        {
            data = new float[samples.Length];
            Array.Copy(samples, data, samples.Length);

            minVal = float.MaxValue;
            maxVal = float.MinValue;

            for (int i=0; i<data.Length; i++)
            {
                if (data[i] < minVal)
                    minVal = data[i];
                if (data[i] > maxVal)
                    maxVal = data[i];
			}

            // Make a slight padding of the signal (10%) at the top and bottom

            float paddingPrc = 5.0f;
			float gap = (paddingPrc * Math.Abs(maxVal - minVal)) / 100;
            maxVal += gap;
            minVal -= gap;

			Invalidate();
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
			Pen pen = new Pen(new SolidBrush(GridColor));

			int horizontalStepping = data.Length / 10;
            float verticalStepping = (maxVal - minVal) / 20;

			for (int i = 0; i < data.Length; i += horizontalStepping)
			{
				PointF pt = TransformCoord(i, 0.0f);
                gr.DrawLine(pen, new PointF(pt.X, 0), new PointF(pt.X, Height));
			}

            for (float val = minVal; val <maxVal; val+=verticalStepping)
            {
                PointF pt = TransformCoord(0, val);
				gr.DrawLine(pen, new PointF(0, pt.Y), new PointF(Width, pt.Y));
			}

            // Zero
            {
				Pen zero_pen = new Pen(new SolidBrush(ZeroColor));
				PointF pt = TransformCoord(0, 0);
                gr.DrawLine(zero_pen, new PointF(0, pt.Y), new PointF(Width, pt.Y));
            }
		}

        private void DrawSignal (Graphics gr)
        {
            Pen pen = new Pen(new SolidBrush(SignalColor));

            PointF prev = TransformCoord(0, data[0]);

            for (int i=1; i<data.Length; i++)
            {
                PointF pos = TransformCoord(i, data[i]);
                gr.DrawLine(pen, prev, pos);
                prev = pos;
            }
        }

        private PointF TransformCoord (int i, float val)
        {
			float x = (float)(i * Width) / data.Length;
			float y = Height - ((val - minVal) * Height) / (maxVal - minVal);
            return new PointF(x, y);
		}

        private void DrawLabels (Graphics gr)
        {
			Brush label_brush = new SolidBrush(LabelsColor);

			int horizontalStepping = data.Length / 10;
			float verticalStepping = (maxVal - minVal) / 20;

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
