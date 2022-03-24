using System.Runtime.InteropServices;

namespace Breaknes
{
    public partial class FormMain : Form
    {
        [DllImport("kernel32")]
        static extern bool AllocConsole();

        DebugEventsHub debugEventsHub = new DebugEventsHub();
        int debugInstanceCounter = 0;

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
            formDebug.FormClosed += FormDebug_FormClosed;
            formDebug.OnStep += Step;
            formDebug.Show();
            debugEventsHub.AddListener(formDebug.DebugEventHandler);
            debugInstanceCounter++;
        }

        private void FormDebug_FormClosed(object? sender, FormClosedEventArgs e)
        {
            FormDebug formDebug = (FormDebug)sender;
            debugEventsHub.RemoveListener(formDebug.DebugEventHandler);
            debugInstanceCounter--;
        }

        bool DebugActive()
        {
            return debugInstanceCounter != 0;
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
#if DEBUG
            ShowBreaksDebugger();
#endif
        }

        void LoadRom(string filename)
        {
            debugEventsHub.SignalDebugEvent(DebugEvent.LoadRom);

            // If the debugger is inactive - run the simulation in a loop.

            if (!DebugActive())
            {

            }
        }

        private void loadROMDumpToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                string filename = openFileDialog1.FileName;
                LoadRom(filename);
            }
        }

        private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormSettings settings = new FormSettings();
            settings.FormClosed += Settings_FormClosed;
            settings.ShowDialog();
        }

        private void Settings_FormClosed(object? sender, FormClosedEventArgs e)
        {
            FormSettings settings = (FormSettings)sender;

            // If the motherboard has changed - stop the simulation and recreate the board instance.

            debugEventsHub.SignalDebugEvent(DebugEvent.BoardChanged);
        }

        void Step()
        {

            debugEventsHub.SignalDebugEvent(DebugEvent.Step);
        }
    }
}
