// PPU FSM

#pragma once

namespace PPUSim
{
	class FSM
	{
		PPU* ppu = nullptr;

		BaseLogic::DLatch h_latch1[6];
		BaseLogic::DLatch h_latch2[6];

		BaseLogic::DLatch fp_latch1;
		BaseLogic::DLatch fp_latch2;
		BaseLogic::DLatch sev_latch1;
		BaseLogic::DLatch sev_latch2;
		BaseLogic::DLatch clpo_latch1;
		BaseLogic::DLatch clpo_latch2;
		BaseLogic::DLatch clpb_latch1;
		BaseLogic::DLatch clpb_latch2;
		BaseLogic::DLatch hpos_latch1;
		BaseLogic::DLatch hpos_latch2;
		BaseLogic::DLatch eval_latch1;
		BaseLogic::DLatch eval_latch2;
		BaseLogic::DLatch eev_latch1;
		BaseLogic::DLatch eev_latch2;
		BaseLogic::DLatch ioam_latch1;
		BaseLogic::DLatch ioam_latch2;
		BaseLogic::DLatch paro_latch1;
		BaseLogic::DLatch paro_latch2;
		BaseLogic::DLatch nvis_latch1;
		BaseLogic::DLatch nvis_latch2;
		BaseLogic::DLatch fnt_latch1;
		BaseLogic::DLatch fnt_latch2;
		BaseLogic::DLatch ftb_latch1;
		BaseLogic::DLatch ftb_latch2;
		BaseLogic::DLatch fta_latch1;
		BaseLogic::DLatch fta_latch2;
		BaseLogic::DLatch fo_latch1;
		BaseLogic::DLatch fo_latch2;
		BaseLogic::DLatch fo_latch3;
		BaseLogic::DLatch fat_latch1;
		BaseLogic::DLatch bp_latch1;
		BaseLogic::DLatch bp_latch2;
		BaseLogic::DLatch hb_latch1;
		BaseLogic::DLatch hb_latch2;
		BaseLogic::DLatch cb_latch1;
		BaseLogic::DLatch cb_latch2;
		BaseLogic::DLatch sync_latch1;
		BaseLogic::DLatch sync_latch2;
		BaseLogic::FF FPORCH_FF;
		BaseLogic::FF BPORCH_FF;
		BaseLogic::FF HBLANK_FF;
		BaseLogic::FF BURST_FF;

		BaseLogic::DLatch vsync_latch1;
		BaseLogic::DLatch pic_latch1;
		BaseLogic::DLatch pic_latch2;
		BaseLogic::DLatch vset_latch1;
		BaseLogic::DLatch vb_latch1;
		BaseLogic::DLatch vb_latch2;
		BaseLogic::DLatch blnk_latch1;
		BaseLogic::DLatch vclr_latch1;
		BaseLogic::DLatch vclr_latch2;
		BaseLogic::FF VSYNC_FF;
		BaseLogic::FF PICTURE_FF;
		BaseLogic::FF VB_FF;
		BaseLogic::FF BLNK_FF;

		BaseLogic::DLatch edge_vset_latch1;
		BaseLogic::DLatch edge_vset_latch2;
		BaseLogic::DLatch db_latch;
		BaseLogic::FF INT_FF;

		BaseLogic::FF EvenOdd_FF1;
		BaseLogic::FF EvenOdd_FF2;

		BaseLogic::DLatch ctrl_latch1;
		BaseLogic::DLatch ctrl_latch2;

		void sim_DelayedH();
		void sim_HPosLogic(BaseLogic::TriState* HPLA);
		void sim_VPosLogic(BaseLogic::TriState* VPLA);
		void sim_VBlankInt();
		void sim_EvenOdd(BaseLogic::TriState* HPLA);
		void sim_CountersControl(BaseLogic::TriState* HPLA, BaseLogic::TriState* VPLA);

	public:
		FSM(PPU *parent);
		~FSM();

		void sim(BaseLogic::TriState *HPLA, BaseLogic::TriState *VPLA);

		// These methods are called BEFORE the FSM simulation, by consumer circuits.

		BaseLogic::TriState get_RESCL();
		BaseLogic::TriState get_VB();
		BaseLogic::TriState get_BLNK(BaseLogic::TriState BLACK);
	};
}
