// Data Reader (Still Picture Generator)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
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

	void DataReader::sim()
	{
		sccx->sim();
		pamux->sim();
		tilecnt->sim();
		par->sim();
		pamux->sim_MuxInputs();
		pamux->sim_MuxOutputs();
		bgcol->sim();
	}
}
