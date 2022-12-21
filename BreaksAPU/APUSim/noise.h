// Noise Channel

#pragma once

namespace APUSim
{
	class NoiseChan
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

	public:
		NoiseChan(APU* parent);
		~NoiseChan();

		void sim();
	};
}
