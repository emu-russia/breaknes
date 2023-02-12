// Timing Generator

#pragma once

namespace APUSim
{
	class SoftCLK_SRBit
	{
		BaseLogic::DLatch in_latch{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim(BaseLogic::TriState n_ACLK, BaseLogic::TriState F1, BaseLogic::TriState F2, BaseLogic::TriState sin);

		BaseLogic::TriState get_sout();
		BaseLogic::TriState get_nsout();
	};

	class CLKGen
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

		BaseLogic::DLatch phi1_latch{};
		BaseLogic::DLatch phi2_latch{};

		BaseLogic::TriState sin = BaseLogic::TriState::X;
		BaseLogic::TriState F1 = BaseLogic::TriState::X;
		BaseLogic::TriState F2 = BaseLogic::TriState::X;
		BaseLogic::TriState Z1 = BaseLogic::TriState::X;
		BaseLogic::TriState Z2 = BaseLogic::TriState::X;
		BaseLogic::TriState mode = BaseLogic::TriState::X;
		BaseLogic::TriState n_mode = BaseLogic::TriState::X;
		BaseLogic::TriState pla[6]{};

		BaseLogic::FF z_ff{};
		BaseLogic::DLatch z1{};
		BaseLogic::DLatch z2{};
		BaseLogic::DLatch md_latch{};
		BaseLogic::DLatch int_status{};
		BaseLogic::FF int_ff{};

		SoftCLK_SRBit lfsr[15]{};

		RegisterBit reg_mode{};
		RegisterBit reg_mask{};

		void sim_ACLK();
		void sim_SoftCLK_Mode();
		void sim_SoftCLK_Control();
		void sim_SoftCLK_PLA();
		void sim_SoftCLK_LFSR();

	public:
		CLKGen(APU* parent);
		~CLKGen();

		void sim();
	};
}
