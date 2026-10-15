// Scroll Registers (nesdev `t`)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	ScrollRegs::ScrollRegs(PPU* parent)
	{
		ppu = parent;
	}

	ScrollRegs::~ScrollRegs()
	{
	}

	void ScrollRegs::sim()
	{
		const uint8_t data = ppu->DB;
		if (ppu->wire.RC == TriState::One) { address = 0; fine_x = 0; }
		else if (ppu->wire.n_DBE == TriState::Zero)
		{
			if (ppu->wire.n_W0 == TriState::Zero)
				address = static_cast<uint16_t>((address & ~0x0c00) | ((data & 3) << 10));
			else if (ppu->wire.n_W5_1 == TriState::Zero)
			{
				fine_x = data & 7;
				address = static_cast<uint16_t>((address & ~0x001f) | (data >> 3));
			}
			else if (ppu->wire.n_W5_2 == TriState::Zero)
				address = static_cast<uint16_t>((address & ~0x73e0) | ((data & 7) << 12) | ((data & 0xf8) << 2));
			else if (ppu->wire.n_W6_1 == TriState::Zero)
				address = static_cast<uint16_t>((address & 0x00ff) | ((data & 0x3f) << 8));
			else if (ppu->wire.n_W6_2 == TriState::Zero)
				address = static_cast<uint16_t>((address & 0x7f00) | data);
		}
		published_fine_x = fine_x;
		published_address = address;
	}

	void SCC_FF::sim(TriState val_in, TriState n_DBE, TriState RC, TriState & val_out)
	{
		val_in = val_in == TriState::Z ? ff.get() : val_in;
		ff.set(NOR(NOT(MUX(n_DBE, val_in, ff.get())), RC));
		val_out = ff.get();
	}

	TriState SCC_FF::get()
	{
		return ff.get();
	}

	void SCC_FF::set(TriState val)
	{
		ff.set(val);
	}
}