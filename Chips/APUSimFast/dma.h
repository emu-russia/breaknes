// OAM DMA

#pragma once

namespace APUSimFast
{
	struct DMAState
	{
		PackedCounter<8, true> spr_lo{};
		uint8_t spr_hi = 0;

		// Direction, start, do-sprite, stop, no-sprite, prior address carry.
		uint8_t control = 0;

		uint8_t spr_buf = 0;

		BaseLogic::TriState prev_W4014 = BaseLogic::TriState::X;	// W4014 edge detection for logging (read before written)

	};

	class DMA : private DMAState
	{
		friend APUSimUnitTest::UnitTest;
		friend APU;		// the idle call performs the buffer capture/drive in place
		APU* apu = nullptr;

		static const uint16_t PPU_Addr = 0x2004;

		StablePhase<DMAState, uint32_t> phase;

		// Temporaries written before they are read on every evaluation (outside the snapshot;
		// see the DpcmChanState note). The stability snapshot is DMAState alone.
		BaseLogic::TriState SPRE = BaseLogic::TriState::X;		// OAM counter overflow signal. Used to determine if an OAM DMA is complete.
		BaseLogic::TriState SPRS = BaseLogic::TriState::X;		// A signal that enables OAM counter (low 8 bits of the address). The signal is silenced by RUNDMC.
		BaseLogic::TriState NOSPR = BaseLogic::TriState::X;		// When NOSPR is 0 - the OAM DMA circuitry performs its activities to provide the OAM DMA process.
		BaseLogic::TriState DOSPR = BaseLogic::TriState::X;		// Latches the OAM DMA start event
		BaseLogic::TriState sprdma_rdy = BaseLogic::TriState::X;	// aka oamdma_rdy

		void sim_DMA_Address();
		// Returns true when the binary transition path was taken (false on the X/Z scalar path).
		bool sim_DMA_Control();
#ifdef FASTSIM_SKIP_ORACLE
		void sim_SkipOracle();
#endif

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
		bool IsStable() const { return phase.IsStable(); }
	};
}
