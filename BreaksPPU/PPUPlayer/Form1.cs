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
    public partial class Form1 : Form
    {
        string? ppu_dump;
        string? nes_file;

        public Form1()
        {
            InitializeComponent();
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

        private void choosePPURegsDumpToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                ppu_dump = openFileDialog1.FileName;
            }
        }

        private void choosenesImageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (openFileDialog2.ShowDialog() == DialogResult.OK)
            {
                nes_file = openFileDialog2.FileName;
            }
        }

        private void runToolStripMenuItem_Click(object sender, EventArgs e)
        {
            RunPPU();
        }

        private void toolStripButton1_Click(object sender, EventArgs e)
        {
            RunPPU();
        }

        void RunPPU()
        {
            if (ppu_dump == null || nes_file == null)
            {
                MessageBox.Show(
                    "Before you start the simulation you need to select a PPU register dump and some .nes file, preferably with mapper 0.", 
                    "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }

            // ...
        }
    }
}
