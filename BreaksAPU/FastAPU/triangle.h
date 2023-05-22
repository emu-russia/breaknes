// Triangle Channel

#pragma once

namespace FastAPU
{
	class TriangleChan
	{
		FastAPU* apu = nullptr;

		int TCO{};
		int n_FOUT{};
		int LOAD{};
		int STEP{};
		int TSTEP{};

		int lc_reg{};
		int Reload_FF{};
		int reload_latch1{};
		int reload_latch2{};
		int tco_latch{};

		uint8_t lin_reg{};
		uint8_t lin_cnt{};
		uint16_t freq_reg{};
		uint16_t freq_cnt{};
		int fout_latch{};
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
		int get_LC();
	};
}
