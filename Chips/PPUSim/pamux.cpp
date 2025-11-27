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

		FAT_in[0] = ppu->wire.THO[2];
		FAT_in[1] = ppu->wire.THO[3];
		FAT_in[2] = ppu->wire.THO[4];
		FAT_in[3] = ppu->wire.TVO[2];
		FAT_in[4] = ppu->wire.TVO[3];
		FAT_in[5] = ppu->wire.TVO[4];
		FAT_in[6] = TriState::One;
		FAT_in[7] = TriState::One;
		FAT_in[8] = TriState::One;
		FAT_in[9] = TriState::One;

		PAR_in[0] = ppu->wire.THO[0];
		PAR_in[1] = ppu->wire.THO[1];
		PAR_in[2] = ppu->wire.THO[2];
		PAR_in[3] = ppu->wire.THO[3];
		PAR_in[4] = ppu->wire.THO[4];
		PAR_in[5] = ppu->wire.TVO[0];
		PAR_in[6] = ppu->wire.TVO[1];
		PAR_in[7] = ppu->wire.TVO[2];
		PAR_in[8] = ppu->wire.TVO[3];
		PAR_in[9] = ppu->wire.TVO[4];

		FAT_in[10] = PAR_in[10] = ppu->wire.NTHOut;
		FAT_in[11] = PAR_in[11] = ppu->wire.NTVOut;
		FAT_in[12] = PAR_in[12] = NOR(ppu->wire.n_FVO[0], NOT(BLNK));
		FAT_in[13] = PAR_in[13] = NOT(NOR(ppu->wire.FVO[1], NOT(BLNK)));

		for (size_t n = 0; n < 13; n++)
		{
			PAD_in[n] = ppu->wire.PAD[n];
		}
		PAD_in[13] = TriState::Zero;
	}

	void PAMUX::sim_MuxOutputs()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState DB_PAR = ppu->wire.DB_PAR;
		TriState F_AT = ppu->fsm.FAT;

		for (size_t n = 0; n < 8; n++)
		{
			par_lo[n].sim(PCLK, PARR, DB_PAR, PAL, F_AT, FAT_in[n], PAR_in[n], PAD_in[n], ppu->GetDBBit(n), ppu->wire.n_PA_Bot[n]);
		}

		for (size_t n = 0; n < 6; n++)
		{
			par_hi[n].sim(PCLK, PARR, PAH, F_AT, FAT_in[8 + n], PAR_in[8 + n], PAD_in[8 + n], ppu->wire.n_PA_Top[n]);
		}
	}

	void PAMUX_LowBit::sim(TriState PCLK, TriState PARR, TriState DB_PAR, TriState PAL, TriState F_AT,
		TriState FAT_in, TriState PAL_in, TriState PAD_in, TriState DB_in,
		TriState& n_PAx)
	{
		auto val = MUX(DB_PAR,
			MUX(PARR,
				MUX(PAL, MUX(F_AT, TriState::Z, FAT_in), PAL_in), PAD_in), DB_in);
		in_latch.set(val, TriState::One);
		out_latch.set(in_latch.nget(), PCLK);
		n_PAx = out_latch.get();
	}

	void PAMUX_HighBit::sim(TriState PCLK, TriState PARR, TriState PAH, TriState F_AT,
		TriState FAT_in, TriState PAH_in, TriState PAD_in,
		TriState& n_PAx)
	{
		auto val = MUX(PARR,
			MUX(PAH, MUX(F_AT, TriState::Z, FAT_in), PAH_in), PAD_in);
		in_latch.set(val, TriState::One);
		out_latch.set(in_latch.nget(), PCLK);
		n_PAx = out_latch.get();
	}
}