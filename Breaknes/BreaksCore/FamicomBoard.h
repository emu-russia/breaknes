#pragma once

// Historically, the NES board was made first, so the Famicom is partially similar in implementation.
// Let's try to do without the debugging mechanisms for the sake of purity. If anything, all debugging can be done on the NES board

namespace Breaknes
{
	class FamicomBoard;

	class FamicomBoardIO : public IO::IOSubsystem
	{
		FamicomBoard* base;
	public:
		FamicomBoardIO(FamicomBoard *board);
		virtual ~FamicomBoardIO();
		int GetPorts() override;
		void GetPortSupportedDevices(int port, std::list<IO::DeviceID>& devices) override;
		void sim(int port) override;
	};

	class FamicomBoard : public Board
	{
		friend FamicomBoardIO;

		BaseBoard::SRAM* wram = nullptr;
		const size_t wram_bits = 11;
		const size_t wram_size = 1ULL << wram_bits;
		uint32_t WRAM_Addr = 0;		// board -> sram

		BaseBoard::SRAM* vram = nullptr;
		const size_t vram_bits = 11;
		const size_t vram_size = 1ULL << vram_bits;
		uint32_t VRAM_Addr = 0;		// board -> sram

		BaseBoard::LS139 DMX;		// DMX acts as the north bridge. Such a small and simple bridge, and what did you want for the early 80's?
		BaseLogic::TriState nY1[4]{};			// DMX Stage1 output
		BaseLogic::TriState nY2[4]{};			// DMX Stage2 output

		BaseBoard::LS373 PPUAddrLatch;
		uint8_t LatchedAddr = 0;

		// Famicom Board specific ⚠️
		// TBD: Expansion port
		BaseLogic::TriState p2_nirq = BaseLogic::TriState::Z;
		float p2_sound = 0.0f;
		BaseLogic::TriState p2_4017_data[4]{};	// 4:1
		BaseLogic::TriState p2_4016_data{};		// d1

		// Famicom Board specific ⚠️
		Mappers::CartAudioOutSignal cart_snd{};

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

		// Famicom Board specific I/O ⚠️
		float mic_level = 0.0f;
		BaseBoard::LS368 P4_IO;
		BaseBoard::LS368 P5_IO;
		BaseLogic::TriState nRDP0 = BaseLogic::TriState::X;
		BaseLogic::TriState nRDP1 = BaseLogic::TriState::X;
		BaseLogic::TriState OUT_0 = BaseLogic::TriState::X;
		BaseLogic::TriState OUT_1 = BaseLogic::TriState::X;
		BaseLogic::TriState OUT_2 = BaseLogic::TriState::X;
		BaseLogic::TriState p4_inputs[(size_t)BaseBoard::LS368_Input::Max]{};
		BaseLogic::TriState p4_outputs[(size_t)BaseBoard::LS368_Output::Max]{};
		BaseLogic::TriState p5_inputs[(size_t)BaseBoard::LS368_Input::Max]{};
		BaseLogic::TriState p5_outputs[(size_t)BaseBoard::LS368_Output::Max]{};
		BaseLogic::TriState p4016_d0;
		BaseLogic::TriState p4017_d0;
		bool io_enabled = false;

		bool pendingReset = false;
		int resetHalfClkCounter = 0;

		void IOBinding();
		void SetDataBusIfNotFloating(size_t n, BaseLogic::TriState val);

	public:
		FamicomBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, Mappers::ConnectorType p1);
		virtual ~FamicomBoard();

		void Step() override;

		void Reset() override;

		bool InResetState() override;

		void SampleAudioSignal(float* sample);
	};
}
