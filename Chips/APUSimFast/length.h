// Length Counters

#pragma once

namespace APUSimFast
{
	class LengthCounter
	{
		friend APUSimUnitTest::UnitTest;
		APU* apu = nullptr;

		BaseLogic::DLatch reg_enable_latch{};
		BaseLogic::DLatch ena_latch{};
		BaseLogic::DLatch cout_latch{};
		BaseLogic::FF stat_ff{};
		BaseLogic::DLatch step_latch{};
		BaseLogic::TriState STEP{};


		uint8_t reload = 0;
		PackedCounter<8, false> cnt{};
		BaseLogic::TriState carry_out{};

		void sim_Control(size_t bit_ena, BaseLogic::TriState WriteEn, BaseLogic::TriState& LC_NoCount);
		void sim_Decoder();
		void sim_Counter(BaseLogic::TriState LC_CarryIn, BaseLogic::TriState WriteEn);

	public:
		LengthCounter(APU* parent);
		~LengthCounter();

		void sim(size_t bit_ena, BaseLogic::TriState WriteEn, BaseLogic::TriState LC_CarryIn, BaseLogic::TriState & LC_NoCount);

		uint8_t Debug_GetCnt();
		void Debug_SetCnt(uint8_t value);

		bool Debug_GetEnable();
		void Debug_SetEnable(bool enable);
	};
}
