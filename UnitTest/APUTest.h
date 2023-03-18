#pragma once

namespace APUSimUnitTest
{
	class UnitTest
	{
		M6502Core::M6502* core = nullptr;
		APUSim::APU* apu = nullptr;

		bool VerifyRegOpByAddress(uint16_t addr, bool read);

	public:
		UnitTest(APUSim::Revision rev);
		~UnitTest();

		bool TestCounters();
		bool TestDiv(bool trace);
		bool TestAclk();
		bool TestLFO(bool mode);
		bool TestRegOps();
		bool TestLengthDecoder();
		bool TestLengthCounter();
		bool TestOAM_DMA();

		/// <summary>
		/// Execute some million cycles and check that their execution time is faster or equal to the real chip.
		/// The chip in this test is in "pumpkin" mode: it lives, but it does nothing useful.
		/// </summary>
		/// <param name="desired_clk">Desired cycle rate per second (Hz)</param>
		/// <returns></returns>
		bool MegaCyclesTest(size_t desired_clk);
	};
}
