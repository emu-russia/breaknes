// Scrolling Registers

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
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
		TriState n_DBE = ppu->wire.n_DBE;
		TriState RC = ppu->wire.RC;
		TriState W0 = NOR(ppu->wire.n_W0, n_DBE);
		TriState W5_1 = NOR(ppu->wire.n_W5_1, n_DBE);
		TriState W5_2 = NOR(ppu->wire.n_W5_2, n_DBE);
		TriState W6_1 = NOR(ppu->wire.n_W6_1, n_DBE);
		TriState W6_2 = NOR(ppu->wire.n_W6_2, n_DBE);

		// Make quick checks

		bool any_regops = (W0 == TriState::One) || (W5_1 == TriState::One) || (W5_2 == TriState::One) || (W6_1 == TriState::One) || (W6_2 == TriState::One);

		if (any_regops || RC == TriState::One || not_derived || ppu->DB != Prev_DB)
		{
			Prev_DB = ppu->DB;
			not_derived = false;
		}
		else
		{
			if (ppu->fast)
				return;
		}

		// FineH
		
		for (size_t n = 0; n < 3; n++)
		{
			FineH[n].sim(MUX(W5_1, TriState::Z, ppu->GetDBBit(n)), n_DBE, RC, ppu->wire.FH[n]);
		}

		// FineV

		for (size_t n = 0; n < 3; n++)
		{
			FineV[n].sim(MUX(W5_2, MUX(W6_1, TriState::Z, 
				n == 2 ? TriState::Zero : ppu->GetDBBit(4 + n) ), 
				ppu->GetDBBit(n)), n_DBE, RC, ppu->wire.FV[n]);
		}

		// NTV

		NTV.sim(MUX(W0, MUX(W6_1, TriState::Z, ppu->GetDBBit(3)), ppu->GetDBBit(1)), n_DBE, RC, ppu->wire.NTV);
		
		// NTH
		
		NTH.sim(MUX(W0, MUX(W6_1, TriState::Z, ppu->GetDBBit(2)), ppu->GetDBBit(0)), n_DBE, RC, ppu->wire.NTH);

		// TileV

		for (size_t n = 0; n < 5; n++)
		{
			TriState val_prev2 = MUX(W6_2, TriState::Z, n < 3 ? ppu->GetDBBit(5 + n) : TriState::Z);
			TriState val_prev = MUX(W6_1, val_prev2, n >= 3 ? ppu->GetDBBit(n - 3) : TriState::Z);
			TriState val_in = MUX(W5_2, val_prev, ppu->GetDBBit(3 + n));

			TileV[n].sim(val_in, n_DBE, RC, ppu->wire.TV[n]);
		}

		// TileH

		for (size_t n = 0; n < 5; n++)
		{
			TileH[n].sim(MUX(W5_1, MUX(W6_2, TriState::Z, ppu->GetDBBit(n)), ppu->GetDBBit(3 + n)), n_DBE, RC, ppu->wire.TH[n]);
		}
	}

	void SCC_FF::sim(TriState val_in, TriState n_DBE, TriState RC, TriState & val_out)
	{
		val_in = val_in == TriState::Z ? ff.get() : val_in;
		ff.set(NOR(NOT(MUX(n_DBE, val_in, ff.get())), RC));
		val_out = ff.get();
	}
}
