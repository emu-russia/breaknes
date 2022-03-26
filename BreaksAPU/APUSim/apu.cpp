#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	APU::APU(M6502Core::M6502* _core, Revision _rev)
	{
		core = _core;
		rev = _rev;

		dma = new DMA(this);
	}

	APU::~APU()
	{
		delete dma;
	}

	void APU::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint8_t* data, uint16_t* addr, float* AUX_A, float* AUX_B)
	{

	}
}
