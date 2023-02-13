// 6502, which can only simulate reading and writing APU registers from the dump history

#include "pch.h"

namespace M6502Core
{
	FakeM6502::FakeM6502()
	{
	}

	FakeM6502::~FakeM6502()
	{
	}

	void FakeM6502::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus)
	{
	}
}
