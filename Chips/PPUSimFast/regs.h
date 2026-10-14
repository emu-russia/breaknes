// Control Registers

#pragma once

namespace PPUSimFast
{
	class ControlRegs
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		BaseLogic::DLatch nvis_latch;
		BaseLogic::DLatch clipb_latch;
		BaseLogic::DLatch clipo_latch;

		BaseLogic::FF SCCX_FF1;
		BaseLogic::FF SCCX_FF2;

		BaseLogic::TriState n_W56 = BaseLogic::TriState::X;

		uint8_t ctrl0 = 0, ctrl1 = 0, latched0 = 0, latched1 = 0;
		uint8_t previous_decode = 0xff, previous_w56 = 0xff;
		uint64_t previous_exports = static_cast<uint64_t>(-1);
		BaseLogic::TriState base_black = BaseLogic::Zero;

		// For PAL-like PPUs
		BaseLogic::FF BLACK_FF1;
		BaseLogic::FF BLACK_FF2;
		BaseLogic::DLatch black_latch1;
		BaseLogic::DLatch black_latch2;
		BaseLogic::DLatch vbl_latch;

		void sim_RegularRegOps();
		void sim_W56RegOps();
		void sim_FirstSecond_SCCX_Write();
		void sim_RegFFs();

		BaseLogic::TriState get_Frst();
		BaseLogic::TriState get_Scnd();

		bool RenderAlwaysEnabled = false;
		bool ClippingAlwaysDisabled = false;

		void sim_PalBLACK();

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
		void Debug_SetCTRL0(uint8_t val);
		void Debug_SetCTRL1(uint8_t val);

		BaseLogic::TriState get_nSLAVE();
	};
}
