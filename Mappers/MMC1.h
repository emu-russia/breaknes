// Generic MMC1 simulation (but based on Sharp MMC1A for now, since it is the only chip studied).

#pragma once

namespace Mappers
{
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
		CIRAM_A10,
		SRAM_CE,
		PRG_nCE,
		Max,
	};

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
		int div_clock_dff = 0;
		int div_reset_dff = 0;
		int div_dff[4]{};		// Addressed individually for simplification
		int prev_div_dff[4]{};		// Previous divider DFFs values
		int cpu_d0_dff = 0;
		int shifter_dffs = 0;	// 5 bit
		int decoder_dffs = 0;	// 2 bit (A13+A14)
		MMC1_Reg reg[4]{};			// All registers are 5 bits each. All bits are latches except bits 2 and 3 for Reg0 (they are DFF+Enable)

		static bool posedge(int prev, int cur);

		// Used to emulate posedge

		int prev_m2 = -1;
		int prev_div_ck = -1;
		int prev_reg0_enable = -1;

	public:
		MMC1();
		~MMC1();

		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[]);
	};
}
