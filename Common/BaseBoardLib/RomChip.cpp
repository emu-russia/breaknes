// A JEDEC-style ROM chip.

// https://github.com/emu-russia/breaks/blob/master/Docs/ROMs/ (see JEDEC standard pinouts)

#include "pch.h"

using namespace BaseLogic;

namespace BaseBoard
{
	RomChip::RomChip(const char* entity, size_t bits, bool trace)
	{
		memSize = 1LL << bits;
		mem = new uint8_t[memSize];
		memset(mem, 0, memSize);
		do_trace = trace;
		strcpy(rom_name, entity);
	}

	RomChip::~RomChip()
	{
		delete[] mem;
	}

	void RomChip::LoadImage(const uint8_t* image, size_t imageSize)
	{
		size_t n = imageSize < memSize ? imageSize : memSize;
		memcpy(mem, image, n);
	}

	void RomChip::sim(TriState n_CE, TriState n_OE, uint32_t* addr, uint8_t* data, bool& dz)
	{
		if (n_CE == TriState::Zero && n_OE == TriState::Zero)
		{
			// Address is masked to the chip capacity (2^bits).
			uint32_t a = *addr & (uint32_t)(memSize - 1);

			if (do_trace) {
				printf("%s Read 0x%x = 0x%02x\n", rom_name, a, mem[a]);
			}

			*data = mem[a];
			dz = false;
		}
	}

	size_t RomChip::Dbg_GetSize()
	{
		return memSize;
	}

	uint8_t RomChip::Dbg_ReadByte(size_t addr)
	{
		// Mirror out-of-range addresses (all ROM sizes are powers of two), the
		// same way the old mapper implementations masked the PRG address.
		return mem[addr & (memSize - 1)];
	}

	void RomChip::Dbg_WriteByte(size_t addr, uint8_t data)
	{
		mem[addr & (memSize - 1)] = data;
	}
}
