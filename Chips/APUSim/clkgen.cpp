// Timing Generator

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
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
		sim_ACLK();
		sim_SoftCLK_Mode();
		sim_SoftCLK_PLA();
		sim_SoftCLK_Control();
		sim_SoftCLK_LFSR();
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
		BaseLogic::TriState s[15]{};
		BaseLogic::TriState ns[15]{};

		for (size_t n = 0; n < 15; n++)
		{
			s[n] = lfsr[n].get_sout();
			ns[n] = lfsr[n].get_nsout();
		}

		pla[0] = NOR15(ns[0], s[1], s[2], s[3], s[4], ns[5], ns[6], s[7], s[8], s[9], s[10], s[11], ns[12], s[13], s[14]);
		pla[1] = NOR15(ns[0], ns[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], ns[9], ns[10], s[11], ns[12], ns[13], s[14]);
		pla[2] = NOR15(ns[0], ns[1], s[2], s[3], ns[4], s[5], ns[6], ns[7], s[8], s[9], ns[10], ns[11], s[12], ns[13], s[14]);
		pla[3] = NOR16(ns[0], ns[1], ns[2], ns[3], ns[4], s[5], s[6], s[7], s[8], ns[9], s[10], ns[11], s[12], s[13], s[14], mode);	// ⚠️
		pla[4] = NOR15(ns[0], s[1], ns[2], s[3], s[4], s[5], s[6], ns[7], ns[8], s[9], s[10], s[11], ns[12], ns[13], ns[14]);
		pla[5] = NOR15(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14]);
	}

	void CLKGen::sim_SoftCLK_LFSR()
	{
		TriState ACLK1 = apu->wire.ACLK1;

		// Feedback

		TriState C13 = lfsr[13].get_sout();
		TriState C14 = lfsr[14].get_sout();
		shift_in = NOR(AND(C13, C14), NOR3(C13, C14, pla[5]));

		// SR15

		for (size_t n = 0; n < 15; n++)
		{
			lfsr[n].sim(ACLK1, F1, F2, shift_in);
			shift_in = lfsr[n].get_sout();
		}
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
