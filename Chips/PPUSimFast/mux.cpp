// Multiplexer

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	template<int Phase> void Mux::sim(const EdgeContextT<Phase>& c)
	{
		const bool high = c.PCLK == TriState::One;
		const uint8_t background = ppu->data_reader->bgcol->get_Pixel();
		if (high)
		{
			step1 = uint8_t(c.n_ZCOL0 | (c.n_ZCOL1 << 1) |
				((c.ZCOL2 ^ 1) << 2) | ((c.ZCOL3 ^ 1) << 3));
			dir_color = ppu->data_reader->tilecnt->get_TH() & 15;
			zprio_latch.set(NOT(c.n_ZPRIO), TriState::One);
		}
		const uint8_t object = uint8_t(~step1) & 15;
		const bool bg_opaque = (background & 3) != 0, obj_opaque = (object & 3) != 0;
		const bool choose_object = obj_opaque && (!bg_opaque || zprio_latch.get() == TriState::One);
		OCOL = choose_object ? TriState::One : TriState::Zero;
		if (!high)
		{
			bgc_latch.set(bg_opaque ? TriState::One : TriState::Zero, TriState::One);
			zcol_latch.set(obj_opaque ? TriState::One : TriState::Zero, TriState::One);
			ocol_latch.set(OCOL, TriState::One);
		}
		EXT = NOR(bgc_latch.get(), zcol_latch.get());
		n_PAL4 = ocol_latch.nget();
		const uint8_t selected = choose_object ? object : background;
		const uint8_t n_ext = uint8_t(~selected) & 15;
		if (!high) step2 = n_ext;
		if (high) step3 = EXT == TriState::One ? PackNibble(ppu->wire.EXT_In) : uint8_t(~step2) & 15;
		const uint8_t color = c.TH_MUX == TriState::One ? dir_color : step3;
		if (high)
		{
			tho4_latch.set(static_cast<TriState>((ppu->data_reader->tilecnt->get_TH() >> 4) & 1), TriState::One);
			pal4_latch.set(c.TH_MUX == TriState::One ? tho4_latch.nget() : n_PAL4, TriState::One);
		}
		palette_address = uint8_t(color | ((pal4_latch.nget() == TriState::One ? 1U : 0U) << 4));
		for (unsigned n = 0; n < 4; n++) ppu->wire.n_EXT_Out[n] = static_cast<TriState>((n_ext >> n) & 1);
		sim_Spr0Hit(c);
	}

	template<int Phase> void Mux::sim_Spr0Hit(const EdgeContextT<Phase>& c)
	{
		TriState n[5]{};
		n[0] = c.PCLK;
		n[1] = (ppu->data_reader->bgcol->get_Pixel() & 3) == 0 ? TriState::One : TriState::Zero;
		n[2] = c.n_SPR0HIT;
		n[3] = c.n_SPR0_EV;
		n[4] = c.nVIS;
		TriState STRIKE = NOR5(n);

		StrikeFF.set(NOR(c.RESCL, NOR(StrikeFF.get(), STRIKE)));
		TriState enable = NOT(NOT(NOR(ppu->wire.n_R2, ppu->wire.n_DBE)));
		TriState DB6 = MUX(enable, TriState::Z, StrikeFF.get());
		ppu->SetDBBit(6, DB6);
	}

	template void Mux::sim<0>(const EdgeContextT<0>&);
	template void Mux::sim<1>(const EdgeContextT<1>&);
}
