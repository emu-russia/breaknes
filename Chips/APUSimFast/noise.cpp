// Noise Channel

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
{
	// The two-level NOR decoder of the original: Dec1_out[k] is One exactly when the four-bit
	// frequency register equals k, and NNF[i] is the NOR of the listed Dec1_out terms, so
	// NNF[i] is One exactly when the register value is not in that list. Each mask below holds
	// the original list of Dec1_out indices (bit k set <=> d[k] is an input of that NOR).
	static constexpr uint16_t NoiseDecoderTerms[11] =
	{
		(1u<<0)|(1u<<1)|(1u<<2)|(1u<<9)|(1u<<11)|(1u<<12)|(1u<<14)|(1u<<15),										// NNF[0]
		(1u<<4)|(1u<<8)|(1u<<14)|(1u<<15),																		// NNF[1]
		(1u<<0)|(1u<<1)|(1u<<2)|(1u<<3)|(1u<<4)|(1u<<5)|(1u<<6)|(1u<<8)|(1u<<11)|(1u<<12)|(1u<<14)|(1u<<15),		// NNF[2]
		(1u<<0)|(1u<<5)|(1u<<6)|(1u<<7)|(1u<<9)|(1u<<10)|(1u<<11)|(1u<<12)|(1u<<14)|(1u<<15),						// NNF[3]
		(1u<<0)|(1u<<1)|(1u<<2)|(1u<<6)|(1u<<7)|(1u<<8)|(1u<<9)|(1u<<10)|(1u<<11)|(1u<<12)|(1u<<13)|(1u<<14)|(1u<<15),	// NNF[4]
		(1u<<0)|(1u<<1)|(1u<<9)|(1u<<12)|(1u<<13)|(1u<<14)|(1u<<15),												// NNF[5]
		(1u<<0)|(1u<<1)|(1u<<2)|(1u<<3)|(1u<<4)|(1u<<8)|(1u<<9)|(1u<<10)|(1u<<13)|(1u<<14),						// NNF[6]
		(1u<<0)|(1u<<1)|(1u<<4)|(1u<<5)|(1u<<6)|(1u<<7)|(1u<<9)|(1u<<10)|(1u<<11)|(1u<<12)|(1u<<13)|(1u<<14)|(1u<<15),	// NNF[7]
		(1u<<0)|(1u<<1)|(1u<<2)|(1u<<3)|(1u<<6)|(1u<<7)|(1u<<10)|(1u<<11)|(1u<<12)|(1u<<13),						// NNF[8]
		(1u<<0)|(1u<<1)|(1u<<2)|(1u<<4)|(1u<<5)|(1u<<6)|(1u<<7)|(1u<<8)|(1u<<9)|(1u<<11)|(1u<<15),					// NNF[9]
		(1u<<0)|(1u<<1)|(1u<<2)|(1u<<3)|(1u<<4)|(1u<<5)|(1u<<6)|(1u<<7)|(1u<<8)|(1u<<9)|(1u<<10)|(1u<<11)|(1u<<13)|(1u<<14)|(1u<<15),	// NNF[10]
	};

	static constexpr uint16_t NoisePeriodWord(unsigned value)
	{
		uint16_t word = 0;
		for (unsigned i = 0; i < 11; ++i)
			word |= uint16_t((((NoiseDecoderTerms[i] >> value) & 1u) ^ 1u) << i);
		return word;
	}

	static constexpr std::array<uint16_t, 16> MakeNoisePeriodRom()
	{
		std::array<uint16_t, 16> rom{};
		for (unsigned value = 0; value < 16; ++value) rom[value] = NoisePeriodWord(value);
		return rom;
	}

	static constexpr std::array<uint16_t, 16> NoisePeriodRom = MakeNoisePeriodRom();

	NoiseChan::NoiseChan(APU* parent)
	{
		apu = parent;
		env_unit = new EnvelopeUnit(apu);
	}

	NoiseChan::~NoiseChan()
	{
		delete env_unit;
	}

	uint32_t NoiseChan::sim()
	{
		sim_FreqReg();
		sim_Decoder();
		sim_FreqLFSR();
		sim_RandomLFSR();

		env_unit->sim(Vol, apu->wire.W400C, apu->wire.W400F);
		// RND_Out[n] = NOR(NOT(Vol[n]), RNDOUT)
		return (RNDOUT & 1) != 0 ? 0u : PackNibble(Vol);
	}

	void NoiseChan::sim_FreqReg()
	{
		// Four RegisterBitRes cells: Res == One clears; otherwise the ACLK1-low transparent
		// path loads DB while W400E selects it, and the ACLK1-high path holds.
		if (apu->wire.RES == TriState::One) freq_reg = 0;
		else if ((apu->wire.ACLK1 & 1) == 0 && (apu->wire.W400E & 1) != 0) freq_reg = apu->DB & 15;
	}

	void NoiseChan::sim_Decoder()
	{
		nnf = NoisePeriodRom[freq_reg & 15];
	}

	void NoiseChan::sim_FreqLFSR()
	{
		const TriState ACLK1 = apu->wire.ACLK1;
		const bool aclk4 = apu->wire.nACLK2 == TriState::Zero;	// ACLK4 = NOT(nACLK2)
		const bool res = (apu->wire.RES & 1) != 0;				// RES feeds NOR gates
		const unsigned sout = flfsr_output;
		const bool nfz = sout == 0;								// NOR11 of all outputs
		const bool nfout = sout == 1;							// NOR11(NOT(sout0), sout1..10)
		const bool step_load = nfout && !res;					// NOR(NOT(NFOUT), RES)
		const bool nfload = aclk4 && step_load;					// NOR(NOT(ACLK4), NOT(step_load))
		const bool nfstep = aclk4 && !step_load;				// NOR(NOT(ACLK4), step_load)
		const unsigned s0 = sout & 1u, s2 = (sout >> 2) & 1u;
		// NSIN = NOR3(AND(s0, s2), NOR3(s0, s2, NFZ), RES)
		const unsigned nsin = (!(s0 & s2) && ((s0 | s2) != 0 || nfz) && !res) ? 1u : 0u;
		RSTEP = nfload ? TriState::One : TriState::Zero;

		// Descending scalar order: with ACLK1 transparent every stage consumes the stage above
		// it in the same call (every bit becomes NSIN); otherwise the input bank captures the
		// retained outputs shifted by one. Load has priority over step; the output bank only
		// follows the input bank while ACLK1 is One.
		if (nfload) flfsr_input = nnf;
		else if (nfstep)
			flfsr_input = ACLK1 == TriState::One ? uint16_t(nsin ? 0x7ffu : 0u) : uint16_t((sout >> 1) | (nsin << 10));
		if (ACLK1 == TriState::One) flfsr_output = flfsr_input;
	}

	void NoiseChan::sim_RandomLFSR()
	{
		const TriState ACLK1 = apu->wire.ACLK1;

		// rmod_reg: RegisterBit loaded from DB7 on the ACLK1-low transparent path.
		if ((ACLK1 & 1) == 0 && (apu->wire.W400E & 1) != 0) rmod = uint8_t(apu->DB >> 7);

		const unsigned sout = rlfsr_output;
		const bool rsoz = sout == 0;							// NOR15 of all outputs
		const unsigned s0 = sout & 1u;
		const unsigned mux_out = (rmod & 1u) != 0 ? (sout >> 6) & 1u : (sout >> 1) & 1u;
		const unsigned lock = apu->wire.LOCK & 1u;
		const unsigned nornd = apu->wire.NORND & 1u;
		// RIN = NOR3(LOCK, NOR3(RSOZ, s0, mux_out), AND(s0, mux_out))
		const unsigned rin = (lock == 0 && (rsoz || (s0 | mux_out) != 0) && (s0 & mux_out) == 0) ? 1u : 0u;
		// RNDOUT = NOR(NOR(s0, NORND), LOCK)
		RNDOUT = TriState(((s0 | nornd) & (lock ^ 1u)) & 1u);

		// RandomLFSRBit: the input RegisterBit loads the stage above (retained output, since the
		// output latches are closed while ACLK1 is low) when RSTEP selects it; the output bank
		// follows the input bank while ACLK1 is One, during which the input bank holds.
		if ((ACLK1 & 1) == 0)
		{
			if ((RSTEP & 1) != 0) rlfsr_input = uint16_t((sout >> 1) | (rin << 14));
		}
		if (ACLK1 == TriState::One) rlfsr_output = rlfsr_input;
	}

	TriState NoiseChan::get_LC()
	{
		return env_unit->get_LC();
	}

#pragma region "Debug"

	uint32_t NoiseChan::Get_FreqReg()
	{
		return freq_reg;
	}

	void NoiseChan::Set_FreqReg(uint32_t value)
	{
		freq_reg = uint8_t(value & 15);
	}

#pragma endregion "Debug"
}
