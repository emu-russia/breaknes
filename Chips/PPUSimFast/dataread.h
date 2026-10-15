// Data Reader (Still Picture Generator)

#pragma once

namespace PPUSimFast
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

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim(const EdgeContextT<Phase>& c);
	};
}
