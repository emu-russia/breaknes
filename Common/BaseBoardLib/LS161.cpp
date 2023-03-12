// Presettable synchronous 4-bit binary counter; asynchronous reset

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
		if (nLD == TriState::Zero && CLK == TriState::Zero /*nROMSEL :P */ )
		{
			val = PackNibble(P);
		}

		UnpackNibble(val, Q);

		// TBD: So far, crooked, some of the signals are not used at all because they are not needed
	}
}
