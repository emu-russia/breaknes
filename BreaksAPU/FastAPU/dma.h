// OAM DMA

#pragma once

namespace FastAPU
{
	class DMA
	{
		FastAPU* apu = nullptr;

		static const uint16_t PPU_Addr = 0x2004;

		uint8_t spr_lo{};
		uint8_t spr_hi{};

		int spre_latch{};
		int nospr_latch{};
		int dospr_latch{};
		int StopDMA{};
		int StartDMA{};
		int DMADirToggle{};

		uint8_t spr_buf{};

		int SPRE{};		// OAM counter overflow signal. Used to determine if an OAM DMA is complete.
		int SPRS{};		// A signal that enables OAM counter (low 8 bits of the address). The signal is silenced by RUNDMC.

		int NOSPR{};		// When NOSPR is 0 - the OAM DMA circuitry performs its activities to provide the OAM DMA process.
		int DOSPR{};		// Latches the OAM DMA start event
		int sprdma_rdy{};	// aka oamdma_rdy

		void sim_DMA_Address();
		void sim_DMA_Control();

	public:
		DMA(FastAPU* parent);
		~DMA();

		void sim();
		void sim_DMA_Buffer();
		void sim_AddressMux();
	};
}
