// Data Reader (Still Picture Generator)

#pragma once

namespace PPUSim
{
	class DataReader
	{
		PPU* ppu = nullptr;

	public:

		PAR* par = nullptr;
		TileCnt* tilecnt = nullptr;
		PAMUX* pamux = nullptr;
		ScrollRegs* sccx = nullptr;
		BGCol* bgcol = nullptr;

		DataReader(PPU* parent);
		~DataReader();

		void sim();
	};
}
