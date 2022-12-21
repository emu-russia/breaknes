// Differential Pulse-code Modulation (DPCM)

#pragma once

namespace APUSim
{
	class DpcmChan
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

	public:
		DpcmChan(APU* parent);
		~DpcmChan();

		void sim();
	};
}
