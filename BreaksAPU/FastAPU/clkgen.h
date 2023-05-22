// Timing Generator

#pragma once

namespace FastAPU
{
	class CLKGen
	{
		FastAPU* apu = nullptr;

		BaseLogic::DLatch phi1_latch{};
		BaseLogic::DLatch phi2_latch{};

		BaseLogic::TriState shift_in = BaseLogic::TriState::X;
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

		uint16_t lfsr{};

		int reg_mode{};
		int reg_mask{};

		void sim_ACLK();
		void sim_SoftCLK_Mode();
		void sim_SoftCLK_Control();
		void sim_SoftCLK_PLA();
		void sim_SoftCLK_LFSR();

	public:
		CLKGen(FastAPU* parent);
		~CLKGen();

		void sim();

		int GetINTFF();
	};
}
