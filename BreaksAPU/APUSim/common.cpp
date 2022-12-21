// Common elements of APU circuitry

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	void RegisterBit::sim(TriState n_ACLK, TriState Enable, TriState Value)
	{
		// TBD.
	}

	TriState RegisterBit::get()
	{
		// TBD.
		return TriState::Zero;
	}

	TriState RegisterBit::nget()
	{
		// TBD.
		return TriState::Zero;
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
