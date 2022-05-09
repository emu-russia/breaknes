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
		if (n_CS == TriState::Zero)
		{
			if (n_WE == TriState::Zero && !dz)
			{
				mem[*addr] = *data;
			}

			else if (n_OE == TriState::Zero && n_WE == TriState::One)
			{
				*data = mem[*addr];
				dz = false;
			}
		}
	}

	size_t SRAM::Dbg_GetSize()
	{
		return memSize;
	}

	uint8_t SRAM::Dbg_ReadByte(size_t addr)
	{
		if (addr < memSize)
		{
			return mem[addr];
		}
		else
		{
			return 0;
		}
	}
}
