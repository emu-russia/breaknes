// OAM FIFO (Motion picture buffer memory)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	FIFO::FIFO(PPU* parent)
	{
		ppu = parent;

		for (size_t n = 0; n < 8; n++)
		{
			lane[n] = new FIFOLane(ppu);
		}
	}

	FIFO::~FIFO()
	{
		for (size_t n = 0; n < 8; n++)
		{
			delete lane[n];
		}
	}

	void FIFO::sim()
	{
		sim_HInv();
		sim_Lanes();
		sim_Prio();
	}

	void FIFO::sim_HInv()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_SH2 = ppu->wire.n_SH2;
		TriState OB6 = ppu->wire.OB[6];
		TriState Z_FIFO = ppu->wire.Z_FIFO;

		HINV_FF.set(MUX(NOR(n_PCLK, n_SH2), NOT(NOT(HINV_FF.get())), OB6));

		for (size_t n = 0; n < 8; n++)
		{
			tout_latch[n].set(MUX(HINV_FF.get(), ppu->GetPDBit(n), ppu->GetPDBit(7 - n)), n_PCLK);
			n_TX[n] = NAND(tout_latch[n].get(), Z_FIFO);
		}
	}

	/// <summary>
	/// Generate LaneOut outputs for the priority circuit.
	/// </summary>
	void FIFO::sim_Lanes()
	{
		for (size_t n = 0; n < 8; n++)
		{
			lane[n]->sim(LaneOut[n]);
		}
	}

	/// <summary>
	/// Based on the priorities, select one of the LaneOut values.
	/// </summary>
	void FIFO::sim_Prio()
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

#pragma region "FIFO Lane"

	void FIFOLane::sim_LaneControl()
	{

	}

	void FIFOLane::sim_CounterControl()
	{

	}

	void FIFOLane::sim_PairedSR()
	{

	}

	void FIFOLane::sim_Counter()
	{

	}

	void FIFOLane::sim(TriState ZOut[(size_t)FIFOLaneOutput::Max])
	{

	}

	void FIFO_CounterBit::sim()
	{
	}

	void FIFO_SRBit::sim()
	{
	}

#pragma endregion "FIFO Lane"

}
