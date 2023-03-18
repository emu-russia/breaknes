// H/V Counters

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
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
		assert(bits <= bitCountMax);

		ppu = parent;
		bitCount = bits;

		for (size_t n = 0; n < bitCount; n++)
		{
			bit[n] = new HVCounterBit(parent);
		}

		fast_bits_mask = (1ULL << bitCount) - 1;
	}

	HVCounter::~HVCounter()
	{
		for (size_t n = 0; n < bitCount; n++)
		{
			delete bit[n];
		}
	}

	void HVCounter::sim(TriState Carry, TriState CLR)
	{
		if (fast_hv) {
			TriState n_PCLK = ppu->wire.n_PCLK;
			TriState RES = ppu->wire.RES;

			if (n_PCLK) {
				// Keep
				fast_bits = RES == TriState::One ? 0 : fast_bits;
			}
			else {
				// Count
				if (Carry == TriState::One) {
					fast_bits++;
				}
				if (CLR == TriState::One) {
					fast_bits = 0;
				}
			}
		}
		else {
			for (size_t n = 0; n < bitCount; n++)
			{
				Carry = bit[n]->sim(Carry, CLR);
			}
		}
	}

	size_t HVCounter::get()
	{
		size_t val = 0;

		if (fast_hv) {
			val = fast_bits;
		}
		else {
			for (size_t n = 0; n < bitCount; n++)
			{
				auto bitVal = bit[n]->getOut();
				if (bitVal == TriState::One)
				{
					val |= (1ULL << n);
				}
			}
		}

		return val;
	}

	void HVCounter::set(size_t val)
	{
		if (fast_hv) {
			fast_bits = val & fast_bits_mask;
		}
		else {
			for (size_t n = 0; n < bitCount; n++)
			{
				auto bitVal = (val >> n) & 1 ? TriState::One : TriState::Zero;
				bit[n]->set(bitVal);
			}
		}
	}

	TriState HVCounter::getBit(size_t n)
	{
		if (fast_hv) {
			return FromByte((fast_bits >> n) & 1);
		}
		else {
			return bit[n]->getOut();
		}
	}
}
