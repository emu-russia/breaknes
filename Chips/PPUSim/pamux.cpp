// PPU Address Mux

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
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
		TriState BLNK = ppu->fsm.BLNK;

		AT_ADR[0] = ppu->wire.THO[2];
		AT_ADR[1] = ppu->wire.THO[3];
		AT_ADR[2] = ppu->wire.THO[4];
		AT_ADR[3] = ppu->wire.TVO[2];
		AT_ADR[4] = ppu->wire.TVO[3];
		AT_ADR[5] = ppu->wire.TVO[4];
		AT_ADR[6] = TriState::One;
		AT_ADR[7] = TriState::One;
		AT_ADR[8] = TriState::One;
		AT_ADR[9] = TriState::One;

		NT_ADR[0] = ppu->wire.THO[0];
		NT_ADR[1] = ppu->wire.THO[1];
		NT_ADR[2] = ppu->wire.THO[2];
		NT_ADR[3] = ppu->wire.THO[3];
		NT_ADR[4] = ppu->wire.THO[4];
		NT_ADR[5] = ppu->wire.TVO[0];
		NT_ADR[6] = ppu->wire.TVO[1];
		NT_ADR[7] = ppu->wire.TVO[2];
		NT_ADR[8] = ppu->wire.TVO[3];
		NT_ADR[9] = ppu->wire.TVO[4];

		AT_ADR[10] = NT_ADR[10] = ppu->wire.NTHOut;
		AT_ADR[11] = NT_ADR[11] = ppu->wire.NTVOut;
		AT_ADR[12] = NT_ADR[12] = NOR(ppu->wire.n_FVO[0], NOT(BLNK));
		AT_ADR[13] = NT_ADR[13] = NOT(NOR(ppu->wire.FVO[1], NOT(BLNK)));

		for (size_t n = 0; n < 14; n++)
		{
			PAT_ADR[n] = ppu->wire.PAT_ADR[n];
		}
	}

	void PAMUX::sim_MuxOutputs()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState DB_PAR = ppu->wire.DB_PAR;
		TriState F_AT = ppu->fsm.FAT;

		for (size_t n = 0; n < 8; n++)
		{
			par_lo[n].sim(PCLK, PARR, DB_PAR, PAL, F_AT, AT_ADR[n], NT_ADR[n], PAT_ADR[n], ppu->GetDBBit(n), ppu->wire.n_PA_Bot[n]);
		}

		for (size_t n = 0; n < 6; n++)
		{
			par_hi[n].sim(PCLK, PARR, PAH, F_AT, AT_ADR[8 + n], NT_ADR[8 + n], PAT_ADR[8 + n], ppu->wire.n_PA_Top[n]);
		}
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