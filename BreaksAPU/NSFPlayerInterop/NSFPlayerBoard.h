// Module for maintaining a simulated APU environment.

#pragma once

namespace NSFPlayer
{
	struct BoardDebugInfo
	{
		uint32_t CLK;
	};

	class Board
	{
		APUSim::APU* apu = nullptr;
		M6502Core::M6502* core = nullptr;
		BankedSRAM* sram = nullptr;

		BaseLogic::TriState CLK = BaseLogic::TriState::Zero;

		uint8_t data_bus = 0;

		bool pendingReset = false;
		int resetHalfClkCounter = 0;

		BaseLogic::TriState n_INT = BaseLogic::TriState::X;

		static uint8_t DumpSRAM(void* opaque, size_t addr);

		static void WriteSRAM(void* opaque, size_t addr, uint8_t data);

		static uint32_t GetCoreDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);
		static uint32_t GetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);
		static uint32_t GetApuDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);
		static uint32_t GetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);
		static uint32_t GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);

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
	};
}
