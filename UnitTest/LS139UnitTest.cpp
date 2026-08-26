#include "pch.h"
#include "../Common/BaseBoardLib/LS139.h"

using namespace BaseLogic;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	LS139_Test::LS139_Test()
	{
	}

	LS139_Test::~LS139_Test()
	{
	}

	bool LS139_Test::TestHalfTruthTable()
	{
		TriState y1[4]{}, y2[4]{};
		TriState nEN1, nEN2, a0, a1, b0, b1;

		// Half 1: enabled, Half 2: disabled (all its outputs go high)
		nEN1 = TriState::Zero;
		nEN2 = TriState::One;

		for (int sel = 0; sel < 4; sel++)
		{
			a0 = (sel & 1) ? TriState::One : TriState::Zero;
			a1 = (sel & 2) ? TriState::One : TriState::Zero;
			b0 = TriState::Zero;
			b1 = TriState::Zero;

			BaseBoard::LS139::sim(nEN1, nEN2, a0, a1, b0, b1, y1, y2);

			for (int k = 0; k < 4; k++)
			{
				TriState expected = (k == sel) ? TriState::Zero : TriState::One;
				if (y1[k] != expected)
				{
					return false;
				}

				if (y2[k] != TriState::One)
				{
					return false;
				}
			}
		}

		// Half 2: enabled, Half 1: disabled (all its outputs go high)
		nEN1 = TriState::One;
		nEN2 = TriState::Zero;

		for (int sel = 0; sel < 4; sel++)
		{
			a0 = TriState::Zero;
			a1 = TriState::Zero;
			b0 = (sel & 1) ? TriState::One : TriState::Zero;
			b1 = (sel & 2) ? TriState::One : TriState::Zero;

			BaseBoard::LS139::sim(nEN1, nEN2, a0, a1, b0, b1, y1, y2);

			for (int k = 0; k < 4; k++)
			{
				TriState expected = (k == sel) ? TriState::Zero : TriState::One;
				if (y1[k] != TriState::One)
				{
					return false;
				}

				if (y2[k] != expected)
				{
					return false;
				}
			}
		}

		return true;
	}

	bool LS139_Test::TestAllDisabled()
	{
		TriState y1[4]{}, y2[4]{};
		TriState nEN1 = TriState::One, nEN2 = TriState::One;
		TriState a0 = TriState::Zero, a1 = TriState::Zero;
		TriState b0 = TriState::Zero, b1 = TriState::Zero;

		BaseBoard::LS139::sim(nEN1, nEN2, a0, a1, b0, b1, y1, y2);

		for (int k = 0; k < 4; k++)
		{
			if (y1[k] != TriState::One || y2[k] != TriState::One)
			{
				return false;
			}
		}

		return true;
	}

	bool LS139_Test::TestInternalPullUp()
	{
		TriState y1[4]{}, y2[4]{};

		// Half 1 enabled; A1_0 floats (exactly like the M2 line during CPU reset).
		TriState nEN1 = TriState::Zero;
		TriState nEN2 = TriState::One;
		TriState a0 = TriState::Z;
		TriState a1 = TriState::Zero;
		TriState b0 = TriState::One;
		TriState b1 = TriState::One;

		BaseBoard::LS139::sim(nEN1, nEN2, a0, a1, b0, b1, y1, y2);

		// The referenced floating net must have been pulled up to '1' by the chip.
		if (a0 != TriState::One)
		{
			return false;
		}

		// Decode now sees (A1_0 = 1, A1_1 = 0) -> n_Y1[1] low, the rest high.
		if (y1[1] != TriState::Zero)
		{
			return false;
		}

		for (int k = 0; k < 4; k++)
		{
			if (k != 1 && y1[k] != TriState::One)
			{
				return false;
			}
		}

		// Half 2 is disabled -> all its outputs high.
		for (int k = 0; k < 4; k++)
		{
			if (y2[k] != TriState::One)
			{
				return false;
			}
		}

		return true;
	}

	namespace
	{
		// NES wiring (NESBoard::Step): DMX.sim(gnd, nY1[1], M2, A15, A13, A14, nY1, nY2)
		// Stage 2 is enabled by nY1[1] = (M2 && !A15) ? 0 : 1 (steady state).

		void RunNesDecode(TriState m2, TriState a15, TriState a13, TriState a14,
			TriState& nROMSEL, TriState& WRAM_nCE, TriState& PPU_nCE)
		{
			TriState nEN1 = TriState::Zero;
			TriState nEN2 = (m2 == TriState::One && a15 == TriState::Zero) ? TriState::Zero : TriState::One;
			TriState y1[4]{}, y2[4]{};

			BaseBoard::LS139::sim(nEN1, nEN2, m2, a15, a13, a14, y1, y2);

			nROMSEL = y1[3];
			WRAM_nCE = y2[0];
			PPU_nCE = y2[1];
		}

		// Famicom wiring (FamicomBoard::Step): DMX.sim(nY2[1], gnd, A13, A14, M2, A15, nY1, nY2)
		// Stage 1 is enabled by nY2[1] = (M2 && !A15) ? 0 : 1 (steady state).

		void RunFamicomDecode(TriState m2, TriState a15, TriState a13, TriState a14,
			TriState& nROMSEL, TriState& WRAM_nCE, TriState& PPU_nCE)
		{
			TriState nEN1 = (m2 == TriState::One && a15 == TriState::Zero) ? TriState::Zero : TriState::One;
			TriState nEN2 = TriState::Zero;
			TriState y1[4]{}, y2[4]{};

			BaseBoard::LS139::sim(nEN1, nEN2, a13, a14, m2, a15, y1, y2);

			nROMSEL = y2[3];
			WRAM_nCE = y1[0];
			PPU_nCE = y1[1];
		}
	}

	bool LS139_Test::TestNesDecode()
	{
		TriState nROMSEL, WRAM_nCE, PPU_nCE;

		// CPU write to WRAM $0000-$0FFF
		RunNesDecode(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, nROMSEL, WRAM_nCE, PPU_nCE);
		if (nROMSEL != TriState::One || WRAM_nCE != TriState::Zero || PPU_nCE != TriState::One)
		{
			return false;
		}

		// CPU access to PPU registers $2000-$2FFF (bit13=1, bit14=0)
		RunNesDecode(TriState::One, TriState::Zero, TriState::One, TriState::Zero, nROMSEL, WRAM_nCE, PPU_nCE);
		if (nROMSEL != TriState::One || WRAM_nCE != TriState::One || PPU_nCE != TriState::Zero)
		{
			return false;
		}

		// CPU access to cart ROM $8000-$FFFF
		RunNesDecode(TriState::One, TriState::One, TriState::Zero, TriState::Zero, nROMSEL, WRAM_nCE, PPU_nCE);
		if (nROMSEL != TriState::Zero || WRAM_nCE != TriState::One || PPU_nCE != TriState::One)
		{
			return false;
		}

		// CPU idle (M2 low): nothing selected
		RunNesDecode(TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, nROMSEL, WRAM_nCE, PPU_nCE);
		if (nROMSEL != TriState::One || WRAM_nCE != TriState::One || PPU_nCE != TriState::One)
		{
			return false;
		}

		return true;
	}

	bool LS139_Test::TestFamicomDecode()
	{
		TriState nROMSEL, WRAM_nCE, PPU_nCE;

		// CPU write to WRAM $0000-$0FFF
		RunFamicomDecode(TriState::One, TriState::Zero, TriState::Zero, TriState::Zero, nROMSEL, WRAM_nCE, PPU_nCE);
		if (nROMSEL != TriState::One || WRAM_nCE != TriState::Zero || PPU_nCE != TriState::One)
		{
			return false;
		}

		// CPU access to PPU registers $2000-$2FFF (bit13=1, bit14=0)
		RunFamicomDecode(TriState::One, TriState::Zero, TriState::One, TriState::Zero, nROMSEL, WRAM_nCE, PPU_nCE);
		if (nROMSEL != TriState::One || WRAM_nCE != TriState::One || PPU_nCE != TriState::Zero)
		{
			return false;
		}

		// CPU access to cart ROM $8000-$FFFF
		RunFamicomDecode(TriState::One, TriState::One, TriState::Zero, TriState::Zero, nROMSEL, WRAM_nCE, PPU_nCE);
		if (nROMSEL != TriState::Zero || WRAM_nCE != TriState::One || PPU_nCE != TriState::One)
		{
			return false;
		}

		// CPU idle (M2 low): nothing selected
		RunFamicomDecode(TriState::Zero, TriState::Zero, TriState::Zero, TriState::Zero, nROMSEL, WRAM_nCE, PPU_nCE);
		if (nROMSEL != TriState::One || WRAM_nCE != TriState::One || PPU_nCE != TriState::One)
		{
			return false;
		}

		return true;
	}
}
