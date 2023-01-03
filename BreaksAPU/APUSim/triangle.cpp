// Triangle Channel

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
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
		sim_LinearCounter();
		sim_FreqCounter();
		sim_Output();
	}

	void TriangleChan::sim_Control()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W4008 = apu->wire.W4008;
		TriState W400B = apu->wire.W400B;
		TriState n_LFO1 = apu->wire.n_LFO1;

		n_FOUT = fout_latch.nget();

		lc_reg.sim(n_ACLK, W4008, apu->GetDBBit(7));

		TriState set_reload = NOR3(reload_latch1.get(), lc_reg.get(), n_LFO1);
		Reload_FF.set(NOR(NOR(Reload_FF.get(), set_reload), W400B));
		TriState TRELOAD = Reload_FF.nget();

		reload_latch1.set(Reload_FF.get(), n_ACLK);
		reload_latch2.set(TRELOAD, n_ACLK);
		tco_latch.set(TCO, n_ACLK);

		LOAD = NOR(n_LFO1, reload_latch2.nget());
		STEP = NOR3(n_LFO1, reload_latch2.get(), tco_latch.get());
		TSTEP = NOR5(TCO, apu->wire.LOCK, apu->wire.PHI1, apu->wire.NOTRI, n_FOUT);
	}

	void TriangleChan::sim_LinearCounter()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W4008 = apu->wire.W4008;
		TriState RES = apu->wire.RES;

		for (size_t n = 0; n < 7; n++)
		{
			lin_reg[n].sim(n_ACLK, W4008, apu->GetDBBit(n));
		}

		TriState carry = TriState::One;

		for (size_t n = 0; n < 7; n++)
		{
			carry = lin_cnt[n].sim(carry, RES, LOAD, STEP, n_ACLK, lin_reg[n].get());
		}

		TCO = carry;
	}

	void TriangleChan::sim_FreqCounter()
	{
		TriState PHI1 = apu->wire.PHI1;
		TriState W400A = apu->wire.W400A;
		TriState W400B = apu->wire.W400B;
		TriState RES = apu->wire.RES;

		for (size_t n = 0; n < 11; n++)
		{
			freq_reg[n].sim(PHI1, n < 8 ? W400A : W400B, n < 8 ? apu->GetDBBit(n) : apu->GetDBBit(n - 8));
		}

		TriState carry = TriState::One;

		TriState FLOAD = NOR(PHI1, n_FOUT);
		TriState FSTEP = NOR(PHI1, NOT(n_FOUT));

		for (size_t n = 0; n < 11; n++)
		{
			carry = freq_cnt[n].sim(carry, RES, FLOAD, FSTEP, PHI1, freq_reg[n].get());
		}

		fout_latch.set(carry, PHI1);
	}

	void TriangleChan::sim_Output()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;
		TriState W401A = apu->wire.W401A;
		TriState T[5]{};
		TriState nT[5]{};

		TriState carry = TriState::One;

		for (size_t n = 0; n < 5; n++)
		{
			carry = out_cnt[n].sim(carry, RES, W401A, TSTEP, n_ACLK, apu->GetDBBit(n));
		}

		TriState T4 = out_cnt[4].get();

		for (size_t n = 0; n < 4; n++)
		{
			T[n] = out_cnt[n].get();
			nT[n] = out_cnt[n].nget();
			apu->TRI_Out[n] = NOT(MUX(T4, T[n], nT[n]));
		}
	}

	TriState TriangleChan::get_LC()
	{
		return lc_reg.nget();
	}
}
