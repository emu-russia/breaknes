// Pixel Clock

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	void PPU::sim_PCLK()
	{
		TriState CLK = wire.CLK;
		TriState n_CLK = wire.n_CLK;
		TriState prev_pclk = wire.PCLK;
		TriState new_pclk{};

		// TBD: Add support for other PPU revisions.

		// In the original circuits there is also a single phase splitter, based on FF, but we are not simulating it here.

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

				new_pclk = pclk_4.nget();
			}
			break;

			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
			{
				pclk_1.set(NOR(pclk_6.get(), wire.RES), CLK);
				pclk_2.set(pclk_1.nget(), n_CLK);
				pclk_3.set(pclk_2.nget(), CLK);
				pclk_4.set(pclk_3.nget(), n_CLK);
				pclk_5.set(pclk_4.nget(), CLK);
				pclk_6.set(NOR(pclk_3.nget(), pclk_5.nget()), n_CLK);

				new_pclk = NOR(pclk_5.nget(), NOT(pclk_4.nget()));
			}
			break;
		}

		wire.PCLK = new_pclk;
		wire.n_PCLK = NOT(wire.PCLK);

		// The software PCLK counter is triggered by the falling edge.
		// This is purely a software design for convenience, and has nothing to do with PPU hardware circuitry.

		if (prev_pclk == TriState::One && new_pclk == TriState::Zero)
		{
			pclk_counter++;
		}
	}
}
