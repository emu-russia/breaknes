// Timing Generator

#pragma once

namespace FastAPU
{
	class CLKGen
	{
		FastAPU* apu = nullptr;

		int phi1_latch{};
		int phi2_latch{};

		int shift_in{};
		int F1{};
		int F2{};
		int Z1{};
		int Z2{};
		int mode{};
		int n_mode{};
		int pla[6]{};

		int z_ff{};
		int z1{};
		int z2{};
		int md_latch{};
		int int_status{};
		int int_ff{};

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
