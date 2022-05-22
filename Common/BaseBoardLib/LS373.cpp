// LS373
// Octal D-type transparent latch; 3-state

// https://github.com/emu-russia/breaks/blob/master/Docs/Famicom/LR74HC373.pdf

#include "pch.h"

using namespace BaseLogic;

namespace BaseBoard
{
	void LS373::sim(TriState LE, TriState n_OE, uint8_t d, uint8_t* q, bool& qz)
	{
		if (LE == TriState::One)
		{
			val = d;
		}

		if (n_OE == TriState::Zero)
		{
			*q = val;
			qz = false;
		}
		else
		{
			qz = true;
		}
	}
}
