// Register Decoder

#pragma once

namespace FastAPU
{
	class RegsDecoder
	{
		FastAPU* apu = nullptr;

		int nREGWR{};
		int nREGRD{};

		int lock_latch{};

		void sim_DebugLock();
		void sim_Predecode();
		void sim_Decoder();
		void sim_RegOps();

	public:
		RegsDecoder(FastAPU* parent);
		~RegsDecoder();

		void sim();
		void sim_DebugRegisters();
	};
}
