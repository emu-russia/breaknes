// Data Reader (Still Picture Generator)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	DataReader::DataReader(PPU* parent)
	{
		ppu = parent;

		patgen = new PATGen(ppu);
		par = new PAR(ppu);
		sccx = new ScrollRegs(ppu);
		bgcol = new BGCol(ppu);
	}

	DataReader::~DataReader()
	{
		delete patgen;
		delete par;
		delete sccx;
		delete bgcol;
	}

	void DataReader::sim()
	{
		sccx->sim();
		par->sim();
		patgen->sim();
		par->sim_PARInputs();
		par->sim_PAROutputs();
		bgcol->sim();
	}
}
