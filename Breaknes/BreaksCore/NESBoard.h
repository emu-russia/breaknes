#pragma once

namespace Breaknes
{
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

		// TBD: CIC :=P

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
		BaseLogic::TriState nRST = BaseLogic::TriState::Z;
		BaseLogic::TriState nIRQ = BaseLogic::TriState::Z;
		BaseLogic::TriState nNMI = BaseLogic::TriState::Z;
		BaseLogic::TriState M2 = BaseLogic::TriState::X; 			// from cpu
		BaseLogic::TriState WRAM_nCE = BaseLogic::TriState::X;
		BaseLogic::TriState PPU_nCE = BaseLogic::TriState::X; 		// 0: Enable CPU/PPU I/F	
		BaseLogic::TriState nROMSEL = BaseLogic::TriState::X;
		BaseLogic::TriState VRAM_A10 = BaseLogic::TriState::X;
		BaseLogic::TriState VRAM_nCE = BaseLogic::TriState::X;
		BaseLogic::TriState PPU_nA13 = BaseLogic::TriState::X;		// To save millions of inverters inside the cartridges

		// I/O -- TBD :(

		BaseBoard::LS368 P4_IO;
		BaseBoard::LS368 P5_IO;

		BaseLogic::TriState nOE1 = BaseLogic::TriState::X; 		// aka nRDP0 from cpu
		BaseLogic::TriState nOE2 = BaseLogic::TriState::X; 		// aka nRDP1 from cpu
		BaseLogic::TriState OUT_0 = BaseLogic::TriState::X;
		BaseLogic::TriState OUT_1 = BaseLogic::TriState::X;
		BaseLogic::TriState OUT_2 = BaseLogic::TriState::X;

		bool pendingReset = false;
		int resetHalfClkCounter = 0;

	public:
		NESBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, ConnectorType p1);
		virtual ~NESBoard();

		void Step() override;

		void Reset() override;

		bool InResetState() override;
	};
}
