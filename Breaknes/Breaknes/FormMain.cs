using System.Runtime.InteropServices;

namespace Breaknes
{
    public partial class FormMain : Form
    {
        [DllImport("kernel32")]
        static extern bool AllocConsole();

        public FormMain()
        {
            InitializeComponent();

#if DEBUG
            AllocConsole();
#endif
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormAbout about = new FormAbout();
            about.ShowDialog();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void breaksDebuggerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowBreaksDebugger();
        }

        void ShowBreaksDebugger()
        {
            FormDebug formDebug = new FormDebug();
            formDebug.Show();
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
#if DEBUG
            ShowBreaksDebugger();
#endif
        }
    }
}