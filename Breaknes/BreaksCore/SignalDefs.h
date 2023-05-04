
#pragma once

#define CORE_BRK_CATEGORY "Core BRK"
#define CORE_DISP_CATEGORY "Core Dispatcher"
#define CORE_ALU_CATEGORY "Core ALU Related"
#define CORE_BOPS_CATEGORY "Core Bops"
#define CORE_FOPS_CATEGORY "Core Fops"

#define CORE_REGS_CATEGORY "Core Regs"

#define APU_WIRES_CATEGORY "APU Wires"
#define APU_REGS_CATEGORY "APU Regs"

#define CRAM_NAME "Color RAM"
#define OAM_NAME "OAM"
#define OAM2_NAME "Temp OAM"

#define PPU_CLKS_CATEGORY "PPU Clocks"
#define PPU_CPU_CATEGORY "PPU CPU I/F"
#define PPU_CTRL_CATEGORY "PPU CTRL"
#define PPU_HV_CATEGORY "PPU H/V"
#define PPU_MUX_CATEGORY "PPU MUX"
#define PPU_SPG_CATEGORY "PPU DataReader"	// aka Still Picture Generator
#define PPU_CRAM_CATEGORY "PPU CRAM"
#define PPU_VRAM_CATEGORY "PPU VRAM"
#define PPU_FSM_CATEGORY "PPU FSM"
#define PPU_EVAL_CATEGORY "PPU Eval"
#define PPU_WIRES_CATEGORY "PPU Wires"		// Uncategorized PPU tentacles
#define PPU_REGS_CATEGORY "PPU Regs"

#define BOARD_CATEGORY "Board"

namespace Breaknes
{
	struct SignalOffsetPair
	{
		const char* name;
		const size_t offset;
		const uint8_t bits;
	};

	extern SignalOffsetPair core_brk_wires[];
	extern SignalOffsetPair core_disp_wires[];
	extern SignalOffsetPair core_alu_wires[];
	extern SignalOffsetPair core_bops_wires[];
	extern SignalOffsetPair core_fops_wires[];
	extern SignalOffsetPair core_regs[];
	
	extern SignalOffsetPair apu_wires[];
	extern SignalOffsetPair apu_regs[];
	
	extern SignalOffsetPair ppu_clks_signals[];
	extern SignalOffsetPair ppu_cpu_signals[];
	extern SignalOffsetPair ppu_ctrl_signals[];
	extern SignalOffsetPair ppu_hv_signals[];
	extern SignalOffsetPair ppu_mux_signals[];
	extern SignalOffsetPair ppu_spg_signals[];
	extern SignalOffsetPair ppu_cram_signals[];
	extern SignalOffsetPair ppu_vram_signals[];
	extern SignalOffsetPair ppu_fsm_signals[];
	extern SignalOffsetPair ppu_eval_signals[];
	extern SignalOffsetPair ppu_wires[];
	extern SignalOffsetPair ppu_regs[];

	extern size_t core_brk_wires_count;
	extern size_t core_disp_wires_count;
	extern size_t core_alu_wires_count;
	extern size_t core_bops_wires_count;
	extern size_t core_fops_wires_count;
	extern size_t core_regs_count;
	
	extern size_t apu_wires_count;
	extern size_t apu_regs_count;

	extern size_t ppu_clks_signals_count;
	extern size_t ppu_cpu_signals_count;
	extern size_t ppu_ctrl_signals_count;
	extern size_t ppu_hv_signals_count;
	extern size_t ppu_mux_signals_count;
	extern size_t ppu_spg_signals_count;
	extern size_t ppu_cram_signals_count;
	extern size_t ppu_vram_signals_count;
	extern size_t ppu_fsm_signals_count;
	extern size_t ppu_eval_signals_count;
	extern size_t ppu_wires_count;
	extern size_t ppu_regs_count;
};
