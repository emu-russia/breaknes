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
		BaseLogic::DLatch transp_latch{};
		BaseLogic::DLatch cg_latch{};

	public:
		BaseLogic::TriState sim(
			BaseLogic::TriState Carry,
			BaseLogic::TriState Clear,
			BaseLogic::TriState Load,
			BaseLogic::TriState Step,
			BaseLogic::TriState n_ACLK,
			BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
	};

	class DownCounterBit
	{
		BaseLogic::DLatch transp_latch{};
		BaseLogic::DLatch cg_latch{};

	public:
		BaseLogic::TriState sim(
			BaseLogic::TriState Carry,
			BaseLogic::TriState Clear,
			BaseLogic::TriState Load,
			BaseLogic::TriState Step,
			BaseLogic::TriState n_ACLK,
			BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
	};

	class RevCounterBit
	{
		BaseLogic::DLatch transp_latch{};
		BaseLogic::DLatch cg_latch{};

	public:
		BaseLogic::TriState sim(
			BaseLogic::TriState Carry,
			BaseLogic::TriState Dec,
			BaseLogic::TriState Clear,
			BaseLogic::TriState Load,
			BaseLogic::TriState Step,
			BaseLogic::TriState n_ACLK,
			BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
	};
}
