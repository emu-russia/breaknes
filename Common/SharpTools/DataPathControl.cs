// Control for visualizing the state of the data flows inside the bottom part of the processor.

using SharpTools;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace System.Windows.Forms
{
    public partial class DataPathView : Control
    {
        private BufferedGraphics gfx = null;
        private BufferedGraphicsContext context;
        GraphicsPath [] mapping = new GraphicsPath[(int)CoreDebug.ControlCommand.Max];
        GraphicsPath cpu_write = new GraphicsPath();
        GraphicsPath cpu_read = new GraphicsPath();
        GraphicsPath alu_add = new GraphicsPath();
		CoreDebug.CpuDebugInfo_Commands cur_info = null;
        bool SavedPHI1 = false;
        Pen path_pen = new Pen(new SolidBrush(Color.OrangeRed), 5);
        Font labelFont = new Font("Segoe UI", 10.0f, FontStyle.Bold);
        Brush labelBrush = new SolidBrush(Color.Black);

		public class DataPathGraphModel
		{
			public Dictionary<string, List<int[]>> paths { get; set; } = new();
			public Dictionary<string, List<int[]>> special_paths { get; set; } = new();
			public Dictionary<string, int[]> labels { get; set; } = new();
		}

		private static T ToEnum<T>(string value) where T : struct, Enum
		{
			return (T)Enum.Parse(typeof(T), value);
		}

		public void LoadGraphModel(string json)
		{
			var model = JsonSerializer.Deserialize<DataPathGraphModel>(json);
            if (model == null) return;

			mapping = new GraphicsPath[(int)CoreDebug.ControlCommand.Max];
			cpu_write = new GraphicsPath();
			cpu_read = new GraphicsPath();
			alu_add = new GraphicsPath();

			// Initialize the mapping to highlight the paths of each command.

			foreach (var path in model.paths)
            {
				GraphicsPath gr_path = new GraphicsPath();
                List<Point> points = new();
                foreach(var pair in path.Value)
                {
					points.Add(new Point(pair[0], pair[1]));
                }
                if (points.Count > 0)
                {
                    gr_path.AddLines(points.ToArray());
                }
				mapping[(int)ToEnum<CoreDebug.ControlCommand>(path.Key)] = gr_path;
			}
		}

        public DataPathView()
        {
            SetStyle(ControlStyles.OptimizedDoubleBuffer, true);

            // CPU R/W (for DL/DB)

            cpu_read.AddLines(new Point[] {
                new Point(769, 181),
                new Point(777, 187),
                new Point(783, 181)
            });

            cpu_write.AddLines(new Point[] {
                new Point(769, 157),
                new Point(777, 151),
                new Point(783, 157)
            });

            // ALU Operation -> ADD

            alu_add.AddLines(new Point[] {
                new Point(305, 112),
                new Point(321, 112),
                new Point(317, 108),
                new Point(317, 116),
                new Point(321, 112)
            });
        }

        private void ReallocateGraphics()
        {
            context = BufferedGraphicsManager.Current;
            context.MaximumBuffer = new Size(Width + 1, Height + 1);

            gfx = context.Allocate(CreateGraphics(),
                 new Rectangle(0, 0, Width, Height));
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

        protected override void OnPaint(PaintEventArgs e)
        {
            if (gfx == null)
            {
                ReallocateGraphics();
            }

            long beginTime = DateTime.Now.Ticks;

            DrawScene(gfx.Graphics, Width, Height);

            gfx.Render(e.Graphics);

            long endTime = DateTime.Now.Ticks;
        }

        private void DrawScene(Graphics gr, int width, int height)
        {
            gr.Clear(BackColor);

            DrawImage(gr);
            DrawPaths(gr);
            DrawLabels(gr);
        }

        private void DrawImage(Graphics gr)
        {
            if (BackgroundImage != null)
            {
                float imageWidth = (float)BackgroundImage.Width;
                float imageHeight = (float)BackgroundImage.Height;
                float sx = 0;
                float sy = 0;

                gr.DrawImage(BackgroundImage, sx, sy, imageWidth, imageHeight);
            }
        }

        private void DrawPaths(Graphics gr)
        {
            if (cur_info == null)
                return;

            for (int i=0; i< (int)CoreDebug.ControlCommand.Max; i++)
            {
                if (cur_info.cmd[i] != 0)
                {
                    // ABH/ABL are only set during PHI1, even if commands are active, they are blocked.

                    if ((i == (int)CoreDebug.ControlCommand.ADL_ABL || i == (int)CoreDebug.ControlCommand.ADH_ABH) && !SavedPHI1)
                    {
                        continue;
                    }

                    gr.DrawPath(path_pen, mapping[i]);
                }
            }

            if (cur_info.cmd[(int)CoreDebug.ControlCommand.DL_DB] != 0)
            {
                if (cur_info.WR)
                {
                    gr.DrawPath(path_pen, cpu_write);
                }
                else
                {
                    gr.DrawPath(path_pen, cpu_read);
                }
            }
        }

        private void DrawLabels(Graphics gr)
        {
            if (cur_info == null)
                return;

            Point point = new Point(263, 59);

            // ALU operations are saved only during PHI2

            if (!SavedPHI1)
            {
                bool anyOp = false;

                if (cur_info.cmd[(int)CoreDebug.ControlCommand.ANDS] != 0)
                {
                    gr.DrawString("ANDS", labelFont, labelBrush, point);
                    anyOp = true;
                }

                if (cur_info.cmd[(int)CoreDebug.ControlCommand.EORS] != 0)
                {
                    gr.DrawString("EORS", labelFont, labelBrush, point);
                    anyOp = true;
                }

                if (cur_info.cmd[(int)CoreDebug.ControlCommand.ORS] != 0)
                {
                    gr.DrawString("ORS", labelFont, labelBrush, point);
                    anyOp = true;
                }

                if (cur_info.cmd[(int)CoreDebug.ControlCommand.SRS] != 0)
                {
                    gr.DrawString("SRS", labelFont, labelBrush, point);
                    anyOp = true;
                }

                if (cur_info.cmd[(int)CoreDebug.ControlCommand.SUMS] != 0)
                {
                    gr.DrawString("SUMS", labelFont, labelBrush, point);
                    anyOp = true;
                }

                if (cur_info.n_ACIN == 0 && anyOp)
                {
                    gr.DrawString("+C", labelFont, labelBrush, new Point(300, 70));
                }

                if (anyOp)
                {
                    gr.DrawPath(path_pen, alu_add);
                }
            }

            if (cur_info.n_DAA == 0)
            {
                gr.DrawString("DAA", labelFont, labelBrush, new Point(367, 52));
            }
            if (cur_info.n_DSA == 0)
            {
                gr.DrawString("DSA", labelFont, labelBrush, new Point(367, 72));
            }

            if (cur_info.n_1PC == 0)
            {
                gr.DrawString("+1", labelFont, labelBrush, new Point(635, 64));
            }
        }

        public void ShowCpuCommands(CoreDebug.CpuDebugInfo_Commands info, bool PHI1)
        {
            cur_info = info;
            SavedPHI1 = PHI1;
            Invalidate();
        }

        public void SaveSceneAsImage(string FileName)
        {
            ImageFormat imageFormat;
            string ext;

            if (BackgroundImage == null)
                return;

            int bitmapWidth = BackgroundImage.Width;
            int bitmapHeight = BackgroundImage.Height;

            Bitmap bitmap = new Bitmap(bitmapWidth, bitmapHeight, PixelFormat.Format16bppRgb565);

            Graphics gr = Graphics.FromImage(bitmap);

            DrawScene(gr, bitmapWidth, bitmapHeight);

            ext = Path.GetExtension(FileName);

            if (ext.ToLower() == ".jpg" || ext.ToLower() == ".jpeg")
                imageFormat = ImageFormat.Jpeg;
            else if (ext.ToLower() == ".png")
                imageFormat = ImageFormat.Png;
            else if (ext.ToLower() == ".bmp")
                imageFormat = ImageFormat.Bmp;
            else
                imageFormat = ImageFormat.Jpeg;

            bitmap.Save(FileName, imageFormat);

            bitmap.Dispose();
            gr.Dispose();
        }

    }
}
