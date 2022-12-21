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

		TriState temp = NOR(RES, phi2_latch.nget());
		phi1_latch.set(temp, PHI1);
		phi2_latch.set(phi1_latch.nget(), PHI2);

		apu->wire.ACLK = NOT(NOR(NOT(PHI1), temp));
		apu->wire.n_ACLK = NOR(NOT(PHI1), phi2_latch.nget());
	}

	void CLKGen::sim_SoftCLK_Mode()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W4017 = apu->wire.W4017;

		// Mode

		reg_mode.sim(n_ACLK, W4017, apu->GetDBBit(7));
		n_mode = reg_mode.nget();
		md_latch.set(n_mode, n_ACLK);
		mode = md_latch.nget();
	}

	void CLKGen::sim_SoftCLK_Control()
	{
		TriState ACLK = apu->wire.ACLK;
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState PHI1 = apu->wire.PHI1;
		TriState RES = apu->wire.RES;
		TriState DMCINT = apu->wire.DMCINT;
		TriState n_R4015 = apu->wire.n_R4015;
		TriState W4017 = apu->wire.W4017;

		// Interrupt(s)

		reg_mask.sim(n_ACLK, W4017, apu->GetDBBit(6));
		int_ff.set(NOR4(NOR(int_ff.get(), AND(n_mode, pla[3])), NOR(n_R4015, PHI1), reg_mask.get(), RES));
		int_status.set(int_ff.nget(), ACLK);
		apu->SetDBBit(6, NOT(n_R4015) == TriState::One ? NOT(int_status.get()) : TriState::Z);
		apu->wire.INT = NOT(NOR(int_ff.get(), DMCINT));

		// LFSR reload

		z1.set(z_ff.get(), n_ACLK);
		z2.set (n_mode, n_ACLK);
		z_ff.set(NOR3(NOR(z_ff.get(), NOR(z1.get(), ACLK)), W4017, RES));
		Z1 = z1.nget();
		Z2 = NOR(z1.get(), z2.get());

		// LFSR operation

		TriState ftemp = NOR3(Z1, pla[3], pla[4]);
		F1 = NOR(ftemp, ACLK);
		F2 = NOR(NOT(ftemp), ACLK);

		// LFO Outputs

		apu->wire.n_LFO1 = NOT(NOR(NOR6(pla[0], pla[1], pla[2], pla[3], pla[4], Z2), ACLK));
		apu->wire.n_LFO2 = NOT(NOR(NOR4(pla[1], pla[3], pla[4], Z2), ACLK));
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

		pla[0] = NOR15(s[0], ns[1], ns[2], ns[3], ns[4], s[5], s[6], ns[7], ns[8], ns[9], ns[10], ns[11], s[12], ns[13], ns[14]);
		pla[1] = NOR15(s[0], s[1], ns[2], ns[3], ns[4], ns[5], ns[6], ns[7], ns[8], s[9], s[10], ns[11], s[12], s[13], ns[14]);
		pla[2] = NOR15(s[0], s[1], ns[2], ns[3], s[4], ns[5], s[6], s[7], ns[8], ns[9], s[10], s[11], ns[12], s[13], ns[14]);
		pla[3] = NOR16(s[0], s[1], s[2], s[3], s[4], ns[5], ns[6], ns[7], ns[8], s[9], ns[10], s[11], ns[12], ns[13], ns[14], mode);	// !!
		pla[4] = NOR15(s[0], ns[1], s[2], ns[3], ns[4], ns[5], ns[6], s[7], s[8], ns[9], ns[10], ns[11], s[12], s[13], s[14]);
		pla[5] = NOR15(ns[0], ns[1], ns[2], ns[3], ns[4], ns[5], ns[6], ns[7], ns[8], ns[9], ns[10], ns[11], ns[12], ns[13], ns[14]);
	}

	void CLKGen::sim_SoftCLK_LFSR()
	{
		TriState n_ACLK = apu->wire.n_ACLK;

		// Feedback

		TriState C13 = lfsr[13].get_nsout();
		TriState C14 = lfsr[14].get_nsout();
		n_sin = NOR(AND(C13, C14), NOR3(C13, C14, pla[5]));

		// SR15

		TriState nsin = n_sin;

		for (size_t n = 0; n < 15; n++)
		{
			lfsr[n].sim(n_ACLK, F1, F2, nsin);
			nsin = lfsr[n].get_nsout();
		}
	}

	void SoftCLK_SRBit::sim(TriState n_ACLK, TriState F1, TriState F2, TriState n_sin)
	{
		in_latch.set(MUX(F2, MUX(F1, TriState::Z, TriState::One), n_sin), TriState::One);
		out_latch.set(in_latch.nget(), n_ACLK);
	}

	TriState SoftCLK_SRBit::get_sout()
	{
		return out_latch.get();
	}

	TriState SoftCLK_SRBit::get_nsout()
	{
		return out_latch.nget();
	}
}
