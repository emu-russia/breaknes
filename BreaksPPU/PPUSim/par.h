// Picture Address Register

#pragma once

namespace PPUSim
{
	class PAR_CounterBit
	{
		BaseLogic::FF ff;
		BaseLogic::DLatch step_latch;

	public:
		void sim();

		void sim_res();
	};

	class PAR_LowBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim();
	};

	class PAR_HighBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim();
	};

	class PAR
	{
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

		PAR_CounterBit FVCounter[3]{};
		PAR_CounterBit NTHCounter;
		PAR_CounterBit NTVCounter;
		PAR_CounterBit TVCounter[5]{};
		PAR_CounterBit THCounter[5]{};

		PAR_LowBit par_lo[8]{};
		PAR_HighBit par_hi[6]{};

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

		BaseLogic::TriState NTHOut = BaseLogic::TriState::X;
		BaseLogic::TriState NTHO = BaseLogic::TriState::X;
		BaseLogic::TriState NTVOut = BaseLogic::TriState::X;
		BaseLogic::TriState NTVO = BaseLogic::TriState::X;

		BaseLogic::TriState PARR = BaseLogic::TriState::X;
		BaseLogic::TriState PAH = BaseLogic::TriState::X;
		BaseLogic::TriState PAL = BaseLogic::TriState::X;

		void sim_CountersControl();
		void sim_CountersCarry();
		void sim_Control();
		void sim_FVCounter();
		void sim_NTCounters();
		void sim_TVCounter();
		void sim_THCounter();
		void sim_PARInputs();
		void sim_PAR();

	public:
		PAR(PPU* parent);
		~PAR();

		void sim();
	};
}
