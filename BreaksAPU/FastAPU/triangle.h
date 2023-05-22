// Triangle Channel

#pragma once

namespace FastAPU
{
	class TriangleChan
	{
		FastAPU* apu = nullptr;

		BaseLogic::TriState TCO = BaseLogic::TriState::X;
		BaseLogic::TriState n_FOUT = BaseLogic::TriState::X;
		BaseLogic::TriState LOAD = BaseLogic::TriState::X;
		BaseLogic::TriState STEP = BaseLogic::TriState::X;
		BaseLogic::TriState TSTEP = BaseLogic::TriState::X;

		int lc_reg{};
		BaseLogic::FF Reload_FF{};
		BaseLogic::DLatch reload_latch1{};
		BaseLogic::DLatch reload_latch2{};
		BaseLogic::DLatch tco_latch{};

		uint8_t lin_reg{};
		uint8_t lin_cnt{};
		uint16_t freq_reg{};
		uint16_t freq_cnt{};
		BaseLogic::DLatch fout_latch{};
		uint8_t out_cnt{};

		void sim_Control();
		void sim_LinearReg();
		void sim_LinearCounter();
		void sim_FreqReg();
		void sim_FreqCounter();
		void sim_Output();

	public:
		TriangleChan(FastAPU* parent);
		~TriangleChan();

		void sim();
		BaseLogic::TriState get_LC();
	};
}
