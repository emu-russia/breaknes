// Register Decoder

#pragma once

namespace APUSimFast
{
	class RegsDecoder
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

		BaseLogic::TriState pla[29]{};		// Decoder

		BaseLogic::TriState nREGWR = BaseLogic::TriState::X;
		BaseLogic::TriState nREGRD = BaseLogic::TriState::X;

		BaseLogic::DLatch lock_latch{};
		uint64_t decoded_key = 0;
		uint64_t decoded_epoch = 0;
		bool decoded = false;

		void sim_DebugLock();
		void sim_Predecode();
		void sim_Decoder();
		void sim_RegOps();

	public:
		RegsDecoder(APU *parent);
		~RegsDecoder();

		void sim();
		void sim_DebugRegisters();
	};
}
