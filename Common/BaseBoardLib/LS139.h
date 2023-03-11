// LS139
// Dual 2-to-4 line decoder/demultiplexer

#pragma once

namespace BaseBoard
{
	class LS139
	{
	public:

		// You need to simulate the signals by the references, because some perverts feed the outputs from this chip back to the input

		static void sim(
			BaseLogic::TriState& n_EN1,
			BaseLogic::TriState& n_EN2,
			BaseLogic::TriState A1_0,
			BaseLogic::TriState A1_1,
			BaseLogic::TriState A2_0,
			BaseLogic::TriState A2_1,
			BaseLogic::TriState n_Y1[4],
			BaseLogic::TriState n_Y2[4]);
	};
}
