// Noise Channel

#pragma once

namespace APUSim
{
	class FreqLFSRBit
	{
		BaseLogic::DLatch in_latch{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState load, BaseLogic::TriState step, BaseLogic::TriState val, BaseLogic::TriState shift_in);
		BaseLogic::TriState get_sout();
	};

	class RandomLFSRBit
	{
		RegisterBit in_reg{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState load, BaseLogic::TriState shift_in);
		BaseLogic::TriState get_sout();
	};

	class NoiseChan
	{
		friend APUSimUnitTest::UnitTest;
		friend APU;

		APU* apu = nullptr;

		BaseLogic::TriState NNF[11]{};
		BaseLogic::TriState RSTEP = BaseLogic::TriState::X;
		BaseLogic::TriState RNDOUT = BaseLogic::TriState::X;
		BaseLogic::TriState Vol[4]{};
		BaseLogic::TriState Dec1_out[16]{};

		RegisterBitRes freq_reg[4]{};
		FreqLFSRBit freq_lfsr[11]{};
		RegisterBit rmod_reg{};
		RandomLFSRBit rnd_lfsr[15]{};
		EnvelopeUnit *env_unit = nullptr;

		void sim_FreqReg();
		void sim_Decoder1();
		void sim_Decoder1_Calc(BaseLogic::TriState *F, BaseLogic::TriState *nF);
		void sim_Decoder2();
		void sim_FreqLFSR();
		void sim_RandomLFSR();

	public:
		NoiseChan(APU* parent);
		~NoiseChan();

		void sim();
		BaseLogic::TriState get_LC();

		uint32_t Get_FreqReg();
		void Set_FreqReg(uint32_t value);
	};
}
