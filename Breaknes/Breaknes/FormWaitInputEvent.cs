using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Breaknes
{
	public partial class FormWaitInputEvent : Form
	{
		public string? bindstr = "Dummy binding";
		public int saved_actuator_id;

		public FormWaitInputEvent(int actuator_id)
		{
			InitializeComponent();
			saved_actuator_id = actuator_id;
		}
	}
}
