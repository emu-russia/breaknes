
namespace Breaknes
{
	public partial class FormBindingFamiController2 : Form
	{
		IOProcessor io;

		public FormBindingFamiController2(IOConfigDevice device, IOProcessor io)
		{
			InitializeComponent();
			this.io = io;
		}
	}
}
