// Data Reader (Still Picture Generator)

#pragma once

namespace PPUSim
{
	class DataReader
	{
		PPU* ppu = nullptr;

	public:

		PATGen* patgen = nullptr;
		PAR* par = nullptr;
		ScrollRegs* sccx = nullptr;
		BGCol* bgcol = nullptr;

		DataReader(PPU* parent);
		~DataReader();

		void sim();
	};
}
