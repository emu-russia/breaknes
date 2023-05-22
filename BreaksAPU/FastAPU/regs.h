// Register Decoder

#pragma once

namespace FastAPU
{
	class RegsDecoder
	{
		FastAPU* apu = nullptr;

		BaseLogic::TriState pla[29]{};		// Decoder

		BaseLogic::TriState nREGWR = BaseLogic::TriState::X;
		BaseLogic::TriState nREGRD = BaseLogic::TriState::X;

		BaseLogic::DLatch lock_latch{};

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
