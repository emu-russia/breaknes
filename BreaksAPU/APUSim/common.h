// Common elements of APU circuitry

#pragma once

namespace APUSim
{
	class RegisterBit
	{
		BaseLogic::DLatch transp_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState Enable, BaseLogic::TriState Value);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
	};

	class RegisterBitRes
	{
		BaseLogic::DLatch transp_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState Enable, BaseLogic::TriState Value, BaseLogic::TriState Res);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
	};

	class RegisterBitRes2
	{
		BaseLogic::DLatch transp_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState Enable, BaseLogic::TriState Value, BaseLogic::TriState Res1, BaseLogic::TriState Res2);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
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
			BaseLogic::TriState ACLK1,
			BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
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
			BaseLogic::TriState ACLK1,
			BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
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
			BaseLogic::TriState ACLK1,
			BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
	};
}
