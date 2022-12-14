// Differential Pulse-code Modulation (DPCM)

#pragma once

namespace APUSim
{
	class DpcmChan
	{
		APU* apu = nullptr;

	public:
		DpcmChan(APU* parent);
		~DpcmChan();

		void sim();
	};
}
