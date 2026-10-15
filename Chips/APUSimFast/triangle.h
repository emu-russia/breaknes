// Triangle Channel

#pragma once

namespace APUSimFast
{
	class TriangleChan
	{
		friend APUSimUnitTest::UnitTest;
		APU* apu = nullptr;

		BaseLogic::TriState TCO = BaseLogic::TriState::Z;
		BaseLogic::TriState FOUT = BaseLogic::TriState::Z;
		BaseLogic::TriState n_FOUT = BaseLogic::TriState::X;
		BaseLogic::TriState LOAD = BaseLogic::TriState::X;
		BaseLogic::TriState STEP = BaseLogic::TriState::X;
		BaseLogic::TriState TSTEP = BaseLogic::TriState::X;

		RegisterBit lc_reg{};
		BaseLogic::FF Reload_FF{};
		BaseLogic::DLatch reload_latch1{};
		BaseLogic::DLatch reload_latch2{};
		BaseLogic::DLatch tco_latch{};

		uint8_t lin_reg = 0;
		PackedCounter<7, false> lin_cnt{};
		uint16_t freq_reg = 0;
		PackedCounter<11, false> freq_cnt{};
		BaseLogic::DLatch fout_latch{};
		PackedCounter<5, true> out_cnt{};

		void sim_Control();
		void sim_LinearReg();
		void sim_LinearCounter();
		void sim_FreqReg();
		void sim_FreqCounter();
		uint32_t sim_Output();

	public:
		TriangleChan(APU* parent);
		~TriangleChan();

		// Returns the 4-bit channel output (the original TRI_Out bits).
		uint32_t sim();
		BaseLogic::TriState get_LC();

		uint32_t Get_LinearReg();
		uint32_t Get_LinearCounter();
		uint32_t Get_FreqReg();
		uint32_t Get_FreqCounter();
		uint32_t Get_OutputCounter();

		void Set_LinearReg(uint32_t value);
		void Set_LinearCounter(uint32_t value);
		void Set_FreqReg(uint32_t value);
		void Set_FreqCounter(uint32_t value);
		void Set_OutputCounter(uint32_t value);
	};
}
