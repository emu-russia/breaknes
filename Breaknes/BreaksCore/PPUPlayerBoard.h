// Module for maintaining a simulated PPU environment.

#pragma once

namespace Breaknes
{
	struct PPUBoardDebugInfo
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
		uint32_t CPUOpsProcessed;
	};

	class DIV_SRBit
	{
		BaseLogic::DLatch in_latch{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim(BaseLogic::TriState q, BaseLogic::TriState nq, BaseLogic::TriState rst, BaseLogic::TriState sin);

		BaseLogic::TriState get_sout(BaseLogic::TriState rst);
		BaseLogic::TriState get_nval();
	};

	class PPUPlayerBoard : public Board
	{
		M6502Core::FakeM6502* core = nullptr;

		BaseBoard::LS373 latch;
		BaseBoard::SRAM *vram = nullptr;

		uint8_t ext_bus = 0;
		uint8_t ad_bus = 0;
		bool ADDirty = false;
		uint8_t pa8_13 = 0;

		bool pendingReset = false;
		int resetHalfClkCounter = 0;

		Mappers::NROM* cart = nullptr;
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

		static uint32_t GetPpuDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetCartDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry);

		static void SetPpuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetCartDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);

		void AddBoardMemDescriptors();
		void AddCartMemDescriptors();
		void AddDebugInfoProviders();
		void AddCartDebugInfoProviders();

		void GetDebugInfo(PPUBoardDebugInfo& info);

		void SimCoreDivider();
		BaseLogic::FF CLK_FF{};
		DIV_SRBit div[6]{};

		BaseLogic::TriState PHI0 = BaseLogic::TriState::X;

		bool prev_pendingCpuOperation = false;
		uint32_t CPUOpsProcessed = 0;

	public:
		PPUPlayerBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev);
		virtual ~PPUPlayerBoard();

		void Step() override;

		int InsertCartridge(uint8_t* nesImage, size_t nesImageSize) override;

		void EjectCartridge() override;

		void Reset() override;

		bool InResetState() override;

		void LoadRegDump(uint8_t* data, size_t data_size) override;
	};
}
