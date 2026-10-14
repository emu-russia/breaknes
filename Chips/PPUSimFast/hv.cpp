// H/V Counters

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	TriState HVCounterBit::sim(TriState Carry, TriState CLR)
	{
		// The CLR actually makes sense as `Load`. But in the PPU all the inputs for the `Load` equivalent are connected to Vss, so we use the name CLR.

		ff.set( MUX(ppu->wire.PCLK, NOR(NOT(ff.get()), ppu->wire.RES), NOR(latch.get(), CLR)) );
		latch.set(MUX(Carry, NOT(ff.get()), ff.get()), NOT(ppu->wire.PCLK));
		TriState CarryOut = NOR(NOT(ff.get()), NOT(Carry));
		return CarryOut;
	}

	TriState HVCounterBit::getOut()
	{
		TriState out = NOR(NOT(ff.get()), ppu->wire.RES);
		return out;
	}

	void HVCounterBit::set(TriState val)
	{
		ff.set(val);
	}

	HVCounter::HVCounter(PPU* parent, size_t bits)
	{
		assert(bits > 0 && bits <= 16);
		ppu = parent; bitCount = bits;
		mask = static_cast<uint16_t>((1U << bits) - 1);
	}
	HVCounter::~HVCounter() {}
	void HVCounter::sim(TriState Carry, TriState CLR)
	{
		if (ppu->wire.PCLK == TriState::One)
			current = CLR == TriState::One ? 0 : static_cast<uint16_t>(~latch) & mask;
		else
		{
			if (ppu->wire.RES == TriState::One) current = 0;
			latch = static_cast<uint16_t>(~(current + (Carry == TriState::One ? 1U : 0U))) & mask;
		}
	}
	size_t HVCounter::get() { return ppu->wire.RES == TriState::One ? 0 : current; }
	void HVCounter::set(size_t value) { current = static_cast<uint16_t>(value) & mask; }
	TriState HVCounter::getBit(size_t n) { return static_cast<TriState>((get() >> n) & 1); }
}
