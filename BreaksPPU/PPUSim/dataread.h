// Data Reader (Still Picture Generator)

#pragma once

namespace PPUSim
{
	class DataReader
	{
		PPU* ppu = nullptr;

	public:

		BGCol* bgcol = nullptr;
		PARGen* pargen = nullptr;
		ScrollRegs* sccx = nullptr;

		DataReader(PPU* parent);
		~DataReader();

		void sim();
	};
}
