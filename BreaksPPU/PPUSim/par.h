// Picture Address Register

#pragma once

namespace PPUSim
{
	class PAR_CounterBit
	{
		BaseLogic::FF ff;
		BaseLogic::DLatch step_latch;

	public:
		BaseLogic::TriState sim(BaseLogic::TriState Clock, BaseLogic::TriState Load, BaseLogic::TriState Step,
			BaseLogic::TriState val_in, BaseLogic::TriState carry_in,
			BaseLogic::TriState & val_out, BaseLogic::TriState & n_val_out);

		BaseLogic::TriState sim_res(BaseLogic::TriState Clock, BaseLogic::TriState Load, BaseLogic::TriState Step,
			BaseLogic::TriState val_in, BaseLogic::TriState carry_in, BaseLogic::TriState Reset,
			BaseLogic::TriState& val_out, BaseLogic::TriState& n_val_out);
	};

	class PAR_LowBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState PCLK, BaseLogic::TriState PARR, BaseLogic::TriState DB_PAR, BaseLogic::TriState PAL, BaseLogic::TriState F_AT,
			BaseLogic::TriState FAT_in, BaseLogic::TriState PAL_in, BaseLogic::TriState PAD_in, BaseLogic::TriState DB_in,
			BaseLogic::TriState& n_PAx);
	};

	class PAR_HighBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState PCLK, BaseLogic::TriState PARR, BaseLogic::TriState PAH, BaseLogic::TriState F_AT,
			BaseLogic::TriState FAT_in, BaseLogic::TriState PAH_in, BaseLogic::TriState PAD_in,
			BaseLogic::TriState & n_PAx);
	};

	class PAR
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		bool fast_par = true;

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

		BaseLogic::TriState FAT_in[14]{};
		BaseLogic::TriState PAR_in[14]{};
		BaseLogic::TriState PAD_in[14]{};

		void sim_CountersControl();
		void sim_CountersCarry();
		void sim_Control();
		void sim_FVCounter();
		void sim_NTCounters();
		void sim_TVCounter();
		void sim_THCounter();

		// Faster counters

		void sim_AllCountersFast();

		uint8_t fast_FVCounter = 0;
		uint8_t fast_NTHCounter = 0;
		uint8_t fast_NTVCounter = 0;
		uint8_t fast_TVCounter = 0;
		uint8_t fast_THCounter = 0;

	public:
		PAR(PPU* parent);
		~PAR();

		void sim();

		void sim_PARInputs();
		void sim_PAROutputs();
	};
}
