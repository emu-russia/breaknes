#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	APU::APU(M6502Core::M6502* _core, Revision _rev)
	{
		core = _core;
		rev = _rev;

		dma = new DMA(this);
		regs = new RegsDecoder(this);
	}

	APU::~APU()
	{
		delete dma;
		delete regs;
	}

	void APU::sim(TriState inputs[], TriState outputs[], uint8_t* data, uint16_t* addr, float* AUX_A, float* AUX_B)
	{

	}
}
