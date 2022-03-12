// LS139
// Dual 2-to-4 line decoder/demultiplexer

#pragma once

namespace LS
{
	enum class LS139_Input
	{
		n_EN1 = 0,
		n_EN2,
		A1_0,
		A1_1,
		A2_0,
		A2_1,
		Max,
	};

	class LS139
	{
	public:
		static void sim(BaseLogic::TriState inputs[], BaseLogic::TriState n_Y1[4], BaseLogic::TriState n_Y2[4]);
	};
}
