namespace BreaksDebug
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
	}
}
