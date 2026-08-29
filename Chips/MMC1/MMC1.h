// Generic MMC1 simulation (but based on Sharp MMC1A for now, since it is the only chip studied).
// Moved from Mappers to Chips (issue #509): a chip is simulated here, the board wiring lives in CartPcb.

#pragma once

#include <cstdint>
#include <cstddef>

#include "../../Common/BaseLogicLib/BaseLogic.h"

namespace Chips
{
	/// <summary>
	/// Log categories of the MMC1 (one bit per category, owned by this component).
	/// </summary>
	enum LogCategory : uint64_t
	{
		Cat_Regs = 1ULL << 0,		// serial register write commits
		Cat_Events = 1ULL << 1,		// reset (D7) events
	};

	/// <summary>
	/// The category list of the MMC1, used by BreaksCore to register the source
	/// for the user interface (definitions flow Component -> BreaksCore).
	/// </summary>
	const Log::LogCategoryDesc* GetLogCategories(size_t& count);

	enum class MMC1_Input
	{
		M2 = 0,
		CPU_RnW,
		CPU_A13,
		CPU_A14,
		CPU_D0,
		CPU_D7,
		PPU_A10,
		PPU_A11,
		PPU_A12,
		nROMSEL,
		Max,
	};

	enum class MMC1_Output
	{
		CHR_A12 = 0,
		CHR_A13,
		CHR_A14,
		CHR_A15,
		CHR_A16,
		PRG_A14,
		PRG_A15,
		PRG_A16,
		PRG_A17,
		VRAM_A10,
		SRAM_CE,
		PRG_nCE,
		Max,
	};

	// Pin names in enum order (used by the CartPcb netlist).
	static const char* const MMC1_InputPinNames[] = {
		"M2", "CPU_RnW", "CPU_A13", "CPU_A14", "CPU_D0", "CPU_D7",
		"PPU_A10", "PPU_A11", "PPU_A12", "nROMSEL"
	};

	static const char* const MMC1_OutputPinNames[] = {
		"CHR_A12", "CHR_A13", "CHR_A14", "CHR_A15", "CHR_A16",
		"PRG_A14", "PRG_A15", "PRG_A16", "PRG_A17",
		"VRAM_A10", "SRAM_CE", "PRG_nCE"
	};

	static_assert(sizeof(MMC1_InputPinNames) / sizeof(MMC1_InputPinNames[0]) == (size_t)MMC1_Input::Max, "MMC1 input pin count mismatch");
	static_assert(sizeof(MMC1_OutputPinNames) / sizeof(MMC1_OutputPinNames[0]) == (size_t)MMC1_Output::Max, "MMC1 output pin count mismatch");

	union MMC1_Reg
	{
		struct
		{
			unsigned b0 : 1;
			unsigned b1 : 1;
			unsigned b2 : 1;
			unsigned b3 : 1;
			unsigned b4 : 1;
		};
		int bitval;
	};

	class MMC1
	{
	private:
		int div_clock_dff = 0;
		int div_reset_dff = 0;
		int d7_settled = 0;
		int div_dff[4]{};		// Addressed individually for simplification
		int prev_div_dff[4]{};		// Previous divider DFFs values
		int cpu_d0_dff = 0;
		int shifter_dffs = 0;	// 5 bit
		int decoder_dffs = 0;	// 2 bit (A13+A14)
		MMC1_Reg reg[4]{};			// All registers are 5 bits each. All bits are latches except bits 2 and 3 for Reg0 (they are DFFs with async set on the D7 reset write).

		static bool posedge(int prev, int cur);

		// Used to emulate posedge

		int prev_m2 = -1;
		int prev_div_ck = -1;
		int prev_reg0_enable = -1;
		int prev_reset_write = 0;	// D7 reset write edge detection for logging

	public:
		MMC1();
		~MMC1();

		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[]);

		/// <summary>
		/// Compute the PRG ROM address for the given CPU bus address using the current
		/// register state, without simulating the chip.
		/// </summary>
		size_t Dbg_GetPRGAddress(size_t cpu_addr);

		/// <summary>
		/// Set the log category mask of the MMC1 (one bit per Chips::LogCategory).
		/// The mask is stored in the global Log manager, so it can be set at any time,
		/// even before the MMC1 instance exists.
		/// </summary>
		void SetLogMask(uint64_t mask);
	};
}
