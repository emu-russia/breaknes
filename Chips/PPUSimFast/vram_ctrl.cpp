// VRAM Controller

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	VRAM_Control::VRAM_Control(PPU* parent) : ppu(parent) {}
	VRAM_Control::~VRAM_Control() {}

	template<int Phase> void VRAM_Control::sim(EdgeContextT<Phase>& c)
	{
		sim_RD(c);		// PD/RB, RD
		sim_WR(c);		// DB/PAR, TSTEP, WR
		sim_ALE(c);		// /ALE
	}

	template<int Phase> void VRAM_Control::sim_RD(EdgeContextT<Phase>& c)
	{
		TriState PCLK = c.PCLK;
		TriState n_PCLK = c.n_PCLK;

		auto R7_EN = NOR(ppu->wire.n_R7, ppu->wire.n_DBE);
		R7_FF.set(NOR(rd_latch3.nget(), NOR(R7_EN, R7_FF.get())));
		RD_FF.set(MUX(PCLK, NOT(NOT(RD_FF.get())), NOR(R7_EN, R7_FF.nget())));
		rd_latch1.set(NOT(RD_FF.get()), n_PCLK);
		rd_latch2.set(rd_latch1.nget(), PCLK);
		rd_latch3.set(rd_latch2.nget(), n_PCLK);
		rd_latch4.set(rd_latch3.nget(), PCLK);
		tmp_2 = NOR(rd_latch4.get(), rd_latch2.nget());

		// PD/RB, RD

		c.PD_RB = ppu->wire.PD_RB = NOR(rd_latch4.nget(), rd_latch2.get());
		h0_latch.set(c.H0_Dash, PCLK);
		ppu->wire.RD = NOT(NOR(AND(h0_latch.get(), NOT(c.BLNK)), c.PD_RB));
	}

	template<int Phase> void VRAM_Control::sim_WR(EdgeContextT<Phase>& c)
	{
		TriState PCLK = c.PCLK;
		TriState n_PCLK = c.n_PCLK;

		auto W7_EN = NOR(ppu->wire.n_W7, ppu->wire.n_DBE);
		W7_FF.set(NOR(wr_latch3.nget(), NOR(W7_EN, W7_FF.get())));
		WR_FF.set(MUX(PCLK, NOT(NOT(WR_FF.get())), NOR(W7_EN, W7_FF.nget())));
		wr_latch1.set(NOT(WR_FF.get()), n_PCLK);
		wr_latch2.set(wr_latch1.nget(), PCLK);
		wr_latch3.set(wr_latch2.nget(), n_PCLK);
		wr_latch4.set(wr_latch3.nget(), PCLK);
		tmp_1 = NOR(wr_latch4.get(), wr_latch2.nget());

		// DB/PAR, TSTEP, WR

		c.DB_PAR = ppu->wire.DB_PAR = NOR(wr_latch3.get(), wr_latch1.nget());
		tstep_latch.set(c.DB_PAR, PCLK);
		c.TSTEP = ppu->wire.TSTEP = NOT(NOR(tstep_latch.get(), c.PD_RB));
		// TH_MUX is this call's PREVIOUS value: sim_TH_MUX runs after the data reader (uroboros).
		ppu->wire.WR = NOR(NOT(c.DB_PAR), ppu->wire.TH_MUX);
	}

	/// <summary>
	/// Call after Data Reader, but BEFORE MUX/CRAM & Read Buffer. PPU Address is obtained only after PAR simulation.
	/// </summary>
	template<int Phase> void VRAM_Control::sim_TH_MUX(EdgeContextT<Phase>& c)
	{
		// TH/MUX

		blnk_latch.set(c.BLNK, c.PCLK);

		const auto address = ppu->data_reader->pamux->get_n_address();
		c.TH_MUX = ppu->wire.TH_MUX = blnk_latch.get() == TriState::One && (address & 0x3f00) == 0 ? TriState::One : TriState::Zero;

		// XRB

		auto R7_EN = NOR(ppu->wire.n_R7, ppu->wire.n_DBE);
		c.XRB = ppu->wire.XRB = NOT(NOR(NOT(R7_EN), c.TH_MUX));
	}

	template<int Phase> void VRAM_Control::sim_ALE(const EdgeContextT<Phase>& c)
	{
		ppu->wire.n_ALE = NOR3(NOR3(c.H0_Dash, c.BLNK, c.n_PCLK), tmp_1, tmp_2);
	}

	/// <summary>
	/// The Read Buffer should simulate after the Data Reader, after the PD bus gets a value to write to the DB.
	/// </summary>
	template<int Phase> void VRAM_Control::sim_ReadBuffer(const EdgeContextT<Phase>& c)
	{
		if (c.PD_RB == TriState::One) read_buffer = ppu->PD;
		if (ppu->wire.RC == TriState::One) read_buffer = 0;
		if (c.XRB == TriState::Zero) ppu->DB = read_buffer;
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

	template void VRAM_Control::sim<0>(EdgeContextT<0>&);
	template void VRAM_Control::sim<1>(EdgeContextT<1>&);
	template void VRAM_Control::sim_TH_MUX<0>(EdgeContextT<0>&);
	template void VRAM_Control::sim_TH_MUX<1>(EdgeContextT<1>&);
	template void VRAM_Control::sim_ReadBuffer<0>(const EdgeContextT<0>&);
	template void VRAM_Control::sim_ReadBuffer<1>(const EdgeContextT<1>&);
}
