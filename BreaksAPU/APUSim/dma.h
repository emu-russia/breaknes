// Sprite DMA

#pragma once

namespace APUSim
{
	class DMACounterBit
	{
		APU* apu = nullptr;

	public:
		DMACounterBit(APU* parent) { apu = parent; }

		BaseLogic::TriState sim(BaseLogic::TriState Carry, BaseLogic::TriState Clear, BaseLogic::TriState Load, BaseLogic::TriState val);
		BaseLogic::TriState getOut();
	};

	class DMA
	{
		APU* apu = nullptr;

		static const uint16_t PPU_Addr = 0x2004;

		DMACounterBit* spr_lo[8];
		BaseLogic::DLatch spr_hi[8];

		BaseLogic::DLatch spre_latch;
		BaseLogic::DLatch nospr_latch;
		BaseLogic::DLatch dospr_latch;
		BaseLogic::FF StopDMA;
		BaseLogic::FF StartDMA;
		BaseLogic::FF DMADirToggle;

		BaseLogic::DLatch spr_buf[8];

	public:
		DMA(APU* parent);
		~DMA();

		void sim();

		void sim_DMA_Address();
		void sim_DMA_Control();
		void sim_DMA_Buffer();
		void sim_AddressMux();
	};
}
