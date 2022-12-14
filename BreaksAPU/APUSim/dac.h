// Obtaining the analog value of the AUX A/B signals from the digital outputs of the generators.

#pragma once

namespace APUSim
{
	class DAC
	{
		APU* apu = nullptr;

	public:
		DAC(APU* parent);
		~DAC();

		void sim(float* AUX_A, float* AUX_B);
	};
}
