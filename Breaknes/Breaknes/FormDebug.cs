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
    public partial class FormDebug : Form
    {
        public FormDebug()
        {
            InitializeComponent();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void FormDebug_Load(object sender, EventArgs e)
        {
            ShowBogusWindow();
            ShowAssembler();
            ShowCoreDataPath();
        }

        void ShowBogusWindow()
        {
            FormDebugBogusWindow bogusWindow = new FormDebugBogusWindow();
            bogusWindow.MdiParent = this;
            bogusWindow.Show();
        }

        void ShowAssembler()
        {
            FormDebugAssembler formDebugAssembler = new FormDebugAssembler();
            formDebugAssembler.MdiParent = this;
            formDebugAssembler.Show();
        }

        void ShowCoreDataPath()
        {
            FormDebugCoreDataPath formDebugCoreDataPath = new FormDebugCoreDataPath();
            formDebugCoreDataPath.MdiParent = this;
            formDebugCoreDataPath.Show();
        }

        private void bogusWindowToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowBogusWindow();
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //...
        }

        private void assemblerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowAssembler();
        }

        private void coreDataPathToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowCoreDataPath();
        }


    }
}
