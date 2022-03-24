#include "pch.h"

namespace Breaknes
{
	BogusBoard::BogusBoard(Core* core, APUSim::Revision apu_rev, PPUSim::Revision ppu_rev) : Board (core, apu_rev, ppu_rev)
	{

	}

	BogusBoard::~BogusBoard()
	{

	}

	void BogusBoard::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analogOutputs[])
	{

	}
}
