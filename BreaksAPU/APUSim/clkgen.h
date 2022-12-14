// Timing Generator

#pragma once

namespace APUSim
{
	class CLKGen
	{
		APU* apu = nullptr;

	public:
		CLKGen(APU* parent);
		~CLKGen();

		void sim();
	};
}
