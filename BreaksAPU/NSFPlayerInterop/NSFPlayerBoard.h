// Module for maintaining a simulated APU environment.

#pragma once

namespace NSFPlayer
{
	struct NSFBoardDebugInfo
	{
		uint32_t CLK;
		uint32_t bank_reg[8];
		uint32_t sync;			// Core SYNC
		uint32_t ABus;		// Board address bus
		uint32_t DBus;		// Board data bus
		uint32_t ResetPending;
	};

	class NSFPlayerBoard : public Board
	{
		BankedSRAM* sram = nullptr;
		BaseBoard::SRAM* wram = nullptr;
		const size_t wram_bits = 11;
		const size_t wram_size = 1ULL << wram_bits;

		BaseLogic::TriState SYNC = BaseLogic::TriState::X;

		bool pendingReset = false;
		int resetHalfClkCounter = 0;
		bool reset_apu_also = false;

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

		void GetDebugInfo(NSFBoardDebugInfo& info);

	public:
		NSFPlayerBoard(char* boardName, char* apu, char* ppu, char* p1);
		virtual ~NSFPlayerBoard();

		void Step() override;

		int InsertCartridge(uint8_t* nesImage, size_t nesImageSize) override;

		void EjectCartridge() override;

		void ResetAPU(uint16_t addr, bool reset_apu_also) override;

		bool APUInResetState() override;

		size_t GetACLKCounter() override;

		size_t GetPHICounter() override;

		void SampleAudioSignal(float* sample) override;

		void LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address) override;

		void EnableNSFBanking(bool enable) override;

		void LoadRegDump(uint8_t* data, size_t data_size) override;

		void GetSignalFeatures(APUSim::AudioSignalFeatures* features) override;
	};
}
