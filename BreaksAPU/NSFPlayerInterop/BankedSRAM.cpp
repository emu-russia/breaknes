// A special type of memory with bank switching logic inside.

#include "pch.h"

namespace NSFPlayer
{
	BankedSRAM::BankedSRAM()
	{
	}

	BankedSRAM::~BankedSRAM()
	{
	}

	size_t BankedSRAM::Dbg_GetSize()
	{
		return 0;
	}

	uint8_t BankedSRAM::Dbg_ReadByte(size_t addr)
	{
		return 0;
	}

	void BankedSRAM::Dbg_WriteByte(size_t addr, uint8_t data)
	{

	}
}
