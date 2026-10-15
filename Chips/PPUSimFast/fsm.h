// PPU FSM

#pragma once

namespace PPUSimFast
{
	class FSM
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		uint8_t delayed_h_input = 0, delayed_h_output = 0;
		uint32_t horizontal_input = 0, horizontal_output = 0;
		uint8_t horizontal_sr = 0;

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

		// PAL PPU only.
		BaseLogic::DLatch EvenOdd_latch1;
		BaseLogic::DLatch EvenOdd_latch2;
		BaseLogic::DLatch EvenOdd_latch3;

		BaseLogic::DLatch ctrl_latch1;
		BaseLogic::DLatch ctrl_latch2;

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_DelayedH(EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_HPosLogic(uint32_t HPLA, uint16_t VPLA, EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_VSYNCEarly(uint16_t VPLA, EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_VPosLogic(uint16_t VPLA, EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_VBlankInt(EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_EvenOdd(uint32_t HPLA, uint16_t VPLA, EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_CountersControl(uint32_t HPLA, uint16_t VPLA, EdgeContextT<Phase>& c);
		// Field-driven entry points for the kernel tests: capture the context from the stored wires.
		void sim_DelayedH();
		void sim_HPosLogic(uint32_t HPLA, uint16_t VPLA);

		uint32_t prev_hpla = 0;
		uint16_t prev_vpla = 0;

		BaseLogic::TriState Prev_n_OBCLIP = BaseLogic::TriState::X;
		BaseLogic::TriState Prev_n_BGCLIP = BaseLogic::TriState::X;
		BaseLogic::TriState Prev_BLACK = BaseLogic::TriState::X;

	public:
		FSM(PPU *parent);
		~FSM();

		// Produces the FSM commands, delayed-H lines, Even/Odd and counter controls into `c`
		// (and into PPU::fsm / PPU::wire for the debug accessors); RESCL is taken from `c`.
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim(uint32_t HPLA, uint16_t VPLA, EdgeContextT<Phase>& c);

		// These methods are called BEFORE the FSM simulation, by consumer circuits.

		void sim_RESCL_early();
		BaseLogic::TriState get_VB();
		BaseLogic::TriState get_BLNK(BaseLogic::TriState BLACK);

		/// <summary>
		/// Get the VBlank status flag (the value that is read as bit 7 of $2002).
		/// </summary>
		BaseLogic::TriState get_VBLFlag();
	};
}
