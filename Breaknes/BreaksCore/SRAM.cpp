// Generic SRAM

#include "pch.h"

using namespace BaseLogic;

namespace Breaknes
{
	SRAM::SRAM(size_t bits)
	{
		memSize = 1LL << bits;
		mem = new uint8_t[memSize];
		memset(mem, 0, memSize);
	}

	SRAM::~SRAM()
	{
		delete[] mem;
	}

	void SRAM::sim(TriState n_CS, TriState n_WE, TriState n_OE, uint32_t* addr, uint8_t* data, bool& dz)
	{
		dz = true;

		if (n_CS == TriState::Zero)
		{
			if (n_OE == TriState::Zero)
			{
				*data = mem[*addr];
				dz = false;
			}

			if (n_WE == TriState::Zero)
			{
				mem[*addr] = *data;
			}
		}
	}
}
