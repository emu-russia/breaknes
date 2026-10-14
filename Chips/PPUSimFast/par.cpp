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

	void PAR::sim()
	{
		sim_Control();
		sim_VInv();
		const auto clock = ppu->wire.n_PCLK;
		const auto objectRead = ppu->fsm.OBJ_READ;
		const uint8_t row = rows.sim(clock, O, VINV, PackNibble(ppu->wire.OV));
		if (clock == TriState::One)
			fine = (objectRead & 1) ? row & 7 : uint8_t(~ppu->data_reader->tilecnt->get_FV()) & 7;
		const uint8_t tile = tiles.sim(clock, O, objectRead, ppu->wire.O8_16,
			ppu->PD, ppu->oam->get_OBByte(), row);
		address = uint16_t((~fine & 7) | ((NOT(ppu->wire.nH1_Dash) == TriState::One) << 3) |
			(unsigned(tile) << 4) | ((pad12_latch.nget() == TriState::One) << 12));
	}

	void PAR::sim_Control()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState nF_NT = ppu->fsm.nFNT;
		TriState BGSEL = ppu->wire.BGSEL;
		TriState OBSEL = ppu->wire.OBSEL;
		TriState O8_16 = ppu->wire.O8_16;
		TriState OBJ_READ = ppu->fsm.OBJ_READ;

		fnt_latch.set(NOT(NOR(NOT(H0_DD), nF_NT)), n_PCLK);
		O = NOR(fnt_latch.get(), n_PCLK);
		ob0_latch.set(ppu->wire.OB[0], O);
		pad12_latch.set(MUX(OBJ_READ, BGSEL, MUX(O8_16, OBSEL, ob0_latch.nget())), n_PCLK);

	}

	void PAR::sim_VInv()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_SH2 = ppu->wire.n_OBJ_RD_ATTR;

		VINV_FF.set(MUX(n_PCLK, MUX(NOR(n_PCLK, n_SH2), TriState::Z, ppu->wire.OB[7]), NOT(NOT(VINV_FF.get()))));
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
}
