// 6502, which can only simulate reading and writing APU registers from the dump history

#pragma once

namespace NSFPlayer
{
	class APUPlayerBoard;
}

namespace M6502Core
{
	class FakeM6502 : public M6502
	{
		friend NSFPlayer::APUPlayerBoard;

		NSFPlayer::RegDumpProcessor* rp = nullptr;

	public:
		FakeM6502();
		~FakeM6502();

		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus);
	};
}
