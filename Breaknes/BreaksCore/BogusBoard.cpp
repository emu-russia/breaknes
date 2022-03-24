#include "pch.h"

namespace Breaknes
{
	BogusBoard::BogusBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev) : Board (apu_rev, ppu_rev)
	{

	}

	BogusBoard::~BogusBoard()
	{

	}

	void BogusBoard::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analogOutputs[])
	{

	}
}
