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
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4008 = apu->wire.W4008;

		for (size_t n = 0; n < 7; n++)
		{
			lin_reg[n].sim(ACLK1, W4008, apu->GetDBBit(n));
		}
	}

	void TriangleChan::sim_LinearCounter()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState RES = apu->wire.RES;

		TriState carry = TriState::One;

		for (size_t n = 0; n < 7; n++)
		{
			carry = lin_cnt[n].sim(carry, RES, LOAD, STEP, ACLK1, lin_reg[n].get());
		}

		TCO = carry;
	}

	void TriangleChan::sim_FreqReg()
	{
		TriState PHI1 = apu->wire.PHI1;
		TriState W400A = apu->wire.W400A;
		TriState W400B = apu->wire.W400B;

		for (size_t n = 0; n < 11; n++)
		{
			freq_reg[n].sim(PHI1, n < 8 ? W400A : W400B, n < 8 ? apu->GetDBBit(n) : apu->GetDBBit(n - 8));
		}
	}

	void TriangleChan::sim_FreqCounter()
	{
		TriState PHI1 = apu->wire.PHI1;
		TriState RES = apu->wire.RES;

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
		TriState PHI1 = apu->wire.PHI1;
		TriState RES = apu->wire.RES;
		TriState W401A = apu->wire.W401A;
		TriState T[5]{};
		TriState nT[5]{};

		TriState carry{};

		// The developers decided to use PHI1 for the triangle channel instead of ACLK to smooth out the "stepped" signal.

		carry = TriState::One;
		for (size_t n = 0; n < 5; n++)
		{
			carry = out_cnt[n].sim(carry, RES, W401A, TSTEP, PHI1 /* !!! */, apu->GetDBBit(n));
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

#pragma region "Debug"

	uint32_t TriangleChan::Get_LinearReg()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 7; n++)
		{
			val_lo[n] = lin_reg[n].get();
		}
		val_lo[7] = TriState::Zero;
		return Pack(val_lo);
	}

	uint32_t TriangleChan::Get_LinearCounter()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 7; n++)
		{
			val_lo[n] = lin_cnt[n].get();
		}
		val_lo[7] = TriState::Zero;
		return Pack(val_lo);
	}

	uint32_t TriangleChan::Get_FreqReg()
	{
		TriState val_lo[8]{};
		TriState val_hi[4]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = freq_reg[n].get();
		}
		for (size_t n = 0; n < 3; n++)
		{
			val_hi[n] = freq_reg[n + 8].get();
		}
		val_hi[3] = TriState::Zero;
		return Pack(val_lo) | ((uint32_t)PackNibble(val_hi) << 8);
	}

	uint32_t TriangleChan::Get_FreqCounter()
	{
		TriState val_lo[8]{};
		TriState val_hi[4]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = freq_cnt[n].get();
		}
		for (size_t n = 0; n < 3; n++)
		{
			val_hi[n] = freq_cnt[n + 8].get();
		}
		val_hi[3] = TriState::Zero;
		return Pack(val_lo) | ((uint32_t)PackNibble(val_hi) << 8);
	}

	uint32_t TriangleChan::Get_OutputCounter()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 5; n++)
		{
			val_lo[n] = out_cnt[n].get();
		}
		val_lo[5] = TriState::Zero;
		val_lo[6] = TriState::Zero;
		val_lo[7] = TriState::Zero;
		return Pack(val_lo);
	}

	void TriangleChan::Set_LinearReg(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		for (size_t n = 0; n < 7; n++)
		{
			lin_reg[n].set(val_lo[n]);
		}
	}

	void TriangleChan::Set_LinearCounter(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		for (size_t n = 0; n < 7; n++)
		{
			lin_cnt[n].set(val_lo[n]);
		}
	}

	void TriangleChan::Set_FreqReg(uint32_t value)
	{
		TriState val_lo[8]{};
		TriState val_hi[4]{};
		Unpack(value, val_lo);
		UnpackNibble(value >> 8, val_hi);
		for (size_t n = 0; n < 8; n++)
		{
			freq_reg[n].set(val_lo[n]);
		}
		for (size_t n = 0; n < 3; n++)
		{
			freq_reg[n + 8].set(val_hi[n]);
		}
	}

	void TriangleChan::Set_FreqCounter(uint32_t value)
	{
		TriState val_lo[8]{};
		TriState val_hi[4]{};
		Unpack(value, val_lo);
		UnpackNibble(value >> 8, val_hi);
		for (size_t n = 0; n < 8; n++)
		{
			freq_cnt[n].set(val_lo[n]);
		}
		for (size_t n = 0; n < 3; n++)
		{
			freq_cnt[n + 8].set(val_hi[n]);
		}
	}

	void TriangleChan::Set_OutputCounter(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		for (size_t n = 0; n < 5; n++)
		{
			out_cnt[n].set(val_lo[n]);
		}
	}

#pragma endregion "Debug"
}
