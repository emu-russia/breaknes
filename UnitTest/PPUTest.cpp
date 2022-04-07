// Various PPU simulator tests. Doing it without stress, according to the mood.

#include "pch.h"

using namespace BaseLogic;
using namespace PPUSim;

namespace PPUSimUnitTest
{
	UnitTest::UnitTest(Revision rev)
	{
		ppu = new PPU(rev);
	}

	UnitTest::~UnitTest()
	{
		delete ppu;
	}

	void UnitTest::pclk()
	{
		ppu->wire.PCLK = TriState::One;
		ppu->wire.n_PCLK = TriState::Zero;
	}

	void UnitTest::n_pclk()
	{
		ppu->wire.PCLK = TriState::Zero;
		ppu->wire.n_PCLK = TriState::One;
	}

	void UnitTest::hv_count()
	{
		ppu->h->sim(TriState::One, TriState::Zero);
		ppu->v->sim(TriState::One, TriState::Zero);
	}

	/// <summary>
	/// Execute several PCLK cycles with RES = 1 and check that the counter value is 0.
	/// Make several cycles of counting beforehand.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::HVCounterTestRES()
	{
		size_t numPclkCycles = 3;

		// Count

		ppu->wire.RES = TriState::Zero;

		for (size_t n = 0; n < numPclkCycles; n++)
		{
			n_pclk();
			hv_count();

			pclk();
			hv_count();
		}

		if (! (ppu->h->get() == numPclkCycles && ppu->v->get() == numPclkCycles) )
		{
			return false;
		}

		// Count while Reset

		ppu->wire.RES = TriState::One;

		for (size_t n = 0; n < numPclkCycles; n++)
		{
			n_pclk();
			hv_count();

			pclk();
			hv_count();
		}

		return ppu->h->get() == 0 && ppu->v->get() == 0;
	}

	/// <summary>
	/// Run several PCLK cycles and make sure that the counters count.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::HVCounterTest()
	{
		size_t numPclkCycles = 7;

		// Count

		ppu->wire.RES = TriState::Zero;

		for (size_t n = 0; n < numPclkCycles; n++)
		{
			n_pclk();
			hv_count();

			pclk();
			hv_count();
		}

		return ppu->h->get() == numPclkCycles && ppu->v->get() == numPclkCycles;
	}
}
