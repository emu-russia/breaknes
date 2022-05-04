// Control Registers

#pragma once

namespace PPUSim
{
	class ControlRegs
	{
		PPU* ppu = nullptr;

		BaseLogic::DLatch nvis_latch;
		BaseLogic::DLatch clipb_latch;
		BaseLogic::DLatch clipo_latch;

		BaseLogic::FF SCCX_FF1;
		BaseLogic::FF SCCX_FF2;

		BaseLogic::TriState n_W56 = BaseLogic::TriState::X;

		BaseLogic::FF PPU_CTRL0[8];
		BaseLogic::FF PPU_CTRL1[8];

		BaseLogic::DLatch i132_latch;
		BaseLogic::DLatch obsel_latch;
		BaseLogic::DLatch bgsel_latch;
		BaseLogic::DLatch o816_latch;

		BaseLogic::DLatch bgclip_latch;
		BaseLogic::DLatch obclip_latch;
		BaseLogic::DLatch bge_latch;
		BaseLogic::DLatch obe_latch;
		BaseLogic::DLatch tr_latch;
		BaseLogic::DLatch tg_latch;

		void sim_RegOps();
		void sim_FirstSecond_SCCX_Write();
		void sim_RegFFs();

		BaseLogic::TriState get_Frst();
		BaseLogic::TriState get_Scnd();

		bool RenderAlwaysEnabled = false;
		bool ClippingAlwaysDisabled = false;

		void trace();

	public:
		ControlRegs(PPU* parent);
		~ControlRegs();

		void sim_RWDecoder();
		void sim();
		void sim_CLP();

		void Debug_RenderAlwaysEnabled(bool enable);
		void Debug_ClippingAlwaysDisabled(bool enable);

		uint8_t Debug_GetCTRL0();

		uint8_t Debug_GetCTRL1();

		BaseLogic::TriState get_nSLAVE();
	};
}
