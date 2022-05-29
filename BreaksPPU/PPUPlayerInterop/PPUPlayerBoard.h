// Module for maintaining a simulated PPU environment.

#pragma once

namespace PPUPlayer
{
	struct BoardDebugInfo
	{
		uint32_t CLK;
		uint32_t ALE;
		uint32_t LS373_Latch;
		uint32_t VRAM_Addr;
		uint32_t n_VRAM_CS;
		uint32_t VRAM_A10;
		uint32_t PA;
		uint32_t n_PA13;
		uint32_t n_RD;
		uint32_t n_WR;
		uint32_t n_INT;
		uint32_t PD;
	};

	struct RGB_Triplet
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};

	class Board
	{
		PPUSim::PPU* ppu = nullptr;

		BaseBoard::LS373 latch;
		BaseBoard::SRAM *vram = nullptr;

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

		bool pendingReset = false;
		int resetHalfClkCounter = 0;

		NROM* cart = nullptr;
		BaseLogic::TriState n_INT = BaseLogic::TriState::X;
		BaseLogic::TriState n_VRAM_CS = BaseLogic::TriState::X;
		BaseLogic::TriState VRAM_A10 = BaseLogic::TriState::X;
		BaseLogic::TriState ALE = BaseLogic::TriState::X;
		BaseLogic::TriState n_RD = BaseLogic::TriState::X;
		BaseLogic::TriState n_WR = BaseLogic::TriState::X;
		BaseLogic::TriState PA[14]{};
		BaseLogic::TriState n_PA13 = BaseLogic::TriState::X;
		uint8_t LatchedAddress = 0;
		uint32_t VRAM_Addr = 0;

		PPUSim::VideoOutSignal vidSample;

		static uint8_t DumpVRAM(void* opaque, size_t addr);
		static uint8_t DumpCHR(void* opaque, size_t addr);
		static uint8_t DumpCRAM(void* opaque, size_t addr);
		static uint8_t DumpOAM(void* opaque, size_t addr);
		static uint8_t DumpTempOAM(void* opaque, size_t addr);

		static void WriteVRAM(void* opaque, size_t addr, uint8_t data);
		static void WriteCHR(void* opaque, size_t addr, uint8_t data);
		static void WriteCRAM(void* opaque, size_t addr, uint8_t data);
		static void WriteOAM(void* opaque, size_t addr, uint8_t data);
		static void WriteTempOAM(void* opaque, size_t addr, uint8_t data);

		static uint32_t GetPpuDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);
		static uint32_t GetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);
		static uint32_t GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);
		static uint32_t GetCartDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits);

		void AddBoardMemDescriptors();
		void AddCartMemDescriptors();
		void AddDebugInfoProviders();
		void AddCartDebugInfoProviders();

		void GetDebugInfo(BoardDebugInfo& info);

		RGB_Triplet* pal = nullptr;
		bool pal_cached = false;

	public:
		Board(char* boardName, char* apu, char* ppu, char* p1);
		~Board();

		void Step();

		void CPUWrite(size_t ppuReg, uint8_t val);

		void CPURead(size_t ppuReg);

		size_t GetPCLKCounter();

		int InsertCartridge(uint8_t* nesImage, size_t nesImageSize);

		void EjectCartridge();

		void SampleVideoSignal(float *sample);

		size_t GetHCounter();

		size_t GetVCounter();

		void ResetPPU();

		bool PPUInResetState();

		void RenderAlwaysEnabled(bool enable);

		void GetSignalFeatures(PPUSim::VideoSignalFeatures* features);

		void ConvertRAWToRGB(uint16_t raw, uint8_t* r, uint8_t* g, uint8_t* b);

		void SetRAWColorMode(bool enable);
	};
}
