// Envelope Unit
// Shared between the square channels and the noise generator.

#pragma once

namespace APUSim
{
	class EnvelopeUnit
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

	public:
		EnvelopeUnit(APU* parent);
		~EnvelopeUnit();

		void sim();
	};
}
