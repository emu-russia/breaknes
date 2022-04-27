// Module for maintaining a simulated PPU environment.

#pragma once

namespace PPUPlayer
{
	class Board
	{
		PPUSim::PPU* ppu = nullptr;

		LS::LS373 latch;
		Breaknes::SRAM *vram = nullptr;

		BaseLogic::TriState CLK = BaseLogic::TriState::Zero;

		uint8_t ext_bus = 0;
		uint8_t data_bus = 0;
		uint8_t ad_bus = 0;
		bool ADDirty = false;
		uint8_t pa8_13 = 0;

		bool pendingCpuOperation = false;
		bool pendingWrite = false;
		size_t ppuRegId = 0;
		uint8_t writeValue = 0;
		size_t savedPclk = 0;

		NROM* cart = nullptr;
		BaseLogic::TriState n_VRAM_CS = BaseLogic::TriState::X;
		BaseLogic::TriState VRAM_A10 = BaseLogic::TriState::X;

		PPUSim::VideoOutSignal vidSample;

		static uint8_t DumpVRAM(void* opaque, size_t addr);
		static uint8_t DumpCHR(void* opaque, size_t addr);

		static uint32_t GetPpuDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry);

		void AddBoardMemDescriptors();
		void AddCartMemDescriptors();
		void AddDebugInfoProviders();

	public:
		Board();
		~Board();

		void Step();

		void CPUWrite(size_t ppuReg, uint8_t val);

		void CPURead(size_t ppuReg);

		size_t GetPCLKCounter();

		void InsertCartridge(uint8_t* nesImage, size_t nesImageSize);

		void EjectCartridge();

		void SampleVideoSignal(float *sample);

		size_t GetHCounter();

		size_t GetVCounter();
	};
}
