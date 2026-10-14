// PPU Address Mux

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	PAMUX::PAMUX(PPU* parent)
	{
		ppu = parent;
	}

	PAMUX::~PAMUX()
	{
	}

	void PAMUX::sim()
	{
		sim_Control();
	}

	void PAMUX::sim_Control()
	{
		TriState nH2_D = ppu->wire.nH2_Dash;
		TriState BLNK = ppu->fsm.BLNK;
		TriState F_AT = ppu->fsm.FAT;
		TriState DB_PAR = ppu->wire.DB_PAR;

		PARR = NOR(nH2_D, BLNK);
		PAH = NOR(PARR, F_AT);
		PAL = NOR(NOT(PAH), DB_PAR);
	}

	void PAMUX::sim_MuxInputs()
	{
		const auto* tile = ppu->data_reader->tilecnt;
		const unsigned th = tile->get_TH(), tv = tile->get_TV(), fv = tile->get_FV();
		const bool blank = ppu->fsm.BLNK == TriState::One;
		const unsigned common = (unsigned(tile->get_NTH()) << 10) | (unsigned(tile->get_NTV()) << 11) |
			(((fv & 1) != 0 && blank) << 12) | (((fv & 2) != 0 || !blank) << 13);
		AT_ADR = uint16_t(common | 0x3c0 | (th >> 2) | ((tv >> 2) << 3));
		NT_ADR = uint16_t(common | th | (tv << 5));
		PAT_ADR = ppu->data_reader->par->get_address();
	}

	void PAMUX::sim_MuxOutputs()
	{
		pipeline.sim(ppu->wire.PCLK, PARR, ppu->wire.DB_PAR, PAL, PAH, ppu->fsm.FAT,
			AT_ADR, NT_ADR, PAT_ADR, ppu->DB);
	}

	void PAMUX_LowBit::sim(TriState PCLK, TriState PARR, TriState DB_PAR, TriState PAL, TriState F_AT,
		TriState AT_ADR, TriState NT_ADR, TriState PAT_ADR, TriState DB_in,
		TriState& n_PAx)
	{
		auto val = MUX(DB_PAR,
			MUX(PARR,
				MUX(PAL, MUX(F_AT, TriState::Z, AT_ADR), NT_ADR), PAT_ADR), DB_in);
		in_latch.set(val, TriState::One);
		out_latch.set(in_latch.nget(), PCLK);
		n_PAx = out_latch.get();
	}

	void PAMUX_HighBit::sim(TriState PCLK, TriState PARR, TriState PAH, TriState F_AT,
		TriState AT_ADR, TriState NT_ADR, TriState PAT_ADR,
		TriState& n_PAx)
	{
		auto val = MUX(PARR,
			MUX(PAH, MUX(F_AT, TriState::Z, AT_ADR), NT_ADR), PAT_ADR);
		in_latch.set(val, TriState::One);
		out_latch.set(in_latch.nget(), PCLK);
		n_PAx = out_latch.get();
	}
}
