// Common elements of APU circuitry

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	void RegisterBit::sim(TriState ACLK1, TriState Enable, TriState Value)
	{
		transp_latch.set(MUX(ACLK1, MUX(Enable, TriState::Z, Value), NOT(transp_latch.nget())), TriState::One);
	}

	TriState RegisterBit::get()
	{
		return NOT(transp_latch.nget());
	}

	TriState RegisterBit::nget()
	{
		return transp_latch.nget();
	}

	void RegisterBit::set(TriState val)
	{
		transp_latch.set(val, TriState::One);
	}

	void RegisterBitRes::sim(TriState ACLK1, TriState Enable, TriState Value, TriState Res)
	{
		transp_latch.set(AND(MUX(ACLK1, MUX(Enable, TriState::Z, Value), NOT(transp_latch.nget())), NOT(Res)), TriState::One);
	}

	TriState RegisterBitRes::get()
	{
		return NOT(transp_latch.nget());
	}

	TriState RegisterBitRes::nget()
	{
		return transp_latch.nget();
	}

	void RegisterBitRes::set(TriState val)
	{
		transp_latch.set(val, TriState::One);
	}

	void RegisterBitRes2::sim(TriState ACLK1, TriState Enable, TriState Value, TriState Res1, TriState Res2)
	{
		transp_latch.set(AND(MUX(ACLK1, MUX(Enable, TriState::Z, Value), NOT(transp_latch.nget())), NOT(OR(Res1, Res2))), TriState::One);
	}

	TriState RegisterBitRes2::get()
	{
		return NOT(transp_latch.nget());
	}

	TriState RegisterBitRes2::nget()
	{
		return transp_latch.nget();
	}

	void RegisterBitRes2::set(TriState val)
	{
		transp_latch.set(val, TriState::One);
	}

	TriState CounterBit::sim(TriState Carry, TriState Clear, TriState Load, TriState Step, TriState ACLK1, TriState val)
	{
		TriState latch_in = 
			MUX(Load, 
				MUX(Clear, 
					MUX(Step, 
						MUX(ACLK1, TriState::Z, NOT(transp_latch.nget())),
						cg_latch.nget()), 
					TriState::Zero), 
				val);

		transp_latch.set(latch_in, TriState::One);
		cg_latch.set( MUX(Carry, transp_latch.nget(), NOT(transp_latch.nget())), ACLK1);

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

	void CounterBit::set(BaseLogic::TriState val)
	{
		transp_latch.set(val, TriState::One);
	}

	TriState DownCounterBit::sim(TriState Carry, TriState Clear, TriState Load, TriState Step, TriState ACLK1, TriState val)
	{
		TriState latch_in =
			MUX(Load,
				MUX(Clear,
					MUX(Step,
						MUX(ACLK1, TriState::Z, NOT(transp_latch.nget())),
						cg_latch.nget()),
					TriState::Zero),
				val);

		transp_latch.set(latch_in, TriState::One);
		cg_latch.set(MUX(Carry, transp_latch.nget(), NOT(transp_latch.nget())), ACLK1);

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

	void DownCounterBit::set(BaseLogic::TriState val)
	{
		transp_latch.set(val, TriState::One);
	}

	TriState RevCounterBit::sim(TriState Carry, TriState Dec, TriState Clear, TriState Load, TriState Step, TriState ACLK1, TriState val)
	{
		TriState latch_in =
			MUX(Load,
				MUX(Clear,
					MUX(Step,
						MUX(ACLK1, TriState::Z, NOT(transp_latch.nget())),
						cg_latch.nget()),
					TriState::Zero),
				val);

		transp_latch.set(latch_in, TriState::One);
		cg_latch.set(MUX(Carry, transp_latch.nget(), NOT(transp_latch.nget())), ACLK1);

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

	void RevCounterBit::set(BaseLogic::TriState val)
	{
		transp_latch.set(val, TriState::One);
	}
}
