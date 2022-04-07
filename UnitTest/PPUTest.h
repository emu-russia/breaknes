#pragma once

namespace PPUSimUnitTest
{
	/// <summary>
	/// Call one public test at a time, after creating the class instance (only this way will guarantee clean conditions between tests)
	/// </summary>
	class UnitTest
	{
		PPUSim::PPU* ppu = nullptr;

		void pclk();
		void n_pclk();
		void hv_count();

	public:
		UnitTest(PPUSim::Revision rev);
		~UnitTest();

		bool HVCounterTestRES();
		bool HVCounterTest();
	};
}
