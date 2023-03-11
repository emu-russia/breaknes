
#pragma once

namespace Breaknes
{
	struct SignalOffsetPair
	{
		const char* name;
		const size_t offset;
		const uint8_t bits;
	};

	extern SignalOffsetPair core_wires[];
	extern SignalOffsetPair core_regs[];
	extern SignalOffsetPair apu_wires[];
	extern SignalOffsetPair apu_regs[];
	extern SignalOffsetPair ppu_wires[];
	extern SignalOffsetPair fsm_signals[];
	extern SignalOffsetPair eval_signals[];
	extern SignalOffsetPair ppu_regs[];

	extern size_t core_wires_count;
	extern size_t core_regs_count;
	extern size_t apu_wires_count;
	extern size_t apu_regs_count;
	extern size_t ppu_wires_count;
	extern size_t fsm_signals_count;
	extern size_t eval_signals_count;
	extern size_t ppu_regs_count;
};
