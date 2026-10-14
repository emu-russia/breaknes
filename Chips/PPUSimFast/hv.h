// H/V Counters

#pragma once

namespace PPUSimFast
{
	/// <summary>
	/// One stage of H/V counters.
	/// </summary>
	class HVCounterBit
	{
		PPU* ppu = nullptr;

		BaseLogic::FF ff;
		BaseLogic::DLatch latch;

	public:
		HVCounterBit(PPU* parent) { ppu = parent; }

		BaseLogic::TriState sim(BaseLogic::TriState Carry, BaseLogic::TriState CLR);
		BaseLogic::TriState getOut();
		void set(BaseLogic::TriState val);
	};

	/// <summary>
	/// Implementation of a full counter (H or V).
	/// This does not simulate the propagation delay carry optimization for the low-order bits of the counter, as is done in the real circuit.
	/// The `bits` constructor parameter specifies the bits of the counter.
	/// </summary>
	class HVCounter
	{
		PPU* ppu = nullptr;

		size_t bitCount = 0;
		uint16_t current = 0, latch = 0;
		uint16_t mask = 0;

	public:
		HVCounter(PPU* parent, size_t bits);
		~HVCounter();

		void sim(BaseLogic::TriState Carry, BaseLogic::TriState CLR);

		size_t get();
		void set(size_t val);

		BaseLogic::TriState getBit(size_t n);
	};
}
