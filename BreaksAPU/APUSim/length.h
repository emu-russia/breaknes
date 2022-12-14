// Length Counters

#pragma once

namespace APUSim
{
	class LengthCounter
	{
		APU* apu = nullptr;

	public:
		LengthCounter(APU* parent);
		~LengthCounter();

		void sim();
	};
}
