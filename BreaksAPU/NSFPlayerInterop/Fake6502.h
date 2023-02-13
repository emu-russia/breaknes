// 6502, which can only simulate reading and writing APU registers from the dump history

#pragma once

namespace M6502Core
{
#pragma pack(push, 1)
	struct APULogEntry
	{
		uint32_t	phiDelta;	// Delta of previous PHI0 counter (CPU Core clock cycles) value at the time of accessing to the register
		uint8_t 	reg; 		// APU register index (0-0x1f) + Flag (msb - 0: write, 1: read)
		uint8_t 	value;		// Written value. Not used for reading.
		uint16_t	padding;	// Not used (yet?)
	};
#pragma pack(pop)

	class FakeM6502 : public M6502
	{
		uint8_t* regdump = nullptr;
		size_t regdump_size = 0;
		size_t regdump_entry = 0;
		size_t regdump_entry_max = 0;
		size_t phi_counter = 0;
		size_t next_phi = 0;

		BaseLogic::TriState PrevPHI0 = BaseLogic::TriState::X;

		APULogEntry* GetCurrentEntry();

	public:
		FakeM6502();
		~FakeM6502();

		void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus);

		void SetRegDump(void* ptr, size_t size);
	};
}
