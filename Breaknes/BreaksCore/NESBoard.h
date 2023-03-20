#pragma once

namespace Breaknes
{
	struct NESBoardDebugInfo
	{
		uint32_t CLK;
		uint32_t ABus;		// Board address bus
		uint32_t DBus;		// Board data bus
	};

	class NESBoard : public Board
	{
		BaseBoard::SRAM* wram = nullptr;
		const size_t wram_bits = 11;
		const size_t wram_size = 1ULL << wram_bits;
		uint32_t WRAM_Addr = 0;		// board -> sram

		BaseBoard::SRAM* vram = nullptr;
		const size_t vram_bits = 11;
		const size_t vram_size = 1ULL << vram_bits;
		uint32_t VRAM_Addr = 0;		// board -> sram

		BaseBoard::LS139 DMX;
		BaseLogic::TriState nY1[4]{};			// DMX Stage1 output
		BaseLogic::TriState nY2[4]{};			// DMX Stage2 output

		BaseBoard::LS373 PPUAddrLatch;
		uint8_t LatchedAddr = 0;

		// NES Board specific ⚠️
		// TBD: CIC :=P

		// NES Board specific ⚠️
		// TBD: Expansion port
		
		uint16_t exp_bus = 0;

		// PPU Buses
		
		uint8_t ext_bus = 0;			// Unused (grounded)
		uint8_t ad_bus = 0;				// Multiplexed data/addr
		bool ADDirty = false;
		uint8_t pa8_13 = 0;				// addr high bits
		uint16_t ppu_addr = 0;			// To cartridge

		// Other wires
		
		BaseLogic::TriState CPU_RnW = BaseLogic::TriState::X;
		BaseLogic::TriState PPU_nRD = BaseLogic::TriState::X;
		BaseLogic::TriState PPU_nWR = BaseLogic::TriState::X;
		BaseLogic::TriState PPU_ALE = BaseLogic::TriState::X;
		BaseLogic::TriState nIRQ = BaseLogic::TriState::Z;
		BaseLogic::TriState nNMI = BaseLogic::TriState::Z;
		BaseLogic::TriState M2 = BaseLogic::TriState::X; 			// from cpu
		BaseLogic::TriState WRAM_nCE = BaseLogic::TriState::X;
		BaseLogic::TriState PPU_nCE = BaseLogic::TriState::X; 		// 0: Enable CPU/PPU I/F	
		BaseLogic::TriState nROMSEL = BaseLogic::TriState::X;
		BaseLogic::TriState VRAM_A10 = BaseLogic::TriState::X;
		BaseLogic::TriState VRAM_nCE = BaseLogic::TriState::X;
		BaseLogic::TriState PPU_nA13 = BaseLogic::TriState::X;		// To save millions of inverters inside the cartridges

		// NES Board specific ⚠️
		// I/O -- TBD :(

		BaseBoard::LS368 P4_IO;
		BaseBoard::LS368 P5_IO;

		BaseLogic::TriState nOE1 = BaseLogic::TriState::X; 		// aka nRDP0 from cpu
		BaseLogic::TriState nOE2 = BaseLogic::TriState::X; 		// aka nRDP1 from cpu
		BaseLogic::TriState OUT_0 = BaseLogic::TriState::X;
		BaseLogic::TriState OUT_1 = BaseLogic::TriState::X;
		BaseLogic::TriState OUT_2 = BaseLogic::TriState::X;

		bool pendingReset_CPU = false;
		bool pendingReset_PPU = false;
		int resetHalfClkCounter_CPU = 0;
		int resetHalfClkCounter_PPU = 0;

#pragma region "Debug, look away"

		static uint8_t DumpWRAM(void* opaque, size_t addr);
		static uint8_t DumpVRAM(void* opaque, size_t addr);
		static uint8_t DumpCRAM(void* opaque, size_t addr);
		static uint8_t DumpOAM(void* opaque, size_t addr);
		static uint8_t DumpTempOAM(void* opaque, size_t addr);

		static void WriteWRAM(void* opaque, size_t addr, uint8_t data);
		static void WriteVRAM(void* opaque, size_t addr, uint8_t data);
		static void WriteCRAM(void* opaque, size_t addr, uint8_t data);
		static void WriteOAM(void* opaque, size_t addr, uint8_t data);
		static void WriteTempOAM(void* opaque, size_t addr, uint8_t data);

		static uint32_t GetCoreDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetApuDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetPpuDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry);

		static void SetCoreDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetApuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetPpuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);

		void AddBoardMemDescriptors();
		void AddDebugInfoProviders();

		void GetDebugInfo(NESBoardDebugInfo& info);

		void DumpCpuIF();

#pragma endregion "Debug, look away"

	public:
		NESBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, Mappers::ConnectorType p1);
		virtual ~NESBoard();

		void Step() override;

		void Reset() override;

		bool InResetState() override;
	};
}
