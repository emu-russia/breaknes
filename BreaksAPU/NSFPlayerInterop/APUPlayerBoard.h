// A special version of the board, which contains a bare APU and Fake6502, which can only read/write APU registers from the dump.

#pragma once

namespace NSFPlayer
{
	struct APUBoardDebugInfo
	{
		uint32_t CLK;
		uint32_t ABus;		// Board address bus
		uint32_t DBus;		// Board data bus
	};

	class APUPlayerBoard : public Board
	{
		M6502Core::FakeM6502* core = nullptr;
		BaseBoard::SRAM* wram = nullptr;
		const size_t wram_bits = 11;
		const size_t wram_size = 1ULL << wram_bits;

		static uint8_t DumpWRAM(void* opaque, size_t addr);
		static void WriteWRAM(void* opaque, size_t addr, uint8_t data);

		static uint32_t GetApuDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry);

		static void SetApuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);

		void AddBoardMemDescriptors();
		void AddDebugInfoProviders();

		void GetDebugInfo(APUBoardDebugInfo& info);

	public:
		APUPlayerBoard(char* boardName, char* apu, char* ppu, char* p1);
		virtual ~APUPlayerBoard();

		void Step() override;

		void ResetAPU(uint16_t addr, bool reset_apu_also) override;

		bool APUInResetState() override;

		void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address) override;

		void EnableNSFBanking(bool enable) override;

		void LoadRegDump(uint8_t* data, size_t data_size) override;
	};
}
