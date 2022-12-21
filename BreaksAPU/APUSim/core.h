// 6502 Core Binding

#pragma once

namespace APUSim
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

		APU* apu = nullptr;

		BaseLogic::FF CLK_FF{};
		DIV_SRBit div[6]{};

		void sim_DividerBeforeCore();
		void sim_DividerAfterCore();

	public:
		CoreBinding(APU* parent);
		~CoreBinding();

		void sim();
	};
}
