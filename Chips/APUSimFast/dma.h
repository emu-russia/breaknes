// OAM DMA

#pragma once

namespace APUSimFast
{
	struct DMAState
	{
		PackedCounter<8, true> spr_lo{};
		uint8_t spr_hi = 0;

		BaseLogic::DLatch spre_latch{};
		BaseLogic::DLatch nospr_latch{};
		BaseLogic::DLatch dospr_latch{};
		BaseLogic::FF StopDMA{};
		BaseLogic::FF StartDMA{};
		BaseLogic::FF DMADirToggle{};

		uint8_t spr_buf = 0;

		BaseLogic::TriState SPRE = BaseLogic::TriState::X;		// OAM counter overflow signal. Used to determine if an OAM DMA is complete.
		BaseLogic::TriState SPRS = BaseLogic::TriState::X;		// A signal that enables OAM counter (low 8 bits of the address). The signal is silenced by RUNDMC.

		BaseLogic::TriState NOSPR = BaseLogic::TriState::X;		// When NOSPR is 0 - the OAM DMA circuitry performs its activities to provide the OAM DMA process.
		BaseLogic::TriState DOSPR = BaseLogic::TriState::X;		// Latches the OAM DMA start event
		BaseLogic::TriState sprdma_rdy = BaseLogic::TriState::X;	// aka oamdma_rdy

		BaseLogic::TriState prev_W4014 = BaseLogic::TriState::X;	// W4014 edge detection for logging

	};

	class DMA : private DMAState
	{
		friend APUSimUnitTest::UnitTest;
		APU* apu = nullptr;

		static const uint16_t PPU_Addr = 0x2004;

		StablePhase<DMAState, 9> phase;

		void sim_DMA_Address();
		void sim_DMA_Control();

	public:
		DMA(APU* parent);
		~DMA();

		void sim();
		void sim_DMA_Buffer();
		void sim_AddressMux();

		uint32_t Get_DMABuffer();
		uint32_t Get_DMAAddress();

		void Set_DMABuffer(uint32_t value);
		void Set_DMAAddress(uint32_t value);
	};
}
