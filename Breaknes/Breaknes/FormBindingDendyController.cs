
namespace Breaknes
{
	public partial class FormBindingDendyController : Form
	{
		IOProcessor io;

		public FormBindingDendyController(IOConfigDevice device, IOProcessor io)
		{
			InitializeComponent();
			this.io = io;
		}
	}
}
