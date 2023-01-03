// Sprite DMA

#pragma once

namespace APUSim
{
	class DMA
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

		static const uint16_t PPU_Addr = 0x2004;

		CounterBit spr_lo[8]{};
		RegisterBit spr_hi[8]{};

		BaseLogic::DLatch spre_latch{};
		BaseLogic::DLatch nospr_latch{};
		BaseLogic::DLatch dospr_latch{};
		BaseLogic::FF StopDMA{};
		BaseLogic::FF StartDMA{};
		BaseLogic::FF DMADirToggle{};

		BaseLogic::DLatch spr_buf[8]{};

		BaseLogic::TriState SPRE = BaseLogic::TriState::X;
		BaseLogic::TriState SPRS = BaseLogic::TriState::X;

		void sim_DMA_Address();
		void sim_DMA_Control();
		void sim_DMA_Buffer();
		void sim_AddressMux();

	public:
		DMA(APU* parent);
		~DMA();

		void sim();

		void Debug_Get(APU_Registers* info);
	};
}
