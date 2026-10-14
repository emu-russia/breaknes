// Square Channels

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
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

	uint16_t SquareChan::get_Fx() { return DO_SWEEP == TriState::One ? uint16_t(~freq_sum) & 0x7ff : freq_reg; }
	uint16_t SquareChan::get_nFx() { return DO_SWEEP == TriState::One ? freq_sum & uint16_t(~freq_reg) & 0x7ff : uint16_t(~freq_reg) & 0x7ff; }

	void SquareChan::sim_FreqReg(TriState WR2, TriState WR3)
	{
		// Preserve the old sum bank for the input-side transparent update.
		if (apu->wire.nACLK2 == TriState::Zero) freq_reg = get_Fx();
		if ((WR2 & 1) != 0) freq_reg = (freq_reg & 0x700) | apu->DB;
		if ((WR3 & 1) != 0) freq_reg = (freq_reg & 0xff) | ((apu->DB & 7) << 8);
		if (apu->wire.ACLK1 == TriState::One) freq_sum = n_sum;

	}

	void SquareChan::sim_ShiftReg(TriState WR1)
	{
		if ((apu->wire.ACLK1 & 1) == 0 && (WR1 & 1) != 0) sr_reg = apu->DB & 7;
	}

	void SquareChan::sim_BarrelShifter()
	{
		const unsigned bits = DEC == TriState::One ? get_nFx() : get_Fx();
		shifted = uint16_t((bits >> sr_reg) | (DEC == TriState::One ? (0x7ff ^ (0x7ff >> sr_reg)) : 0));
	}

	void SquareChan::sim_Adder()
	{
		const unsigned f = get_Fx(), nf = get_nFx(), operand = shifted;
		const unsigned input = cin_type == SquareChanCarryIn::Vdd ? 0u : unsigned(DEC == TriState::One);
		unsigned result;
#ifdef APUSIMFAST_SWEEP_ARITHMETIC
		if (DO_SWEEP == TriState::Zero)
		{
			// Complementary rails permit ordinary binary arithmetic.
			result = f + operand + input;
			n_sum = uint16_t(~result) & 0x7ff;
		}
		else
#endif
		{
			// During sum-bank selection, Fx/nFx can both be zero. Preserve the
			// dual-rail gate equations with a parallel carry-prefix network.
			unsigned generate = f & operand;
			unsigned propagate = (f & ~operand) | (nf & operand);
			const unsigned sum0 = propagate, sum1 = (nf & ~operand) | (f & operand);
			for (unsigned distance = 1; distance < 11; distance <<= 1)
			{
				generate |= propagate & (generate << distance);
				propagate &= (propagate << distance) | ((1u << distance) - 1u);
			}
			const unsigned carries = (generate << 1) | (input ? (propagate << 1) | 1u : 0u);
			n_sum = uint16_t(~((sum0 & ~carries) | (sum1 & carries))) & 0x7ff;
			result = carries;
		}
		n_COUT = (result & 0x800) != 0 ? TriState::Zero : TriState::One;
		SW_UVF = (f & 0x7fc) == 0 ? TriState::One : TriState::Zero;
	}

	void SquareChan::sim_FreqCounter()
	{
		FLOAD = NOR(apu->wire.nACLK2, fco_latch.nget());
		const TriState step = NOR(apu->wire.nACLK2, NOT(fco_latch.nget()));
		freq_cnt.sim(apu->wire.RES, FLOAD, step, apu->wire.ACLK1, get_Fx());
		FCO = freq_cnt.get_carry();
		fco_latch.set(FCO, apu->wire.ACLK1);
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

		if ((ACLK1 & 1) == 0 && (WR1 & 1) != 0) sweep_reg = (apu->DB >> 4) & 7;
		TriState temp_reload = NOR(reload_latch.nget(), sco_latch.get());
		TriState SSTEP = NOR(n_LFO2, NOT(temp_reload));
		TriState SLOAD = NOR(n_LFO2, temp_reload);
		sweep_cnt.sim(RES, SLOAD, SSTEP, ACLK1, sweep_reg);
		const TriState SCO = sweep_cnt.get_carry();

		sco_latch.set(SCO, ACLK1);

		SW_OVF = NOR(DEC, n_COUT);
		TriState SRZ = (sr_reg == 0 ? TriState::One : TriState::Zero);
		DO_SWEEP = NOR7(SRZ, SWDIS, NOSQ, SW_OVF, sco_latch.nget(), n_LFO2, SW_UVF);
	}

	void SquareChan::sim_Duty(TriState WR0, TriState WR3)
	{
		if ((apu->wire.ACLK1 & 1) == 0 && (WR0 & 1) != 0) duty_reg = apu->DB >> 6;
		duty_cnt.sim(apu->wire.RES, WR3, FLOAD, apu->wire.ACLK1, 0, FCO == TriState::One);
		const unsigned counter = duty_cnt.get();
		const bool duty = duty_reg == 0 ? counter == 7 : duty_reg == 1 ? counter >= 6 : duty_reg == 2 ? counter >= 4 : counter < 6;
		DUTY = duty ? TriState::One : TriState::Zero;
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

	uint32_t SquareChan::Get_FreqReg() { return freq_reg; }
	uint32_t SquareChan::Get_ShiftReg() { return sr_reg; }
	uint32_t SquareChan::Get_FreqCounter() { return freq_cnt.get(); }
	uint32_t SquareChan::Get_SweepReg() { return sweep_reg; }
	uint32_t SquareChan::Get_SweepCounter() { return sweep_cnt.get(); }
	uint32_t SquareChan::Get_DutyCounter() { return duty_cnt.get(); }
	void SquareChan::Set_FreqReg(uint32_t value) { freq_reg = value & 0x7ff; }
	void SquareChan::Set_ShiftReg(uint32_t value) { sr_reg = value & 7; }
	void SquareChan::Set_FreqCounter(uint32_t value) { freq_cnt.set(uint16_t(value)); }
	void SquareChan::Set_SweepReg(uint32_t value) { sweep_reg = value & 7; }
	void SquareChan::Set_SweepCounter(uint32_t value) { sweep_cnt.set(uint16_t(value)); }
	void SquareChan::Set_DutyCounter(uint32_t value) { duty_cnt.set(uint16_t(value)); }

#pragma endregion "Debug"
}
