#pragma once

namespace APUSim
{
	class APU;
}

// An external class that has access to all internals. Use for unit testing.

namespace APUSimUnitTest
{
	class UnitTest;
}

#include "clkgen.h"
#include "core.h"
#include "dma.h"
#include "dpcm.h"
#include "length.h"
#include "noise.h"
#include "square.h"
#include "triangle.h"
#include "regs.h"
#include "debug.h"

namespace APUSim
{
	/// <summary>
	/// All known revisions of official APUs and compatible clones.
	/// </summary>
	enum class Revision
	{
		Unknown = 0,
		RP2A03,
		RP2A07,
		Max,
	};

	enum class APU_Input
	{
		CLK = 0,
		n_NMI,
		n_IRQ,
		n_RES,
		DBG,
		Max,
	};

	enum class APU_Output
	{
		n_IN0 = 0,
		n_IN1,
		OUT_0,
		OUT_1,
		OUT_2,
		M2,
		RnW,
		Max,
	};

	class APU
	{
		friend APUSimUnitTest::UnitTest;
		friend DMACounterBit;
		friend DMA;
		friend RegsDecoder;

		/// <summary>
		/// Internal auxiliary and intermediate connections.
		/// </summary>
		struct InternalWires
		{
			BaseLogic::TriState PHI0;
			BaseLogic::TriState PHI1;
			BaseLogic::TriState PHI2;
			BaseLogic::TriState RDY;
			BaseLogic::TriState ACLK;
			BaseLogic::TriState n_ACLK;
			BaseLogic::TriState RES;
			BaseLogic::TriState RUNDMC;
			BaseLogic::TriState DMCReady;
			BaseLogic::TriState SPRE;
			BaseLogic::TriState SPRS;
			BaseLogic::TriState RnW;			// From core
			BaseLogic::TriState SPR_CPU;
			BaseLogic::TriState SPR_PPU;
			BaseLogic::TriState RW;				// To pad
			BaseLogic::TriState n_DMC_AB;
			
			// RegOps
			BaseLogic::TriState n_R4015;
			BaseLogic::TriState n_R4016;
			BaseLogic::TriState n_R4017;
			BaseLogic::TriState n_R4018;
			BaseLogic::TriState n_R4019;
			BaseLogic::TriState n_R401A;
			BaseLogic::TriState W4000;
			BaseLogic::TriState W4001;
			BaseLogic::TriState W4002;
			BaseLogic::TriState W4003;
			BaseLogic::TriState W4004;
			BaseLogic::TriState W4005;
			BaseLogic::TriState W4006;
			BaseLogic::TriState W4007;
			BaseLogic::TriState W4008;
			BaseLogic::TriState W400A;
			BaseLogic::TriState W400B;
			BaseLogic::TriState W400C;
			BaseLogic::TriState W400E;
			BaseLogic::TriState W400F;
			BaseLogic::TriState W4010;
			BaseLogic::TriState W4011;
			BaseLogic::TriState W4012;
			BaseLogic::TriState W4013;
			BaseLogic::TriState W4014;
			BaseLogic::TriState W4015;
			BaseLogic::TriState W4016;
			BaseLogic::TriState W4017;
			BaseLogic::TriState W401A;

			// Auxiliary signals associated with the Test mode, which seems to be present only on 2A03.
			BaseLogic::TriState DBG;			// from pad
			BaseLogic::TriState n_DBGRD;		// from regs decoder
		} wire{};

		Revision rev = Revision::Unknown;
		M6502Core::M6502* core = nullptr;

		DMA* dma = nullptr;
		RegsDecoder* regs = nullptr;

		// Internal buses.

		uint8_t DB = 0;
		bool DB_Dirty = false;

		uint16_t DMC_Addr;
		uint16_t SPR_Addr;
		uint16_t CPU_Addr;			// Core to mux & regs predecoder
		uint16_t Ax;			// Mux to pads & regs decoder

	public:
		APU(M6502Core::M6502 *core, Revision rev);
		~APU();

		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint8_t *data, uint16_t* addr, float *AUX_A, float* AUX_B);

		void GetDebugInfo_Wires(APU_Interconnects& wires);

		void GetDebugInfo_Regs(APU_Registers& regs);
	};
}
