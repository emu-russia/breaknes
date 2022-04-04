// Multiplexer

#pragma once

namespace PPUSim
{
	class Mux
	{
		PPU* ppu = nullptr;

		BaseLogic::DLatch step1[4];
		BaseLogic::DLatch step2[4];
		BaseLogic::DLatch step3[4];
		BaseLogic::DLatch dir_color[4];

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
		void sim_Spr0Hit();

	public:
		Mux(PPU* parent) { ppu = parent; }
		~Mux() {}

		void sim();
	};
}
