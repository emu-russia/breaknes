// Common elements of APU circuitry

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	void RegisterBit::sim(TriState n_ACLK, TriState Enable, TriState Value)
	{
		transp_latch.set(MUX(n_ACLK, MUX(Enable, TriState::Z, Value), NOT(transp_latch.nget())), TriState::One);
	}

	TriState RegisterBit::get()
	{
		return NOT(transp_latch.nget());
	}

	TriState RegisterBit::nget()
	{
		return transp_latch.nget();
	}

	TriState CounterBit::sim(TriState Carry, TriState Clear, TriState Load, TriState val)
	{
		// TBD.
		return TriState::Zero;
	}

	TriState CounterBit::get()
	{
		// TBD.
		return TriState::Zero;
	}

	TriState CounterBit::nget()
	{
		// TBD.
		return TriState::Zero;
	}
}
