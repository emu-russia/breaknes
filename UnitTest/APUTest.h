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
		/// Run a few full APU cycles (reset-free, data bus = 0). Used as a smoke-run helper.
		/// </summary>
		void RunCycles(size_t num_cycles);

		bool TestAPUDebugInfo();
		bool TestCommonBitCells();
		bool TestSoftCLK_SRBit();
		bool TestDIV_SRBit();
		bool TestCLKGen();
		bool TestDMA();
		bool TestDPCMBits();
		bool TestDpcmChan();
		bool TestLengthCounterDebug();
		bool TestEnvelopeUnit();
		bool TestNoiseBits();
		bool TestNoiseChan();
		bool TestSquareBits();
		bool TestSquareChan();
		bool TestTriangleChan();
		bool TestRegsDecoder();
		bool TestBIDIR();
		bool TestPads();
		bool TestDAC();

		/// <summary>
		/// Execute some million cycles and check that their execution time is faster or equal to the real chip.
		/// The chip in this test is in "pumpkin" mode: it lives, but it does nothing useful.
		/// </summary>
		/// <param name="desired_clk">Desired cycle rate per second (Hz)</param>
		/// <returns></returns>
		bool MegaCyclesTest(size_t desired_clk);
	};
}
