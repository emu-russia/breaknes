// Noise Channel

#pragma once

namespace FastAPU
{
	class NoiseChan
	{
		friend FastAPU;

		FastAPU* apu = nullptr;

		BaseLogic::TriState NNF[11]{};
		BaseLogic::TriState RSTEP = BaseLogic::TriState::X;
		BaseLogic::TriState RNDOUT = BaseLogic::TriState::X;
		BaseLogic::TriState Vol[4]{};
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
		BaseLogic::TriState get_LC();
	};
}
