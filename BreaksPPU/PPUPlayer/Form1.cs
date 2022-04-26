using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.IO;
using System.Runtime.InteropServices;

namespace PPUPlayer
{
    public partial class Form1 : Form
    {
        [DllImport("kernel32")]
        static extern bool AllocConsole();

        string? ppu_dump;
        string? nes_file;

        int logPointer = 0;
        byte[] logData = new byte[0];
        PPULogEntry? currentEntry;
        int recordCounter = 0;

        public Form1()
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

        private void choosePPURegsDumpToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                ppu_dump = openFileDialog1.FileName;

                Console.WriteLine("The PPU registers dump is selected: " + ppu_dump);
            }
        }

        private void choosenesImageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (openFileDialog2.ShowDialog() == DialogResult.OK)
            {
                nes_file = openFileDialog2.FileName;

                Console.WriteLine("The .nes file has been selected: " + nes_file);
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

        private void Form1_Load(object sender, EventArgs e)
        {
            pictureBox1.BackColor = Color.Gray;
        }

        private void stopPPUAndUnloadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            StopPPU();
        }

        private void toolStripButton2_Click(object sender, EventArgs e)
        {
            StopPPU();
        }

        void RunPPU()
        {
            if (backgroundWorker1.IsBusy)
            {
                Console.WriteLine("Background Worker is already running.");
                return;
            }    

            if (ppu_dump == null || nes_file == null)
            {
                MessageBox.Show(
                    "Before you start the simulation you need to select a PPU register dump and some .nes file, preferably with mapper 0.",
                    "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            logData = File.ReadAllBytes(ppu_dump);
            logPointer = 0;
            recordCounter = 0;
            Console.WriteLine("Number of PPU Dump records: " + (logData.Length / 4).ToString());

            byte[] nes = File.ReadAllBytes(nes_file);

            PPUPlayerInterop.CreateBoard();
            PPUPlayerInterop.InsertCartridge(nes, nes.Length);

            currentEntry = NextLogEntry();

            if (currentEntry != null)
            {
                backgroundWorker1.RunWorkerAsync();
            }
            else
            {
                MessageBox.Show(
                    "The trace history of PPU register accesses does not contain any data.",
                    "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        void StopPPU()
        {
            if (backgroundWorker1.IsBusy)
            {
                backgroundWorker1.CancelAsync();
                DisposeBoard();
            }
            else
            {
                Console.WriteLine("Background Worker is not running or has already completed its activity.");
            }
        }

        void DisposeBoard()
        {
            PPUPlayerInterop.EjectCartridge();
            PPUPlayerInterop.DestroyBoard();
            ppu_dump = null;
            nes_file = null;
        }

        PPULogEntry? NextLogEntry()
        {
            PPULogEntry entry = new PPULogEntry();

            var bytesLeft = logData.Length - logPointer;
            if (bytesLeft < 4)
            {
                return null;
            }

            UInt16 pclkDelta = logData[logPointer + 1];
            pclkDelta <<= 8;
            pclkDelta |= logData[logPointer + 0];

            entry.pclk = PPUPlayerInterop.GetPCLKCounter() + pclkDelta;
            entry.write = (logData[logPointer + 2] & 0x80) == 0 ? true : false;
            entry.reg = (byte)(logData[logPointer + 2] & 0x7);
            entry.value = logData[logPointer + 3];

            logPointer += 4;

            return entry;
        }

        public class PPULogEntry
        {
            public int pclk;
            public bool write;
            public byte reg;
            public byte value;
        }

        private void backgroundWorker1_DoWork_1(object sender, DoWorkEventArgs e)
        {
            while (!backgroundWorker1.CancellationPending)
            {
                if (currentEntry == null)
                {
                    Console.WriteLine("PPU Dump records are out.");

                    DisposeBoard();

                    return;
                }

                if (PPUPlayerInterop.GetPCLKCounter() == currentEntry.pclk)
                {
                    if (currentEntry.write)
                    {
                        PPUPlayerInterop.CPUWrite(currentEntry.reg, currentEntry.value);
                    }
                    else
                    {
                        PPUPlayerInterop.CPURead(currentEntry.reg);
                    }

                    currentEntry = NextLogEntry();

                    recordCounter++;
                    if (recordCounter > 10000)
                    {
                        recordCounter = 0;
                        Console.WriteLine("Another 10000\n");
                    }
                }

                PPUPlayerInterop.Step();

                float sample;
                PPUPlayerInterop.SampleVideoSignal(out sample);
            }

            Console.WriteLine("Background Worker canceled.");
        }
    }
}
