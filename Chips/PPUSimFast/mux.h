// Multiplexer

#pragma once

namespace PPUSimFast
{
	class Mux
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		uint8_t step1 = 0, step2 = 0, step3 = 0, dir_color = 0;
		uint8_t palette_address = 0;
		BaseLogic::DLatch zprio_latch;
		BaseLogic::DLatch bgc_latch;
		BaseLogic::DLatch zcol_latch;
		BaseLogic::DLatch ocol_latch;
		BaseLogic::DLatch tho4_latch;
		BaseLogic::DLatch pal4_latch;

		BaseLogic::FF StrikeFF;

		// Internal intermediate signals

		BaseLogic::TriState n_PAL4 = BaseLogic::TriState::X;
		BaseLogic::TriState OCOL = BaseLogic::TriState::X;
		BaseLogic::TriState EXT = BaseLogic::TriState::X;

		void sim_MuxControl();
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_Spr0Hit(const EdgeContextT<Phase>& c);

	public:
		Mux(PPU* parent) { ppu = parent; }
		~Mux() {}

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim(const EdgeContextT<Phase>& c);
		uint8_t get_Address() const { return palette_address; }
	};
}
