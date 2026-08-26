// LS139
// Dual 2-to-4 line decoder/demultiplexer

#pragma once

namespace BaseBoard
{
	class LS139
	{
	public:

		// The signals are simulated by reference: some boards feed the outputs of this chip back to its own inputs,
		// and a floating (Z) input has to be resolved by the internal pull-ups of this chip on the actual net.

		static void sim(
			BaseLogic::TriState& n_EN1,
			BaseLogic::TriState& n_EN2,
			BaseLogic::TriState& A1_0,
			BaseLogic::TriState& A1_1,
			BaseLogic::TriState& A2_0,
			BaseLogic::TriState& A2_1,
			BaseLogic::TriState n_Y1[4],
			BaseLogic::TriState n_Y2[4]);
	};
}
