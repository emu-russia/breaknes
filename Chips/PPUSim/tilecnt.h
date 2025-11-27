// Tile Counters (nesdev `v`)

#pragma once

namespace PPUSim
{
	class TileCounterBit
	{
		BaseLogic::FF ff;
		BaseLogic::DLatch step_latch;

	public:
		BaseLogic::TriState sim(BaseLogic::TriState Clock, BaseLogic::TriState Load, BaseLogic::TriState Step,
			BaseLogic::TriState val_in, BaseLogic::TriState carry_in,
			BaseLogic::TriState& val_out, BaseLogic::TriState& n_val_out);

		BaseLogic::TriState sim_res(BaseLogic::TriState Clock, BaseLogic::TriState Load, BaseLogic::TriState Step,
			BaseLogic::TriState val_in, BaseLogic::TriState carry_in, BaseLogic::TriState Reset,
			BaseLogic::TriState& val_out, BaseLogic::TriState& n_val_out);
	};

	class TileCnt
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		BaseLogic::DLatch w62_latch;
		BaseLogic::FF W62_FF1;
		BaseLogic::FF W62_FF2;
		BaseLogic::DLatch sccnt_latch;
		BaseLogic::DLatch eev_latch1;
		BaseLogic::DLatch eev_latch2;
		BaseLogic::DLatch tvz_latch1;
		BaseLogic::DLatch tvz_latch2;
		BaseLogic::DLatch tvstep_latch;

		TileCounterBit FVCounter[3]{};
		TileCounterBit NTHCounter;
		TileCounterBit NTVCounter;
		TileCounterBit TVCounter[5]{};
		TileCounterBit THCounter[5]{};

		BaseLogic::TriState THZB = BaseLogic::TriState::X;
		BaseLogic::TriState THZ = BaseLogic::TriState::X;
		BaseLogic::TriState TVZB = BaseLogic::TriState::X;
		BaseLogic::TriState TVZ = BaseLogic::TriState::X;
		BaseLogic::TriState FVZ = BaseLogic::TriState::X;

		BaseLogic::TriState TVLOAD = BaseLogic::TriState::X;
		BaseLogic::TriState THLOAD = BaseLogic::TriState::X;
		BaseLogic::TriState TVSTEP = BaseLogic::TriState::X;
		BaseLogic::TriState THSTEP = BaseLogic::TriState::X;

		BaseLogic::TriState NTHIN = BaseLogic::TriState::X;
		BaseLogic::TriState NTVIN = BaseLogic::TriState::X;
		BaseLogic::TriState FVIN = BaseLogic::TriState::X;
		BaseLogic::TriState TVIN = BaseLogic::TriState::X;
		BaseLogic::TriState THIN = BaseLogic::TriState::X;
		BaseLogic::TriState Z_TV = BaseLogic::TriState::X;

		BaseLogic::TriState NTHO = BaseLogic::TriState::X;
		BaseLogic::TriState NTVO = BaseLogic::TriState::X;

		void sim_CountersControl();
		void sim_CountersCarry();
		void sim_FVCounter();
		void sim_NTCounters();
		void sim_TVCounter();
		void sim_THCounter();

	public:
		TileCnt(PPU* parent);
		~TileCnt();

		void sim();
	};
}