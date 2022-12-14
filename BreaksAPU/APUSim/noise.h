// Noise Channel

#pragma once

namespace APUSim
{
	class NoiseChan
	{
		APU* apu = nullptr;

	public:
		NoiseChan(APU* parent);
		~NoiseChan();

		void sim();
	};
}
