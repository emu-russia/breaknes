// Presettable synchronous 4-bit binary counter; asynchronous reset

#pragma once

namespace BaseBoard
{
	class LS161
	{
		uint8_t val = 0;

	public:
		void sim(
			BaseLogic::TriState CLK,
			BaseLogic::TriState nRST,
			BaseLogic::TriState nLD,
			BaseLogic::TriState EN_T,
			BaseLogic::TriState EN_P,
			BaseLogic::TriState P[4],
			BaseLogic::TriState& RCO,
			BaseLogic::TriState Q[4] );
	};
}
