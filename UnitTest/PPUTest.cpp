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

	/// <summary>
	/// Run one PCLK and check that nothing is broken.
	/// The safest combinations of inputs are used.
	/// </summary>
	/// <returns></returns>
	bool UnitTest::RunSinglePCLK()
	{
		TriState inputs[(size_t)InputPad::Max]{};
		TriState outputs[(size_t)OutputPad::Max]{};
		uint8_t ext = 0;
		uint8_t data_bus = 0;
		uint8_t ad_bus = 0;
		uint8_t addrHi_bus = 0;
		VideoOutSignal vout{};

		inputs[(size_t)InputPad::RnW] = TriState::Zero;
		inputs[(size_t)InputPad::RS0] = TriState::Zero;
		inputs[(size_t)InputPad::RS1] = TriState::Zero;
		inputs[(size_t)InputPad::RS2] = TriState::Zero;
		inputs[(size_t)InputPad::n_DBE] = TriState::One;

		inputs[(size_t)InputPad::n_RES] = TriState::One;

		// Iterate over CLK until the internal PCLK counter changes value.

		auto prevPclk = ppu->GetPCLKCounter();

		while (prevPclk == ppu->GetPCLKCounter())
		{
			inputs[(size_t)InputPad::CLK] = TriState::Zero;

			ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);

			inputs[(size_t)InputPad::CLK] = TriState::One;

			ppu->sim(inputs, outputs, &ext, &data_bus, &ad_bus, &addrHi_bus, vout);
		}

		return true;
	}

	/// <summary>
	/// Run the specified number of full scans.
	/// </summary>
	/// <param name="n"></param>
	/// <returns></returns>
	bool UnitTest::RunLines(size_t n)
	{
		size_t pclkInLine = 341;

		while (n--)
		{
			for (size_t pclk = 0; pclk < pclkInLine; pclk++)
			{
				RunSinglePCLK();
			}
		}

		return true;
	}
}
