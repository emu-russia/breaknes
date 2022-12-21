// Triangle Channel

#pragma once

namespace APUSim
{
	class TriangleChan
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

	public:
		TriangleChan(APU* parent);
		~TriangleChan();

		void sim();
	};
}
