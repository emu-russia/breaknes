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

	void SquareChan::sim(TriState WR0, TriState WR1, TriState WR2, TriState WR3)
	{
		sim_FreqReg();
		sim_ShiftReg();
		sim_BarrelShifter();
		sim_Adder();
		sim_FreqCounter();
		sim_Sweep();
		sim_Duty();

		env_unit->sim(Vol, WR0, WR3);
		sim_Output();
	}

	void SquareChan::sim_FreqReg()
	{

	}

	void SquareChan::sim_ShiftReg()
	{

	}

	void SquareChan::sim_BarrelShifter()
	{

	}

	void SquareChan::sim_Adder()
	{

	}

	void SquareChan::sim_FreqCounter()
	{

	}

	void SquareChan::sim_Sweep()
	{

	}

	void SquareChan::sim_Duty()
	{

	}

	void SquareChan::sim_Output()
	{

	}

	void FreqRegBit::sim(TriState n_ACLK3, TriState n_ACLK, TriState WR, TriState DB_in, TriState n_sum)
	{
	}

	TriState FreqRegBit::get_nFx(TriState ADDOUT)
	{
		return NOR(AND(sum_latch.nget(), ADDOUT), transp_latch.get());
	}

	TriState FreqRegBit::get_Fx(TriState ADDOUT)
	{
		return NOR(get_nFx(ADDOUT), AND(sum_latch.get(), ADDOUT));
	}

	void AdderBit::sim(TriState F, TriState nF, TriState S, TriState nS, TriState C, TriState nC,
		TriState& n_cout, TriState& cout, TriState& n_sum)
	{

	}

	TriState SquareChan::get_LC()
	{
		return env_unit->get_LC();
	}
}
