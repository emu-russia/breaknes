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

		const float IntRes = 39000.f;		// Internal single MOSFET resistance
		const float IntUnloaded = 999999.f;	// Non-loaded DAC internal resistance
		const float ExtRes = 100.f;		// On-board pull-down resistor to GND
		const float Vdd = 5.0f;

		float AUX_A_Resistance(size_t sqa, size_t sqb);
		float AUX_B_Resistance(size_t tri, size_t rnd, size_t dmc);

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
