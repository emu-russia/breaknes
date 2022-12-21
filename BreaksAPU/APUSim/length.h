// Length Counters

#pragma once

namespace APUSim
{
	class LengthCounter
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

	public:
		LengthCounter(APU* parent);
		~LengthCounter();

		void sim();
	};
}
