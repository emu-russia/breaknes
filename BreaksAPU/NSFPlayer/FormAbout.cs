using System.ComponentModel;
using System.Drawing.Drawing2D;

namespace APUPlayer
{
	public partial class FormAbout : Form
	{
		public FormAbout()
		{
			InitializeComponent();
		}

		private void FormAbout_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}

		private void button1_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void pictureBox1_Click(object sender, EventArgs e)
		{
			if (!backgroundWorker1.IsBusy)
			{
				backgroundWorker1.RunWorkerAsync();
			}
		}

		private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
		{
			while (!backgroundWorker1.CancellationPending)
			{
				pictureBox1.Image = RotateImage(pictureBox1.Image, 2.0F);

				Thread.Sleep(50);
			}
		}

		// https://stackoverflow.com/questions/2163829/how-do-i-rotate-a-picture-in-winforms

		Image RotateImage(Image img, float rotationAngle)
		{
			//create an empty Bitmap image
			Bitmap bmp = new Bitmap(img.Width, img.Height);

			//turn the Bitmap into a Graphics object
			Graphics gfx = Graphics.FromImage(bmp);

			//now we set the rotation point to the center of our image
			gfx.TranslateTransform((float)bmp.Width / 2, (float)bmp.Height / 2);

			//now rotate the image
			gfx.RotateTransform(rotationAngle);

			gfx.TranslateTransform(-(float)bmp.Width / 2, -(float)bmp.Height / 2);

			//set the InterpolationMode to HighQualityBicubic so to ensure a high
			//quality image once it is transformed to the specified size
			gfx.InterpolationMode = InterpolationMode.HighQualityBicubic;

			//now draw our new image onto the graphics object
			gfx.DrawImage(img, new Point(0, 0));

			//dispose of our Graphics object
			gfx.Dispose();

			//return the image
			return bmp;
		}

	}
}
