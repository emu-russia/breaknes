// Length Counters

#pragma once

namespace FastAPU
{
	class LengthCounter
	{
		FastAPU* apu = nullptr;

		int reg_enable_latch{};
		int ena_latch{};
		int cout_latch{};
		int stat_ff{};
		int step_latch{};
		int STEP{};

		uint8_t dec_latch{};
		BaseLogic::TriState Dec1_out[32]{};

		uint8_t LC{};
		uint8_t cnt{};
		int carry_out{};

		void sim_Control(size_t bit_ena, int WriteEn, int& LC_NoCount);
		void sim_Decoder1();
		void sim_Decoder2();
		void sim_Counter(int LC_CarryIn, int WriteEn);

	public:
		LengthCounter(FastAPU* parent);
		~LengthCounter();

		void sim(size_t bit_ena, int WriteEn, int LC_CarryIn, int& LC_NoCount);
	};
}
