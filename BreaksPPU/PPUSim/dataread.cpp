// Data Reader (Still Picture Generator)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	DataReader::DataReader(PPU* parent)
	{
		ppu = parent;

		bgcol = new BGCol(ppu);
		pargen = new PARGen(ppu);
		sccx = new ScrollRegs(ppu);
	}

	DataReader::~DataReader()
	{
		delete bgcol;
		delete pargen;
		delete sccx;
	}

	void DataReader::sim()
	{
		sccx->sim();
		pargen->sim();
		bgcol->sim();
	}
}
