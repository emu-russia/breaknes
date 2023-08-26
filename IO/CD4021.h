// CD4021 8-Stage Static Shift Register
#pragma once

namespace IO
{
	class CD4021
	{
		uint8_t dff = 0;
		BaseLogic::TriState prev_clk = BaseLogic::TriState::X;

	public:
		void sim(
			BaseLogic::TriState CLK, 
			BaseLogic::TriState PAR_SER, 
			BaseLogic::TriState SER_IN, uint8_t PAR_IN,
			BaseLogic::TriState &Q5, BaseLogic::TriState& Q6, BaseLogic::TriState& Q7 );
	};
}
