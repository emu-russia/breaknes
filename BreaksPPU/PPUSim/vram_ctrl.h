// VRAM Controller

#pragma once

namespace PPUSim
{
	class VRAM_Control
	{
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

		BaseLogic::TriState R7_EN = BaseLogic::TriState::Z;
		BaseLogic::TriState W7_EN = BaseLogic::TriState::Z;
		BaseLogic::TriState tmp_1 = BaseLogic::TriState::Z;
		BaseLogic::TriState tmp_2 = BaseLogic::TriState::Z;

		void sim_RD();
		void sim_WR();
		void sim_Misc();
		void sim_ALE();

	public:
		VRAM_Control(PPU* parent);
		~VRAM_Control();

		void sim();
	};
}
