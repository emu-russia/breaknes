#pragma once

namespace BaseBoard
{
#pragma pack(push, 1)
	struct RegDumpEntry
	{
		uint32_t	clkDelta;	// Delta of previous "some CLK" counter value at the time of accessing to the register
		uint8_t 	reg; 		// Register index + Flag (msb - 0: write, 1: read)
		uint8_t 	value;		// Written value. Not used for reading.
		uint16_t	padding;	// Not used (yet?)
	};
#pragma pack(pop)

	class RegDumpProcessor
	{
		uint16_t regbase = 0x1000;	// e.g. 0x4000 - APU, 0x2000 - PPU
		uint16_t regmask = 0x7f;			// e.g. 0x1f - APU, 0x7 - PPU
		uint8_t* regdump = nullptr;
		size_t regdump_size = 0;
		size_t regdump_entry = 0;
		size_t regdump_entry_max = 0;
		size_t clk_counter = 0;
		size_t next_clk = 0;

		bool hold = false;				// Hold the register operation for the rest of the cycle
		RegDumpEntry hold_entry{};

		BaseLogic::TriState PrevCLK = BaseLogic::TriState::X;

		RegDumpEntry* GetCurrentEntry();

		const bool dump_regops = false;		// Enable debug output of register operations

		const bool dump_4015_writes_only = false;	// The most interesting

		char regdump_target[32];
		bool first_access = true;

	public:
		RegDumpProcessor(const char *target, uint16_t regs_base, uint16_t regs_mask);
		~RegDumpProcessor();

		void sim(BaseLogic::TriState CLK, BaseLogic::TriState n_RES, BaseLogic::TriState& RnW, uint16_t* addr_bus, uint8_t* data_bus);
		
		void SetRegDump(void* ptr, size_t size);
	};
}
