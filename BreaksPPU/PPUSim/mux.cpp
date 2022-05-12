// Multiplexer

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	void Mux::sim()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_ZCOL0 = ppu->wire.n_ZCOL0;
		TriState n_ZCOL1 = ppu->wire.n_ZCOL1;
		TriState ZCOL2 = ppu->wire.ZCOL2;
		TriState ZCOL3 = ppu->wire.ZCOL3;
		TriState TH_MUX = ppu->wire.TH_MUX;

		step1[0].set(n_ZCOL0, PCLK);
		step1[1].set(n_ZCOL1, PCLK);
		step1[2].set(NOT(ZCOL2), PCLK);
		step1[3].set(NOT(ZCOL3), PCLK);

		sim_MuxControl();

		for (size_t n = 0; n < 4; n++)
		{
			dir_color[n].set(ppu->wire.THO[n], PCLK);
			ppu->wire.n_EXT_Out[n] = NOT(MUX(OCOL, ppu->wire.BGC[n], step1[n].nget()));
			step2[n].set(ppu->wire.n_EXT_Out[n], n_PCLK);
			step3[n].set(MUX(EXT, step2[n].nget(), ppu->wire.EXT_In[n]), PCLK);
			ppu->wire.PAL[n] = NOT(MUX(TH_MUX, step3[n].nget(), dir_color[n].nget()));
		}

		tho4_latch.set(ppu->wire.THO[4], PCLK);
		pal4_latch.set(MUX(TH_MUX, n_PAL4, tho4_latch.nget()), PCLK);
		ppu->wire.PAL[4] = pal4_latch.nget();

		sim_Spr0Hit();
	}

	void Mux::sim_MuxControl()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_ZPRIO = ppu->wire.n_ZPRIO;
		TriState OBJC[4]{};

		for (size_t n = 0; n < 4; n++)
		{
			OBJC[n] = step1[n].nget();
		}

		zprio_latch.set(NOT(n_ZPRIO), PCLK);
		auto bgc01 = NOR(ppu->wire.BGC[0], ppu->wire.BGC[1]);
		auto objc01 = NOR(OBJC[0], OBJC[1]);
		OCOL = AND(NAND(NOT(bgc01), zprio_latch.nget()), NOT(objc01));

		bgc_latch.set(NOT(bgc01), n_PCLK);
		zcol_latch.set(NOT(objc01), n_PCLK);
		EXT = NOR(bgc_latch.get(), zcol_latch.get());

		ocol_latch.set(OCOL, n_PCLK);
		n_PAL4 = ocol_latch.nget();
	}

	void Mux::sim_Spr0Hit()
	{
		TriState n[5]{};
		n[0] = ppu->wire.PCLK;
		n[1] = NOR(ppu->wire.BGC[0], ppu->wire.BGC[1]);
		n[2] = ppu->wire.n_SPR0HIT;
		n[3] = ppu->wire.I2SEV;
		n[4] = ppu->fsm.nVIS;
		TriState STRIKE = NOR5(n);

		StrikeFF.set(NOR(ppu->fsm.RESCL, NOR(StrikeFF.get(), STRIKE)));
		TriState enable = NOT(NOT(NOR(ppu->wire.n_R2, ppu->wire.n_DBE)));
		TriState DB6 = MUX(enable, TriState::Z, StrikeFF.get());
		ppu->SetDBBit(6, DB6);
	}
}
