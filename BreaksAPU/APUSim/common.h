// Common elements of APU circuitry

#pragma once

namespace APUSim
{
	class RegisterBit
	{
		BaseLogic::DLatch transp_latch{};

	public:
		void sim(BaseLogic::TriState n_ACLK, BaseLogic::TriState Enable, BaseLogic::TriState Value);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
	};

	class CounterBit
	{
	public:
		BaseLogic::TriState sim(BaseLogic::TriState Carry, BaseLogic::TriState Clear, BaseLogic::TriState Load, BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
	};
}
