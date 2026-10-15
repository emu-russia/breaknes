// Noise Channel

#pragma once

namespace APUSimFast
{
	class NoiseChan
	{
		friend APUSimUnitTest::UnitTest;
		friend APU;

		APU* apu = nullptr;

		// Packed representation of the original cells. Each LFSR keeps the transparent input
		// bank and the ACLK1-clocked output bank; the output bank stores the logical sout values
		// (inverted output latches), like CLKGen and DpcmChan. Startup values are the original
		// latch initialization: in=0, out=0 -> sout=1.
		uint8_t freq_reg = 0;								// $400E[3:0], four RegisterBitRes cells
		uint8_t rmod = 0;									// $400E[7], one RegisterBit cell
		uint16_t nnf = 0;									// decoded period word, combinational
		uint16_t flfsr_input = 0, flfsr_output = 0x7ff;		// 11-bit frequency LFSR banks
		uint16_t rlfsr_input = 0, rlfsr_output = 0x7fff;	// 15-bit random LFSR banks
		BaseLogic::TriState RSTEP = BaseLogic::TriState::X;
		BaseLogic::TriState RNDOUT = BaseLogic::TriState::X;
		BaseLogic::TriState Vol[4]{};

		EnvelopeUnit *env_unit = nullptr;

		void sim_FreqReg();
		void sim_Decoder();
		void sim_FreqLFSR();
		void sim_RandomLFSR();

	public:
		NoiseChan(APU* parent);
		~NoiseChan();

		// Returns the 4-bit channel output (the original RND_Out bits).
		uint32_t sim();
		BaseLogic::TriState get_LC();

		uint32_t Get_FreqReg();
		void Set_FreqReg(uint32_t value);
	};
}
