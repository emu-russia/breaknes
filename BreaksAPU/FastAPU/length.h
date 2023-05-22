// Length Counters

#pragma once

namespace FastAPU
{
	class LengthCounter
	{
		FastAPU* apu = nullptr;

		BaseLogic::DLatch reg_enable_latch{};
		BaseLogic::DLatch ena_latch{};
		BaseLogic::DLatch cout_latch{};
		BaseLogic::FF stat_ff{};
		BaseLogic::DLatch step_latch{};
		BaseLogic::TriState STEP{};

		BaseLogic::DLatch dec_latch[5]{};
		BaseLogic::TriState Dec1_out[32]{};

		uint8_t LC{};
		uint8_t cnt{};
		BaseLogic::TriState carry_out{};

		void sim_Control(size_t bit_ena, BaseLogic::TriState WriteEn, BaseLogic::TriState& LC_NoCount);
		void sim_Decoder1();
		void sim_Decoder2();
		void sim_Counter(BaseLogic::TriState LC_CarryIn, BaseLogic::TriState WriteEn);

	public:
		LengthCounter(FastAPU* parent);
		~LengthCounter();

		void sim(size_t bit_ena, BaseLogic::TriState WriteEn, BaseLogic::TriState LC_CarryIn, BaseLogic::TriState& LC_NoCount);
	};
}
