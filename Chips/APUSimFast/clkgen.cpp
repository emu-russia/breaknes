// Timing Generator

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
{
	CLKGen::CLKGen(APU* parent)
	{
		apu = parent;
	}

	CLKGen::~CLKGen()
	{
	}

	void CLKGen::sim()
	{
		const auto& w = apu->wire;
		const std::array<uint8_t, 7> key = { uint8_t(w.PHI1), uint8_t(w.PHI2), uint8_t(w.RES),
			uint8_t(w.W4017), uint8_t(w.DMCINT), uint8_t(w.n_R4015),
			uint8_t((w.W4017 & 1) != 0 ? apu->DB & 0xc0 : 0) };
		if (!phase.begin(*this, key, apu->mutation_epoch))
		{
			if (w.n_R4015 == TriState::Zero) apu->SetDBBit(6, NOT(int_status.get()));
			return;
		}
		sim_ACLK();
		sim_SoftCLK_Mode();
		sim_SoftCLK_PLA();
		sim_SoftCLK_Control();
		sim_SoftCLK_LFSR();
		phase.end(*this);
	}

	void CLKGen::sim_ACLK()
	{
		TriState PHI1 = apu->wire.PHI1;
		TriState PHI2 = apu->wire.PHI2;
		TriState RES = apu->wire.RES;
		TriState prev_aclk = apu->wire.nACLK2;

		TriState temp = NOR(RES, phi2_latch.nget());
		phi1_latch.set(temp, PHI1);
		phi2_latch.set(phi1_latch.nget(), PHI2);

		TriState new_aclk = NOT(NOR(NOT(PHI1), temp));

		apu->wire.nACLK2 = new_aclk;
		apu->wire.ACLK1 = NOR(NOT(PHI1), phi2_latch.nget());

		// The software ACLK counter is triggered by the falling edge.
		// This is purely a software design for convenience, and has nothing to do with APU hardware circuitry.

		if (IsNegedge(prev_aclk, new_aclk))
		{
			apu->aclk_counter++;
		}
	}

	void CLKGen::sim_SoftCLK_Mode()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4017 = apu->wire.W4017;

		// Mode

		reg_mode.sim(ACLK1, W4017, apu->GetDBBit(7));
		n_mode = reg_mode.nget();
		md_latch.set(n_mode, ACLK1);
		mode = md_latch.nget();
	}

	void CLKGen::sim_SoftCLK_Control()
	{
		TriState nACLK2 = apu->wire.nACLK2;
		TriState ACLK1 = apu->wire.ACLK1;
		TriState PHI1 = apu->wire.PHI1;
		TriState RES = apu->wire.RES;
		TriState DMCINT = apu->wire.DMCINT;
		TriState n_R4015 = apu->wire.n_R4015;
		TriState W4017 = apu->wire.W4017;

		// Interrupt(s)

		reg_mask.sim(ACLK1, W4017, apu->GetDBBit(6));
		int_ff.set(NOR4(NOR(int_ff.get(), AND(n_mode, pla[3])), NOR(n_R4015, PHI1), reg_mask.get(), RES));
		int_status.set(int_ff.nget(), ACLK1);
		apu->SetDBBit(6, NOT(n_R4015) == TriState::One ? NOT(int_status.get()) : TriState::Z);
		apu->wire.INT = NOT(NOR(int_ff.get(), DMCINT));

		// LFSR reload

		z1.set(z_ff.get(), ACLK1);
		z2.set (n_mode, ACLK1);
		z_ff.set(NOR3(NOR(z_ff.get(), NOR(z1.get(), nACLK2)), W4017, RES));
		Z1 = z1.nget();
		Z2 = NOR(z1.get(), z2.get());

		// LFSR operation

		TriState ftemp = NOR3(Z1, pla[3], pla[4]);
		F1 = NOR(ftemp, nACLK2);
		F2 = NOR(NOT(ftemp), nACLK2);

		// LFO Outputs

		apu->wire.n_LFO1 = NOT(NOR(NOR6(pla[0], pla[1], pla[2], pla[3], pla[4], Z2), nACLK2));
		apu->wire.n_LFO2 = NOT(NOR(NOR4(pla[1], pla[3], pla[4], Z2), nACLK2));
	}

	void CLKGen::sim_SoftCLK_PLA()
	{
		// Each product term accepts one exact 15-bit output-bank word.
		static constexpr uint16_t points[6] = { 0x1061, 0x3603, 0x2cd3, 0x0a1f, 0x7185, 0 };
		for (unsigned n = 0; n < 6; ++n)
			pla[n] = lfsr_output == points[n] ? TriState::One : TriState::Zero;
		pla[3] = AND(pla[3], NOT(mode));
	}

	void CLKGen::sim_SoftCLK_LFSR()
	{
		const unsigned a = (lfsr_output >> 13) & 1u;
		const unsigned b = (lfsr_output >> 14) & 1u;
		const unsigned feedback = !(a & b) && (a | b | (pla[5] == TriState::One));
		// Ascending scalar evaluation propagates through every stage when the
		// output latches are transparent. Preserve that same-call behavior.
		if ((F2 & 1) != 0)
			lfsr_input = apu->wire.ACLK1 == TriState::One ? (feedback ? 0x7fff : 0) :
				uint16_t(((lfsr_output << 1) | feedback) & 0x7fff);
		else if ((F1 & 1) != 0) lfsr_input = 0x7fff;
		if (apu->wire.ACLK1 == TriState::One) lfsr_output = lfsr_input;
		shift_in = (lfsr_output & 0x4000) != 0 ? TriState::One : TriState::Zero;
	}

	void SoftCLK_SRBit::sim(TriState ACLK1, TriState F1, TriState F2, TriState shift_in)
	{
		in_latch.set(MUX(F2, MUX(F1, TriState::Z, TriState::One), shift_in), TriState::One);
		out_latch.set(in_latch.nget(), ACLK1);
	}

	TriState SoftCLK_SRBit::get_sout()
	{
		return out_latch.nget();
	}

	TriState SoftCLK_SRBit::get_nsout()
	{
		return out_latch.get();
	}

	TriState CLKGen::GetINTFF()
	{
		return int_ff.get();
	}
}
