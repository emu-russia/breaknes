// Obtaining the analog value of the AUX A/B signals from the digital outputs of the generators.

#pragma once

namespace APUSim
{
	class DAC
	{
		APU* apu = nullptr;

		bool raw_mode = false;
		bool norm_mode = false;

		float auxa_mv[0x100]{};
		float auxa_norm[0x100]{};

		float auxb_mv[32768]{};
		float auxb_norm[32768]{};

		void gen_DACTabs();

		void sim_DACA(AudioOutSignal& aout);
		void sim_DACB(AudioOutSignal& aout);

	public:
		DAC(APU* parent);
		~DAC();

		void sim(AudioOutSignal& AUX);

		void SetRAWOutput(bool enable);

		void SetNormalizedOutput(bool enable);
	};
}
