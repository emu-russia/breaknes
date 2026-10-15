// 6502 Core Binding

#pragma once

namespace APUSimFast
{
	class DIV_SRBit
	{
		BaseLogic::DLatch in_latch{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim(BaseLogic::TriState q, BaseLogic::TriState nq, BaseLogic::TriState rst, BaseLogic::TriState sin);

		BaseLogic::TriState get_sout(BaseLogic::TriState rst);
		BaseLogic::TriState get_nval();
	};

	class CoreBinding
	{
		friend APUSimUnitTest::UnitTest;
		friend APU;		// the idle call steps the divider banks in place

		APU* apu = nullptr;

		// Raw input latches, and logical inverted output latches.
		uint8_t divider_input = 0;
		uint8_t divider_output = 0x3f;

		void sim_DividerBeforeCore();
		void sim_DividerAfterCore();

	public:
		CoreBinding(APU* parent);
		~CoreBinding();

		void sim();

		// v4 idle path: PHI0 after the next divider step, computed without committing it. The
		// divider output bank only moves on the n_CLK != 0 half, exactly as sim_DividerBeforeCore.
		bool PeekPhi0Edge() const;
		bool core_called = false;
	};
}
