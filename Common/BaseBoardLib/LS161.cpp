// 74LS161: presettable synchronous 4-bit binary counter with asynchronous reset.

#include "pch.h"

using namespace BaseLogic;

// TBD: We will simulate high level without gates, then we can make a decap of the original SHARP chip when we are in the mood.

namespace BaseBoard
{
	void LS161::sim(
		TriState CLK,
		TriState nRST,
		TriState nLD,
		TriState EN_T,
		TriState EN_P,
		TriState P[4],
		TriState& RCO,
		TriState Q[4])
	{
		// Asynchronous reset has the highest priority.
		if (nRST == TriState::Zero)
		{
			val = 0;
		}
		else
		{
			// Synchronous operation on the rising edge of CLK.
			bool clkHigh = (CLK == TriState::One);
			bool rising = clkHigh && prev_CLK != TriState::One;

			if (rising)
			{
				if (nLD == TriState::Zero)
				{
					// Parallel load (e.g. the UNROM bank register: data bus -> P).
					val = PackNibble(P);
				}
				else if (EN_T == TriState::One && EN_P == TriState::One)
				{
					// Count up.
					val = (uint8_t)((val + 1) & 0xF);
				}
			}
		}

		prev_CLK = CLK;

		UnpackNibble(val, Q);

		// RCO = EN_T & Q0 & Q1 & Q2 & Q3
		RCO = (EN_T == TriState::One && val == 0xF) ? TriState::One : TriState::Zero;
	}
}
