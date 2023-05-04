
namespace NSFPlayer
{
	public partial class FormNSFInfo : Form
	{
		public FormNSFInfo(NSFHeader? head)
		{
			InitializeComponent();

			if (head != null)
			{
				propertyGrid1.SelectedObject = head;
			}
		}

		private void FormNSFInfo_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Escape)
			{
				Close();
			}
		}
	}
}
