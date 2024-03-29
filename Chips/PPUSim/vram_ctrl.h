// VRAM Controller

#pragma once

namespace PPUSim
{
	class RB_Bit
	{
		PPU* ppu = nullptr;

		BaseLogic::FF ff;

	public:
		RB_Bit(PPU* parent) { ppu = parent; }
		~RB_Bit() {}

		void sim(size_t bit_num);

		BaseLogic::TriState get();
		void set(BaseLogic::TriState value);
	};

	class VRAM_Control
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		BaseLogic::DLatch wr_latch1;
		BaseLogic::DLatch wr_latch2;
		BaseLogic::DLatch wr_latch3;
		BaseLogic::DLatch wr_latch4;

		BaseLogic::DLatch rd_latch1;
		BaseLogic::DLatch rd_latch2;
		BaseLogic::DLatch rd_latch3;
		BaseLogic::DLatch rd_latch4;

		BaseLogic::FF W7_FF;
		BaseLogic::FF R7_FF;
		BaseLogic::FF WR_FF;
		BaseLogic::FF RD_FF;

		BaseLogic::DLatch tstep_latch;
		BaseLogic::DLatch h0_latch;
		BaseLogic::DLatch blnk_latch;

		BaseLogic::TriState tmp_1 = BaseLogic::TriState::Z;
		BaseLogic::TriState tmp_2 = BaseLogic::TriState::Z;

		RB_Bit *RB[8];

		void sim_RD();
		void sim_WR();
		void sim_ALE();

	public:
		VRAM_Control(PPU* parent);
		~VRAM_Control();

		void sim();

		void sim_TH_MUX();

		void sim_ReadBuffer();

		uint8_t Debug_GetRB();
		void Debug_SetRB(uint8_t value);
	};
}
