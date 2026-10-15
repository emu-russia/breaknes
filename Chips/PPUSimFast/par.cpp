// Picture Address Register

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	PAR::PAR(PPU* parent)
	{
		ppu = parent;
	}

	PAR::~PAR()
	{
	}

	template<int Phase> void PAR::sim(const EdgeContextT<Phase>& c)
	{
		sim_Control(c);
		sim_VInv(c);
		const auto clock = c.n_PCLK;
		const auto objectRead = c.OBJ_READ;
		const uint8_t row = rows.sim(clock, O, VINV, ppu->eval->get_Row() & 15);
		if (clock == TriState::One)
			fine = (objectRead & 1) ? row & 7 : uint8_t(~ppu->data_reader->tilecnt->get_FV()) & 7;
		const uint8_t tile = tiles.sim(clock, O, objectRead, ppu->wire.O8_16,
			ppu->PD, ppu->oam->get_OBByte(), row);
		address = uint16_t((~fine & 7) | ((NOT(c.nH1_Dash) == TriState::One) << 3) |
			(unsigned(tile) << 4) | ((pad12_latch.nget() == TriState::One) << 12));
	}

	template<int Phase> void PAR::sim_Control(const EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;
		TriState H0_DD = c.H0_Dash2;
		TriState nF_NT = c.nFNT;
		TriState BGSEL = ppu->wire.BGSEL;
		TriState OBSEL = ppu->wire.OBSEL;
		TriState O8_16 = ppu->wire.O8_16;
		TriState OBJ_READ = c.OBJ_READ;

		fnt_latch.set(NOT(NOR(NOT(H0_DD), nF_NT)), n_PCLK);
		O = NOR(fnt_latch.get(), n_PCLK);
		ob0_latch.set(static_cast<TriState>(ppu->oam->get_OBByte() & 1), O);
		pad12_latch.set(MUX(OBJ_READ, BGSEL, MUX(O8_16, OBSEL, ob0_latch.nget())), n_PCLK);

	}

	template<int Phase> void PAR::sim_VInv(const EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;
		TriState n_SH2 = c.n_OBJ_RD_ATTR;

		VINV_FF.set(MUX(n_PCLK, MUX(NOR(n_PCLK, n_SH2), TriState::Z, static_cast<TriState>((ppu->oam->get_OBByte() >> 7) & 1)), NOT(NOT(VINV_FF.get()))));
		VINV = NOT(NOT(VINV_FF.get()));
	}

	void ParBitInv::sim(TriState n_PCLK, TriState O, TriState INV, TriState val_in,
		TriState& val_out)
	{
		in_latch.set(val_in, n_PCLK);
		out_latch.set(in_latch.nget(), O);
		val_out = NOT(MUX(INV, out_latch.nget(), NOT(out_latch.nget())));
	}

	void ParBit::sim(TriState n_PCLK, TriState O, TriState val_OB, TriState val_PD, TriState OBJ_READ,
		TriState& PADx)
	{
		pdin_latch.set(NOT(val_PD), n_PCLK);
		pdout_latch.set(pdin_latch.nget(), O);
		ob_latch.set(val_OB, O);
		padx_latch.set(MUX(OBJ_READ, pdout_latch.nget(), ob_latch.nget()), n_PCLK);
		PADx = padx_latch.nget();
	}

	template void PAR::sim<0>(const EdgeContextT<0>&);
	template void PAR::sim<1>(const EdgeContextT<1>&);
}
