// Data Reader (Still Picture Generator)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	DataReader::DataReader(PPU* parent)
	{
		ppu = parent;

		par = new PAR(ppu);
		tilecnt = new TileCnt(ppu);
		pamux = new PAMUX(ppu);
		sccx = new ScrollRegs(ppu);
		bgcol = new BGCol(ppu);
	}

	DataReader::~DataReader()
	{
		delete par;
		delete tilecnt;
		delete pamux;
		delete sccx;
		delete bgcol;
	}

	template<int Phase> void DataReader::sim(const EdgeContextT<Phase>& c)
	{
		sccx->sim();
		pamux->sim(c);
		tilecnt->sim(c);
		par->sim(c);
		pamux->sim_MuxInputs(c);
		pamux->sim_MuxOutputs(c);
		bgcol->sim(c);
	}

	template void DataReader::sim<0>(const EdgeContextT<0>&);
	template void DataReader::sim<1>(const EdgeContextT<1>&);
}
