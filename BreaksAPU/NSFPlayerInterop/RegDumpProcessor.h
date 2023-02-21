#pragma once

namespace NSFPlayer
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

	class RegDumpProcessor
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
		RegDumpProcessor();
		~RegDumpProcessor();

		void sim(BaseLogic::TriState PHI0, BaseLogic::TriState& RnW, uint16_t* addr_bus, uint8_t* data_bus);
		
		void SetRegDump(void* ptr, size_t size);
	};
}