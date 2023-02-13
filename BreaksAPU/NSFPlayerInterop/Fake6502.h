// 6502, which can only simulate reading and writing APU registers from the dump history

#pragma once

namespace M6502Core
{
	class FakeM6502 : public M6502
	{
	public:
		FakeM6502();
		~FakeM6502();

		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus);
	};
}
