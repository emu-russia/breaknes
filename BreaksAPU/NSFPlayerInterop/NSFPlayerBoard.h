// Module for maintaining a simulated APU environment.

#pragma once

namespace NSFPlayer
{
	struct BoardDebugInfo
	{
		uint32_t CLK;
		uint32_t bank_reg[8];
		uint32_t load_addr;
	};

	class Board
	{
		APUSim::APU* apu = nullptr;
		M6502Core::M6502* core = nullptr;
		BankedSRAM* sram = nullptr;
		uint16_t sram_load_addr = 0;
		BaseBoard::SRAM* wram = nullptr;
		const size_t wram_bits = 11;
		const size_t wram_size = 1ULL << wram_bits;

		BaseLogic::TriState CLK = BaseLogic::TriState::Zero;

		uint8_t data_bus = 0;
		uint16_t addr_bus = 0;

		bool pendingReset = false;
		int resetHalfClkCounter = 0;

		static uint8_t DumpSRAM(void* opaque, size_t addr);
		static void WriteSRAM(void* opaque, size_t addr, uint8_t data);

		static uint8_t DumpWRAM(void* opaque, size_t addr);
		static void WriteWRAM(void* opaque, size_t addr, uint8_t data);

		static uint32_t GetCoreDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetApuDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry);

		static void SetCoreDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetApuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);

		void AddBoardMemDescriptors();
		void AddDebugInfoProviders();

		void GetDebugInfo(BoardDebugInfo& info);

		APUSim::AudioOutSignal aux{};

	public:
		Board(char* boardName, char* apu, char* ppu, char* p1);
		~Board();

		void Step();

		int InsertCartridge(uint8_t* nesImage, size_t nesImageSize);

		void EjectCartridge();

		void ResetAPU();

		bool APUInResetState();

		size_t GetACLKCounter();

		void SampleAudioSignal(float* sample);

		void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address);

		void EnableNSFBanking(bool enable);
	};
}
