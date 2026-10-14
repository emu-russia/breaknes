// Triangle Channel

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
{
	TriangleChan::TriangleChan(APU* parent)
	{
		apu = parent;
	}

	TriangleChan::~TriangleChan()
	{
	}

	void TriangleChan::sim()
	{
		sim_Control();
		sim_LinearReg();
		sim_LinearCounter();
		sim_FreqReg();
		sim_FreqCounter();
		sim_Output();
	}

	void TriangleChan::sim_Control()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4008 = apu->wire.W4008;
		TriState W400B = apu->wire.W400B;
		TriState n_LFO1 = apu->wire.n_LFO1;

		n_FOUT = fout_latch.nget();

		lc_reg.sim(ACLK1, W4008, apu->GetDBBit(7));

		TriState set_reload = NOR3(reload_latch1.get(), lc_reg.get(), n_LFO1);
		Reload_FF.set(NOR(NOR(Reload_FF.get(), set_reload), W400B));
		TriState TRELOAD = Reload_FF.nget();

		reload_latch1.set(Reload_FF.get(), ACLK1);
		reload_latch2.set(TRELOAD, ACLK1);
		tco_latch.set(TCO, ACLK1);

		LOAD = NOR(n_LFO1, reload_latch2.nget());
		STEP = NOR3(n_LFO1, reload_latch2.get(), tco_latch.get());
		TSTEP = NOR5(TCO, apu->wire.LOCK, apu->wire.PHI1, apu->wire.NOTRI, n_FOUT);
	}

	void TriangleChan::sim_LinearReg()
	{
		if ((apu->wire.ACLK1 & 1) == 0 && (apu->wire.W4008 & 1) != 0) lin_reg = apu->DB & 0x7f;
	}

	void TriangleChan::sim_LinearCounter()
	{
		lin_cnt.sim(apu->wire.RES, LOAD, STEP, apu->wire.ACLK1, lin_reg);
		TCO = lin_cnt.get_carry();
	}

	void TriangleChan::sim_FreqReg()
	{
		if ((apu->wire.PHI1 & 1) == 0)
		{
			if ((apu->wire.W400A & 1) != 0) freq_reg = (freq_reg & 0x700) | apu->DB;
			if ((apu->wire.W400B & 1) != 0) freq_reg = (freq_reg & 0xff) | ((apu->DB & 7) << 8);
		}
	}

	void TriangleChan::sim_FreqCounter()
	{
		const TriState load = NOR(apu->wire.PHI1, n_FOUT);
		const TriState step = NOR(apu->wire.PHI1, NOT(n_FOUT));
		freq_cnt.sim(apu->wire.RES, load, step, apu->wire.PHI1, freq_reg);
		fout_latch.set(freq_cnt.get_carry(), apu->wire.PHI1);
	}

	void TriangleChan::sim_Output()
	{
		out_cnt.sim(apu->wire.RES, apu->wire.W401A, TSTEP, apu->wire.PHI1, apu->DB);
		const unsigned value = out_cnt.get();
		const unsigned triangle = (value & 16) != 0 ? (value & 15) : ((~value) & 15);
		for (unsigned n = 0; n < 4; ++n) apu->TRI_Out[n] = ((triangle >> n) & 1) ? TriState::One : TriState::Zero;
	}

	TriState TriangleChan::get_LC()
	{
		return lc_reg.nget();
	}

#pragma region "Debug"

	uint32_t TriangleChan::Get_LinearReg() { return lin_reg; }
	uint32_t TriangleChan::Get_LinearCounter() { return lin_cnt.get(); }
	uint32_t TriangleChan::Get_FreqReg() { return freq_reg; }
	uint32_t TriangleChan::Get_FreqCounter() { return freq_cnt.get(); }
	uint32_t TriangleChan::Get_OutputCounter() { return out_cnt.get(); }
	void TriangleChan::Set_LinearReg(uint32_t value) { lin_reg = value & 0x7f; }
	void TriangleChan::Set_LinearCounter(uint32_t value) { lin_cnt.set(uint16_t(value)); }
	void TriangleChan::Set_FreqReg(uint32_t value) { freq_reg = value & 0x7ff; }
	void TriangleChan::Set_FreqCounter(uint32_t value) { freq_cnt.set(uint16_t(value)); }
	void TriangleChan::Set_OutputCounter(uint32_t value) { out_cnt.set(uint16_t(value)); }

#pragma endregion "Debug"
}
