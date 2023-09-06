namespace InfernoColormap
{
	public partial class FormMain : Form
	{
		public FormMain()
		{
			InitializeComponent();
		}

		private void Form1_Load(object sender, EventArgs e)
		{
			pictureBox1.Image = GenerateInfernoColormap();
		}

		private Bitmap GenerateInfernoColormap ()
		{
			Bitmap bmp = new Bitmap(256, 256);

			for (int y = 0; y < 256; y++)
			{
				for (int x = 0; x < 256; x++)
				{
					bmp.SetPixel(x, y, InfernoColormap.GetRGB ((byte)y));
				}
			}

			return bmp;
		}
	}
}
