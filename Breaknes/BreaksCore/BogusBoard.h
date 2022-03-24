#pragma once

namespace Breaknes
{
	class BogusBoard : public Board
	{
	public:
		BogusBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev);
		virtual ~BogusBoard();

		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analogOutputs[]);
	};
}
