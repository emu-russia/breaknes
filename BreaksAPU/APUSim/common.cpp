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

	TriState CounterBit::sim(TriState Carry, TriState Clear, TriState Load, TriState Step, TriState n_ACLK, TriState val)
	{
		TriState latch_in = 
			MUX(Load, 
				MUX(Clear, 
					MUX(Step, 
						MUX(n_ACLK, TriState::Z, NOT(transp_latch.nget())),
						cg_latch.nget()), 
					TriState::Zero), 
				val);

		transp_latch.set(latch_in, TriState::One);
		cg_latch.set( MUX(Carry, transp_latch.nget(), NOT(transp_latch.nget())), n_ACLK);

		TriState cout = NOR(NOT(Carry), transp_latch.nget());
		return cout;
	}

	TriState CounterBit::get()
	{
		return NOT(transp_latch.nget());
	}

	TriState CounterBit::nget()
	{
		return transp_latch.nget();
	}

	TriState DownCounterBit::sim(TriState Carry, TriState Clear, TriState Load, TriState Step, TriState n_ACLK, TriState val)
	{
		TriState latch_in =
			MUX(Load,
				MUX(Clear,
					MUX(Step,
						MUX(n_ACLK, TriState::Z, NOT(transp_latch.nget())),
						cg_latch.nget()),
					TriState::Zero),
				val);

		transp_latch.set(latch_in, TriState::One);
		cg_latch.set(MUX(Carry, transp_latch.nget(), NOT(transp_latch.nget())), n_ACLK);

		TriState cout = NOR(NOT(Carry), NOT(transp_latch.nget()));
		return cout;
	}

	TriState DownCounterBit::get()
	{
		return NOT(transp_latch.nget());
	}

	TriState DownCounterBit::nget()
	{
		return transp_latch.nget();
	}

	TriState RevCounterBit::sim(TriState Carry, TriState Dec, TriState Clear, TriState Load, TriState Step, TriState n_ACLK, TriState val)
	{
		TriState latch_in =
			MUX(Load,
				MUX(Clear,
					MUX(Step,
						MUX(n_ACLK, TriState::Z, NOT(transp_latch.nget())),
						cg_latch.nget()),
					TriState::Zero),
				val);

		transp_latch.set(latch_in, TriState::One);
		cg_latch.set(MUX(Carry, transp_latch.nget(), NOT(transp_latch.nget())), n_ACLK);

		TriState cout = NOR(NOT(Carry), MUX(Dec, transp_latch.nget(), NOT(transp_latch.nget())));
		return cout;
	}

	TriState RevCounterBit::get()
	{
		return NOT(transp_latch.nget());
	}

	TriState RevCounterBit::nget()
	{
		return transp_latch.nget();
	}
}
