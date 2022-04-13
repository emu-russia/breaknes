// VRAM Controller

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	VRAM_Control::VRAM_Control(PPU* parent)
	{
		ppu = parent;
	}

	VRAM_Control::~VRAM_Control()
	{
	}

	void VRAM_Control::sim()
	{
		sim_RD();		// PD/RB, RD
		sim_Misc();		// TH/MUX, XRB
		sim_WR();		// DB/PAR, TSTEP, WR
		sim_ALE();		// /ALE
	}

	void VRAM_Control::sim_RD()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;

		R7_EN = NOR(ppu->wire.n_R7, ppu->wire.n_DBE);
		R7_FF.set(NOR(R7_EN, NOR(R7_FF.get(), rd_latch3.nget())));
		RD_FF.set(MUX(PCLK, NOT(NOT(RD_FF.get())), NOR(R7_EN, R7_FF.get())));
		rd_latch1.set(NOT(RD_FF.get()), n_PCLK);
		rd_latch2.set(rd_latch1.nget(), PCLK);
		rd_latch3.set(rd_latch2.nget(), n_PCLK);
		rd_latch4.set(rd_latch3.nget(), PCLK);
		tmp_2 = NOR(rd_latch4.get(), rd_latch2.nget());

		// PD/RB, RD

		ppu->wire.PD_RB = NOR(rd_latch4.nget(), rd_latch2.get());
		h0_latch.set(ppu->wire.H0_Dash, PCLK);
		ppu->wire.RD = NOT(NOR(AND(h0_latch.get(), NOT(ppu->fsm.BLNK)), ppu->wire.PD_RB));
	}

	void VRAM_Control::sim_WR()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;

		W7_EN = NOR(ppu->wire.n_W7, ppu->wire.n_DBE);
		W7_FF.set(NOR(W7_EN, NOR(W7_FF.get(), wr_latch3.nget())));
		WR_FF.set(MUX(PCLK, NOT(NOT(WR_FF.get())), NOR(W7_EN, W7_FF.get())));
		wr_latch1.set(NOT(WR_FF.get()), n_PCLK);
		wr_latch2.set(wr_latch1.nget(), PCLK);
		wr_latch3.set(wr_latch2.nget(), n_PCLK);
		wr_latch4.set(wr_latch3.nget(), PCLK);
		tmp_1 = NOR(wr_latch4.nget(), wr_latch2.nget());

		// DB/PAR, TSTEP, WR

		ppu->wire.DB_PAR = NOR(wr_latch3.get(), wr_latch1.nget());
		tstep_latch.set(ppu->wire.DB_PAR, PCLK);
		ppu->wire.TSTEP = NOT(NOR(tstep_latch.get(), ppu->wire.PD_RB));
		ppu->wire.WR = NOR(NOT(ppu->wire.DB_PAR), ppu->wire.TH_MUX);
	}

	void VRAM_Control::sim_Misc()
	{
		// TH/MUX

		blnk_latch.set(ppu->fsm.BLNK, ppu->wire.PCLK);

		TriState in[7]{};
		in[0] = blnk_latch.nget();
		in[1] = ppu->wire.n_PA_Top[0];	// /PA8
		in[2] = ppu->wire.n_PA_Top[1];
		in[3] = ppu->wire.n_PA_Top[2];
		in[4] = ppu->wire.n_PA_Top[3];
		in[5] = ppu->wire.n_PA_Top[4];
		in[6] = ppu->wire.n_PA_Top[5];	// /PA13
		ppu->wire.TH_MUX = NOR7(in);

		// XRB

		ppu->wire.XRB = NOT(NOR(NOT(R7_EN), ppu->wire.TH_MUX));
	}

	void VRAM_Control::sim_ALE()
	{
		ppu->wire.n_ALE = NOR3(NOR3(ppu->wire.H0_Dash, ppu->fsm.BLNK, ppu->wire.n_PCLK), tmp_1, tmp_2);
	}
}
