// Pixel Clock

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	void PPU::sim_PCLK()
	{
		TriState CLK = wire.CLK;
		TriState n_CLK = wire.n_CLK;
		TriState prev_pclk{};
		TriState new_pclk{};

		// TBD: Add support for other PPU revisions.

		switch (rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
			case Revision::RP2C04_0003:
			{
				pclk_1.set(NOR(wire.RES, pclk_4.nget()), n_CLK);
				pclk_2.set(pclk_1.nget(), CLK);
				pclk_3.set(pclk_2.nget(), n_CLK);
				pclk_4.set(pclk_3.nget(), CLK);

				prev_pclk = wire.PCLK;
				new_pclk = pclk_4.nget();

				wire.PCLK = new_pclk;
				wire.n_PCLK = NOT(wire.PCLK);
			}
			break;
		}

		// The software PCLK counter is triggered by the falling edge.
		// This is purely a software design for convenience, and has nothing to do with PPU hardware circuitry.

		if (prev_pclk == TriState::One && new_pclk == TriState::Zero)
		{
			pclk_counter++;
		}
	}
}
