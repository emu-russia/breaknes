// OAM FIFO (Motion picture buffer memory)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	FIFO::FIFO(PPU* parent)
	{
		ppu = parent;
	}

	FIFO::~FIFO()
	{

	}

	void FIFO::sim()
	{

	}

	void FIFO::sim_SpriteH()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PAR_O = ppu->fsm.PARO;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState H1_DD = ppu->wire.H1_Dash2;
		TriState H2_DD = ppu->wire.H2_Dash2;

		TriState in[3]{};
		TriState dec_out[8]{};
		in[0] = H0_DD;
		in[1] = H1_DD;
		in[2] = H2_DD;

		DMX3(in, dec_out);

		sh2_latch.set(MUX(PAR_O, dec_out[2], TriState::Zero), n_PCLK);
		sh3_latch.set(MUX(PAR_O, dec_out[3], TriState::Zero), n_PCLK);
		sh5_latch.set(MUX(PAR_O, dec_out[5], TriState::Zero), n_PCLK);
		sh7_latch.set(MUX(PAR_O, dec_out[7], TriState::Zero), n_PCLK);

		ppu->wire.n_SH2 = sh2_latch.nget();
		ppu->wire.n_SH3 = sh3_latch.nget();
		ppu->wire.n_SH5 = sh5_latch.nget();
		ppu->wire.n_SH7 = sh7_latch.nget();
	}
}
