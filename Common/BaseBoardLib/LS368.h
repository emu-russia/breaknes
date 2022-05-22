// 40H368
// Hex bus buffer inverted 3-State output

#pragma once

namespace BaseBoard
{
	enum class LS368_Input
	{
		n_G1 = 0,
		n_G2,
		A1,
		A2,
		A3,
		A4,
		A5,
		A6,
		Max,
	};

	enum class LS368_Output
	{
		n_Y1 = 0,
		n_Y2,
		n_Y3,
		n_Y4,
		n_Y5,
		n_Y6,
		Max
	};

	class LS368
	{
	public:
		static void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[]);
	};
}
