// 6502, which can only simulate reading and writing APU/PPU registers from the dump history

#pragma once

#include "RegDumpProcessor.h"

namespace M6502Core
{
	class FakeM6502 : public M6502
	{
		BaseBoard::RegDumpProcessor* rp = nullptr;

	public:
		FakeM6502(const char* target, uint16_t regs_base, uint16_t regs_mask);
		~FakeM6502();

		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus);

		void SetRegDump(void* ptr, size_t size);
	};
}
