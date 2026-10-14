// VRAM Controller

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	VRAM_Control::VRAM_Control(PPU* parent) : ppu(parent) {}
	VRAM_Control::~VRAM_Control() {}

	void VRAM_Control::sim()
	{
		sim_RD();		// PD/RB, RD
		sim_WR();		// DB/PAR, TSTEP, WR
		sim_ALE();		// /ALE
	}

	void VRAM_Control::sim_RD()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;

		auto R7_EN = NOR(ppu->wire.n_R7, ppu->wire.n_DBE);
		R7_FF.set(NOR(rd_latch3.nget(), NOR(R7_EN, R7_FF.get())));
		RD_FF.set(MUX(PCLK, NOT(NOT(RD_FF.get())), NOR(R7_EN, R7_FF.nget())));
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

		auto W7_EN = NOR(ppu->wire.n_W7, ppu->wire.n_DBE);
		W7_FF.set(NOR(wr_latch3.nget(), NOR(W7_EN, W7_FF.get())));
		WR_FF.set(MUX(PCLK, NOT(NOT(WR_FF.get())), NOR(W7_EN, W7_FF.nget())));
		wr_latch1.set(NOT(WR_FF.get()), n_PCLK);
		wr_latch2.set(wr_latch1.nget(), PCLK);
		wr_latch3.set(wr_latch2.nget(), n_PCLK);
		wr_latch4.set(wr_latch3.nget(), PCLK);
		tmp_1 = NOR(wr_latch4.get(), wr_latch2.nget());

		// DB/PAR, TSTEP, WR

		ppu->wire.DB_PAR = NOR(wr_latch3.get(), wr_latch1.nget());
		tstep_latch.set(ppu->wire.DB_PAR, PCLK);
		ppu->wire.TSTEP = NOT(NOR(tstep_latch.get(), ppu->wire.PD_RB));
		ppu->wire.WR = NOR(NOT(ppu->wire.DB_PAR), ppu->wire.TH_MUX);
	}

	/// <summary>
	/// Call after Data Reader, but BEFORE MUX/CRAM & Read Buffer. PPU Address is obtained only after PAR simulation.
	/// </summary>
	void VRAM_Control::sim_TH_MUX()
	{
		// TH/MUX

		blnk_latch.set(ppu->fsm.BLNK, ppu->wire.PCLK);

		const auto address = ppu->data_reader->pamux->get_n_address();
		ppu->wire.TH_MUX = blnk_latch.get() == TriState::One && (address & 0x3f00) == 0 ? TriState::One : TriState::Zero;

		// XRB

		auto R7_EN = NOR(ppu->wire.n_R7, ppu->wire.n_DBE);
		ppu->wire.XRB = NOT(NOR(NOT(R7_EN), ppu->wire.TH_MUX));
	}

	void VRAM_Control::sim_ALE()
	{
		ppu->wire.n_ALE = NOR3(NOR3(ppu->wire.H0_Dash, ppu->fsm.BLNK, ppu->wire.n_PCLK), tmp_1, tmp_2);
	}

	/// <summary>
	/// The Read Buffer should simulate after the Data Reader, after the PD bus gets a value to write to the DB.
	/// </summary>
	void VRAM_Control::sim_ReadBuffer()
	{
		if (ppu->wire.PD_RB == TriState::One) read_buffer = ppu->PD;
		if (ppu->wire.RC == TriState::One) read_buffer = 0;
		if (ppu->wire.XRB == TriState::Zero) ppu->DB = read_buffer;
	}

	void RB_Bit::sim(size_t bit_num)
	{
		TriState XRB = ppu->wire.XRB;
		TriState PD_RB = ppu->wire.PD_RB;
		TriState RC = ppu->wire.RC;

		ff.set(NOR(NOT(MUX(PD_RB, ff.get(), ppu->GetPDBit(bit_num))), RC));
		auto val = MUX(NOT(XRB), TriState::Z, ff.get());
		ppu->SetDBBit(bit_num, val);
	}

	TriState RB_Bit::get()
	{
		return ff.get();
	}

	void RB_Bit::set(TriState value)
	{
		ff.set(value);
	}

	uint8_t VRAM_Control::Debug_GetRB() { return read_buffer; }
	void VRAM_Control::Debug_SetRB(uint8_t value) { read_buffer = value; }
}
