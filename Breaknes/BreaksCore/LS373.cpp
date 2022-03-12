// LS373
// Octal D-type transparent latch; 3-state

#include "pch.h"

using namespace BaseLogic;

namespace LS
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
