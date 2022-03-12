#pragma once

namespace APUSim
{
	class APU;
}

#include "clkgen.h"
#include "core.h"
#include "dma.h"
#include "dpcm.h"
#include "length.h"
#include "noise.h"
#include "square.h"
#include "triangle.h"

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
		Revision rev = Revision::Unknown;
		M6502Core::M6502* core = nullptr;

	public:
		APU(M6502Core::M6502 *core, Revision rev);
		~APU();

		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint8_t *data, uint16_t* addr, float *AUX_A, float* AUX_B);
	};
}
