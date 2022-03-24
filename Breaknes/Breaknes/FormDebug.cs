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
            //ShowBogusWindow();
            //ShowAssembler();
            //ShowCoreDataPath();
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

        void ShowWiki(string url)
        {
            FormDebugOnlineHelp formDebugOnlineHelp = new FormDebugOnlineHelp(url);
            formDebugOnlineHelp.Show();
        }


        #region "6502 Online Help"

        private void overviewToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/Readme.md");
        }

        private void pinoutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/pads.md");
        }

        private void clockToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/clock.md");
        }

        private void instructionRegisterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/ir.md");
        }

        private void extendedCycleCounterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/extra_counter.md");
        }

        private void decoderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/decoder.md");
        }

        private void predecodeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/predecode.md");
        }

        private void interruptProcessingToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/interrupts.md");
        }

        private void randomLogicToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/random_logic.md");
        }

        private void registersControlToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/regs_control.md");
        }

        private void aLUControlToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/alu_control.md");
        }

        private void programCounterControlToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/pc_control.md");
        }

        private void busControlToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/bus_control.md");
        }

        private void dispatcherToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/dispatch.md");
        }

        private void flagsControlToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/flags_control.md");
        }

        private void flagsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/flags.md");
        }

        private void branchLogicToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/branch_logic.md");
        }

        private void controlCommandsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/context_control.md");
        }

        private void addressBusToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/address_bus.md");
        }

        private void dataBusToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/data_bus.md");
        }

        private void registersToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/regs.md");
        }

        private void aLUToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/alu.md");
        }

        private void programCounterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/6502/pc.md");
        }

        #endregion "6502 Online Help"


        #region "APU Online Help"

        private void overviewToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/Readme.md");
        }

        private void pinoutToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/pads.md");
        }

        private void coreBindingToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/core.md");
        }

        private void timingGeneratorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/clkgen.md");
        }

        private void toneGeneratorsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/sound.md");
        }

        private void lengthCountersToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/length.md");
        }

        private void squareChannelsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/square.md");
        }

        private void triangleChannelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/triangle.md");
        }

        private void noiseChannelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/noise.md");
        }

        private void deltaModulationChannelDPCMToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/dpcm.md");
        }

        private void spriteDMAToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/dma.md");
        }

        private void dACToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/dac.md");
        }

        private void testPatternToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/APU/test_pattern.md");
        }

        #endregion "APU Online Help"


        #region "PPU Online Help"

        private void overviewToolStripMenuItem2_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/Readme.md");
        }

        private void pinoutToolStripMenuItem2_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/pads.md");
        }

        private void pixelClockToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/pclk.md");
        }

        private void controlRegistersToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/regs.md");
        }

        private void hVCountersToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/hv.md");
        }

        private void hVDecoderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/hv_dec.md");
        }

        private void hVFSMControlLogicToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/hv_fsm.md");
        }

        private void paletteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/cram.md");
        }

        private void nTSCVideoSignalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/tv.md");
        }

        private void videoSignalGeneratorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/video_out.md");
        }

        private void spriteComparisonToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/sprite_eval.md");
        }

        private void multiplexerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/mux.md");
        }

        private void spriteMemoryOAMToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/oam.md");
        }

        private void oAMFIFOToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/fifo.md");
        }

        private void dataFetcherToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/dataread.md");
        }

        private void scrollingRegistersToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/scroll_regs.md");
        }

        private void pictureAddressGeneratorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/pargen.md");
        }

        private void backgroundColorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/bgcol.md");
        }

        private void vRAMControllerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/vram_ctrl.md");
        }

        private void pPUInterconnectionsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowWiki("https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/rails.md");
        }

        #endregion "PPU Online Help"

        private void testDebugInfoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormDebugInfo formInfo = new FormDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_Test);
            formInfo.MdiParent = this;
            formInfo.Show();
        }

        private void testMemDumpToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormDebugMemDump memDump = new FormDebugMemDump();
            memDump.MdiParent = this;
            memDump.Show();
        }

    }
}
