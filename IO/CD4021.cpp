// CD4021 8-Stage Static Shift Register
#include "pch.h"

using namespace BaseLogic;

namespace IO
{
	void CD4021::sim(
		BaseLogic::TriState CLK,
		BaseLogic::TriState PAR_SER,
		BaseLogic::TriState SER_IN, uint8_t PAR_IN,
		BaseLogic::TriState& Q5, BaseLogic::TriState& Q6, BaseLogic::TriState& Q7)
	{
		// When the parallel input is active (P/S=1) - CLK does not matter. The input value is simply loaded into the shift register.
		// When serial input is active (P/S=0), the register value is shifted by posedge CLK and Serial in is written to the lsb.

		if (PAR_SER == TriState::One) {

			dff = PAR_IN;
		}
		else {

			if (IsPosedge(prev_clk, CLK)) {

				dff <<= 1;
				dff |= ToByte(SER_IN);
			}
		}

		Q5 = FromByte((dff >> 5) & 1);
		Q6 = FromByte((dff >> 6) & 1);
		Q7 = FromByte((dff >> 7) & 1);

		prev_clk = CLK;
	}
}
