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

		bool TestDiv(bool trace);
		bool TestRegOps();
	};
}
