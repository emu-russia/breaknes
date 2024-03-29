// Square Channels

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	SquareChan::SquareChan(APU* parent, SquareChanCarryIn carry_routing)
	{
		apu = parent;
		cin_type = carry_routing;
		env_unit = new EnvelopeUnit(apu);
	}

	SquareChan::~SquareChan()
	{
		delete env_unit;
	}

	void SquareChan::sim(TriState WR0, TriState WR1, TriState WR2, TriState WR3, TriState NOSQ, TriState* SQ_Out)
	{
		dir_reg.sim(apu->wire.ACLK1, WR1, apu->GetDBBit(3));
		DEC = dir_reg.get();
		INC = NOT(DEC);

		sim_ShiftReg(WR1);
		sim_Sweep(WR1, NOSQ);

		sim_FreqReg(WR2, WR3);
		sim_BarrelShifter();
		sim_Adder();
		sim_FreqCounter();
		sim_Duty(WR0, WR3);

		env_unit->sim(Vol, WR0, WR3);
		sim_Output(NOSQ, SQ_Out);
	}

	void SquareChan::sim_FreqReg(TriState WR2, TriState WR3)
	{
		TriState nACLK2 = apu->wire.nACLK2;
		TriState ACLK1 = apu->wire.ACLK1;

		TriState ACLK3 = NOT(nACLK2);

		for (size_t n = 0; n < 11; n++)
		{
			freq_reg[n].sim(ACLK3, ACLK1,
				n < 8 ? WR2 : WR3,
				n < 8 ? apu->GetDBBit(n) : apu->GetDBBit(n - 8), DO_SWEEP, n_sum[n]);
		}
	}

	void SquareChan::sim_ShiftReg(TriState WR1)
	{
		TriState ACLK1 = apu->wire.ACLK1;

		for (size_t n = 0; n < 3; n++)
		{
			sr_reg[n].sim(ACLK1, WR1, apu->GetDBBit(n));
			SR[n] = sr_reg[n].get();
		}
	}

	void SquareChan::sim_BarrelShifter()
	{
		TriState q1[11]{};
		TriState q2[11]{};

		for (size_t n = 0; n < 11; n++)
		{
			BS[n] = MUX(DEC, freq_reg[n].get_Fx(DO_SWEEP), freq_reg[n].get_nFx(DO_SWEEP));
		}
		BS[11] = DEC;

		for (size_t n = 0; n < 11; n++)
		{
			q1[n] = MUX(SR[0], BS[n], BS[n + 1]);
		}

		for (size_t n = 0; n < 11; n++)
		{
			q2[n] = MUX(SR[1], q1[n], n < 9 ? q1[n+2] : BS[11]);
		}

		for (size_t n = 0; n < 11; n++)
		{
			S[n] = MUX(SR[2], q2[n], n < 7 ? q2[n+4] : BS[11]);
		}
	}

	void SquareChan::sim_Adder()
	{
		TriState n_carry = cin_type == SquareChanCarryIn::Vdd ? TriState::One : INC;
		TriState carry = NOT(n_carry);
		TriState Fx[11]{};

		for (size_t n = 0; n < 11; n++)
		{
			Fx[n] = freq_reg[n].get_Fx(DO_SWEEP);
			adder[n].sim(Fx[n], freq_reg[n].get_nFx(DO_SWEEP),
				S[n], NOT(S[n]),
				carry, n_carry,
				carry, n_carry, n_sum[n]);
		}

		n_COUT = n_carry;
		SW_UVF = NOR9 (Fx[2], Fx[3], Fx[4], Fx[5], Fx[6], Fx[7], Fx[8], Fx[9], Fx[10]);
	}

	void SquareChan::sim_FreqCounter()
	{
		TriState nACLK2 = apu->wire.nACLK2;
		TriState ACLK1 = apu->wire.ACLK1;
		TriState RES = apu->wire.RES;

		FLOAD = NOR(nACLK2, fco_latch.nget());
		TriState FSTEP = NOR(nACLK2, NOT(fco_latch.nget()));

		TriState carry = TriState::One;

		for (size_t n = 0; n < 11; n++)
		{
			carry = freq_cnt[n].sim(carry, RES, FLOAD, FSTEP, ACLK1, freq_reg[n].get_Fx(DO_SWEEP));
		}

		FCO = carry;
		fco_latch.set(FCO, ACLK1);
	}

	void SquareChan::sim_Sweep(TriState WR1, TriState NOSQ)
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState RES = apu->wire.RES;
		TriState n_LFO2 = apu->wire.n_LFO2;

		reload_latch.set(reload_ff.get(), ACLK1);
		reload_ff.set(NOR(NOR3(reload_ff.get(), n_LFO2, reload_latch.get()), WR1));

		swdis_reg.sim(ACLK1, WR1, apu->GetDBBit(7));
		TriState SWDIS = swdis_reg.nget();

		for (size_t n = 0; n < 3; n++)
		{
			sweep_reg[n].sim(ACLK1, WR1, apu->GetDBBit(n + 4));
		}

		TriState temp_reload = NOR(reload_latch.nget(), sco_latch.get());
		TriState SSTEP = NOR(n_LFO2, NOT(temp_reload));
		TriState SLOAD = NOR(n_LFO2, temp_reload);

		TriState SCO{};
		SCO = TriState::One;
		for (size_t n = 0; n < 3; n++)
		{
			SCO = sweep_cnt[n].sim(SCO, RES, SLOAD, SSTEP, ACLK1, sweep_reg[n].get());
		}

		sco_latch.set(SCO, ACLK1);

		SW_OVF = NOR(DEC, n_COUT);
		TriState SRZ = NOR3(SR[0], SR[1], SR[2]);
		DO_SWEEP = NOR7(SRZ, SWDIS, NOSQ, SW_OVF, sco_latch.nget(), n_LFO2, SW_UVF);
	}

	void SquareChan::sim_Duty(TriState WR0, TriState WR3)
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState RES = apu->wire.RES;
		TriState DT[3]{};

		for (size_t n = 0; n < 2; n++)
		{
			duty_reg[n].sim(ACLK1, WR0, apu->GetDBBit(n + 6));
		}

		TriState carry = FCO;
		for (size_t n = 0; n < 3; n++)
		{
			carry = duty_cnt[n].sim(carry, RES, WR3, FLOAD, ACLK1, TriState::Zero);
		}
		DT[0] = duty_cnt[0].get();
		DT[1] = duty_cnt[1].get();
		DT[2] = duty_cnt[2].get();

		TriState sel[2]{};
		sel[0] = duty_reg[0].get();
		sel[1] = duty_reg[1].get();

		TriState in[4]{};
		in[3] = NAND(DT[1], DT[2]);
		in[0] = NOR(NOT(DT[0]), in[3]);
		in[1] = NOT(in[3]);
		in[2] = DT[2];

		DUTY = MUX2(sel, in);		
	}

	void SquareChan::sim_Output(TriState NOSQ, TriState* SQ_Out)
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState LOCK = apu->wire.LOCK;

		TriState d = NOR4(NOT(DUTY), SW_UVF, NOSQ, SW_OVF);
		sqo_latch.set(d, ACLK1);
		TriState sqv = NOR(sqo_latch.get(), LOCK);

		for (size_t n = 0; n < 4; n++)
		{
			SQ_Out[n] = NOR(sqv, NOT(Vol[n]));
		}
	}

	void FreqRegBit::sim(TriState ACLK3, TriState ACLK1, TriState WR, TriState DB_in, TriState ADDOUT, TriState n_sum)
	{
		TriState d = MUX(WR, MUX(ACLK3, TriState::Z, get_Fx(ADDOUT)), DB_in);
		transp_latch.set(d, TriState::One);
		sum_latch.set(n_sum, ACLK1);
	}

	TriState FreqRegBit::get_nFx(TriState ADDOUT)
	{
		return NOR(AND(sum_latch.nget(), ADDOUT), transp_latch.get());
	}

	TriState FreqRegBit::get_Fx(TriState ADDOUT)
	{
		return NOR(get_nFx(ADDOUT), AND(sum_latch.get(), ADDOUT));
	}

	TriState FreqRegBit::get()
	{
		return transp_latch.get();
	}

	void FreqRegBit::set(TriState value)
	{
		transp_latch.set(value, TriState::One);
	}

	void AdderBit::sim(TriState F, TriState nF, TriState S, TriState nS, TriState C, TriState nC,
		TriState& cout, TriState& n_cout, TriState& n_sum)
	{
		n_cout = NOR3(AND(F, S), AND3(F, nS, C), AND3(nF, S, C));
		n_sum = NOR4(AND3(F, nS, nC), AND3(nF, S, nC), AND3(nF, nS, C), AND3(F, S, C));
		cout = NOT(n_cout);
	}

	TriState SquareChan::get_LC()
	{
		return env_unit->get_LC();
	}

#pragma region "Debug"

	uint32_t SquareChan::Get_FreqReg()
	{
		TriState val[4]{};
		TriState val_byte[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_byte[n] = freq_reg[n].get();
		}
		for (size_t n = 0; n < 3; n++)
		{
			val[n] = freq_reg[n + 8].get();
		}
		val[3] = TriState::Zero;
		return Pack(val_byte) | ((uint32_t)PackNibble(val) << 8);
	}

	uint32_t SquareChan::Get_ShiftReg()
	{
		TriState val[4]{};
		for (size_t n = 0; n < 3; n++)
		{
			val[n] = sr_reg[n].get();
		}
		val[3] = TriState::Zero;
		return PackNibble(val);
	}

	uint32_t SquareChan::Get_FreqCounter()
	{
		TriState val[4]{};
		TriState val_byte[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_byte[n] = freq_cnt[n].get();
		}
		for (size_t n = 0; n < 3; n++)
		{
			val[n] = freq_cnt[n + 8].get();
		}
		val[3] = TriState::Zero;
		return Pack(val_byte) | ((uint32_t)PackNibble(val) << 8);
	}

	uint32_t SquareChan::Get_SweepReg()
	{
		TriState val[4]{};
		for (size_t n = 0; n < 3; n++)
		{
			val[n] = sweep_reg[n].get();
		}
		val[3] = TriState::Zero;
		return PackNibble(val);
	}

	uint32_t SquareChan::Get_SweepCounter()
	{
		TriState val[4]{};
		for (size_t n = 0; n < 3; n++)
		{
			val[n] = sweep_cnt[n].get();
		}
		val[3] = TriState::Zero;
		return PackNibble(val);
	}

	uint32_t SquareChan::Get_DutyCounter()
	{
		TriState val[4]{};
		for (size_t n = 0; n < 3; n++)
		{
			val[n] = duty_cnt[n].get();
		}
		val[3] = TriState::Zero;
		return PackNibble(val);
	}

	void SquareChan::Set_FreqReg(uint32_t value)
	{
		TriState val[4]{};
		TriState val_byte[8]{};
		Unpack(value, val_byte);
		UnpackNibble(value >> 8, val);
		for (size_t n = 0; n < 8; n++)
		{
			freq_reg[n].set(val_byte[n]);
		}
		for (size_t n = 0; n < 3; n++)
		{
			freq_reg[n + 8].set(val[n]);
		}
	}

	void SquareChan::Set_ShiftReg(uint32_t value)
	{
		TriState val[4]{};
		UnpackNibble(value, val);
		for (size_t n = 0; n < 3; n++)
		{
			sr_reg[n].set(val[n]);
		}
	}

	void SquareChan::Set_FreqCounter(uint32_t value)
	{
		TriState val[4]{};
		TriState val_byte[8]{};
		Unpack(value, val_byte);
		UnpackNibble(value >> 8, val);
		for (size_t n = 0; n < 8; n++)
		{
			freq_cnt[n].set(val_byte[n]);
		}
		for (size_t n = 0; n < 3; n++)
		{
			freq_cnt[n + 8].set(val[n]);
		}
	}

	void SquareChan::Set_SweepReg(uint32_t value)
	{
		TriState val[4]{};
		UnpackNibble(value, val);
		for (size_t n = 0; n < 3; n++)
		{
			sweep_reg[n].set(val[n]);
		}
	}

	void SquareChan::Set_SweepCounter(uint32_t value)
	{
		TriState val[4]{};
		UnpackNibble(value, val);
		for (size_t n = 0; n < 3; n++)
		{
			sweep_cnt[n].set(val[n]);
		}
	}

	void SquareChan::Set_DutyCounter(uint32_t value)
	{
		TriState val[4]{};
		UnpackNibble(value, val);
		for (size_t n = 0; n < 3; n++)
		{
			duty_cnt[n].set(val[n]);
		}
	}

#pragma endregion "Debug"
}
