// Noise Channel

#pragma once

namespace FastAPU
{
	class NoiseChan
	{
		friend FastAPU;

		FastAPU* apu = nullptr;

		uint16_t NNF{};
		int RSTEP{};
		int RNDOUT{};
		uint8_t Vol{};
		BaseLogic::TriState Dec1_out[16]{};

		uint8_t freq_reg{};
		uint16_t freq_lfsr{};
		int rmod_reg{};
		uint16_t rnd_lfsr{};
		EnvelopeUnit* env_unit = nullptr;

		void sim_FreqReg();
		void sim_Decoder1();
		void sim_Decoder1_Calc(BaseLogic::TriState* F, BaseLogic::TriState* nF);
		void sim_Decoder2();
		void sim_FreqLFSR();
		void sim_RandomLFSR();

	public:
		NoiseChan(FastAPU* parent);
		~NoiseChan();

		void sim();
		int get_LC();
	};
}
