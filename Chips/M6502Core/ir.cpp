#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	void IR::sim()
	{
		simImpl<0>();
	}

	template<int Phase> void IR::simImpl()
	{
		using P = Phi<Phase>;
		if (P::Phi1(core->wire.PHI1) && core->wire.FETCH)
		{
			ir_latch = core->predecode->n_PD;
			IROut = ~ir_latch;
		}
	}
	template void IR::simImpl<0>();
	template void IR::simImpl<1>();
	template void IR::simImpl<2>();
}
