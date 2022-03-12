#pragma once

namespace Breaknes
{
	class Board
	{
	public:
		Board(std::string name, std::string apu, std::string ppu, std::string p1);
		virtual ~Board();

		virtual void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analogOutputs[]) = 0;
	};
}
