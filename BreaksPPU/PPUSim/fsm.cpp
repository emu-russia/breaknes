// H/V Control Logic

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	FSM::FSM(PPU* parent)
	{
		ppu = parent;
	}

	FSM::~FSM()
	{

	}

	void FSM::sim()
	{

	}

	TriState FSM::get_RESCL()
	{
		// TBD
		return TriState::Zero;
	}
}
