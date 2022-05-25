using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PPUPlayer
{
	public partial class FormColorSpace : Form
	{
		public FormColorSpace()
		{
			InitializeComponent();
		}

		private void FormColorSpace_Load(object sender, EventArgs e)
		{
			pictureBox1.Image = RenderColorSpace();
		}

		private void FormColorSpace_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}

		Bitmap RenderColorSpace()
		{
			int emphasisBands = 8;
			int colorsPerLine = 8;
			int colorLines = 4;
			int colorWidth = 128;
			int colorHeight = 32;

			int w = colorsPerLine * colorWidth;
			int h = emphasisBands * colorLines * colorHeight;

			Bitmap pic = new Bitmap(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

			for (int band=0; band<emphasisBands; band++)
			{
				int chroma_luma = 0;

				for (int row=0; row<colorLines; row++)
				{
					for (int col=0; col<colorsPerLine; col++)
					{
						UInt16 raw = (UInt16)((band << 6) | chroma_luma);
						byte r, g, b;

						PPUPlayerInterop.ConvertRAWToRGB(raw, out r, out g, out b);

						Color p = Color.FromArgb (r, g, b);

						for (int y=0; y<colorHeight; y++)
						{
							for (int x=0; x<colorWidth; x++)
							{
								pic.SetPixel(
									col * colorWidth + x, 
									band * colorLines * colorHeight + row * colorHeight + y, 
									p);
							}
						}

						chroma_luma++;
					}
				}
			}

			return pic;
		}
	}
}
