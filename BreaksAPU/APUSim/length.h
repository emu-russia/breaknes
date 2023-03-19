// Length Counters

#pragma once

namespace APUSim
{
	class LengthCounter
	{
		friend APUSimUnitTest::UnitTest;
		APU* apu = nullptr;

		bool fast_lc = true;

		BaseLogic::DLatch reg_enable_latch{};
		BaseLogic::DLatch ena_latch{};
		BaseLogic::DLatch cout_latch{};
		BaseLogic::FF stat_ff{};
		BaseLogic::DLatch step_latch{};
		BaseLogic::TriState STEP{};

		BaseLogic::DLatch dec_latch[5]{};
		BaseLogic::TriState Dec1_out[32]{};

		BaseLogic::TriState LC[8]{};
		DownCounterBit cnt[8]{};
		BaseLogic::TriState carry_out{};

		void sim_Control(size_t bit_ena, BaseLogic::TriState WriteEn, BaseLogic::TriState& LC_NoCount);
		void sim_Decoder1();
		void sim_Decoder2();
		void sim_Counter(BaseLogic::TriState LC_CarryIn, BaseLogic::TriState WriteEn);

		// Faster

		size_t decoder_out[32] = {
			0x9, 0xfd, 0x13, 0x1, 0x27, 0x3, 0x4f, 0x5, 0x9f, 0x7, 0x3b, 0x9, 0xd, 0xb, 0x19, 0xd,
			0xb, 0xf, 0x17, 0x11, 0x2f, 0x13, 0x5f, 0x15, 0xbf, 0x17, 0x47, 0x19, 0xf, 0x1b, 0x1f, 0x1d,
		};

		uint8_t fast_down_cnt = 0;
		uint8_t packed_LC = 0;

		void sim_DecoderFast();
		void sim_CounterFast(BaseLogic::TriState LC_CarryIn, BaseLogic::TriState WriteEn);

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
