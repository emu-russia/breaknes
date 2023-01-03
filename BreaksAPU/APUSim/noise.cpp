// Noise Channel

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	NoiseChan::NoiseChan(APU* parent)
	{
		apu = parent;
		env_unit = new EnvelopeUnit(apu);
	}

	NoiseChan::~NoiseChan()
	{
		delete env_unit;
	}

	void NoiseChan::sim()
	{
		sim_FreqReg();
		sim_Decoder1();
		sim_Decoder2();
		sim_FreqLFSR();
		sim_RandomLFSR();
		
		env_unit->sim(Vol, apu->wire.W400C, apu->wire.W400F);
		for (size_t n = 0; n < 4; n++)
		{
			apu->RND_Out[n] = NOR(NOT(Vol[n]), RNDOUT);
		}
	}

	void NoiseChan::sim_FreqReg()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W400E = apu->wire.W400E;
		TriState RES = apu->wire.RES;

		for (size_t n = 0; n < 4; n++)
		{
			freq_reg[n].sim(n_ACLK, W400E, apu->GetDBBit(n), RES);
		}
	}

	void NoiseChan::sim_Decoder1()
	{
		TriState F[4]{};
		TriState nF[4]{};

		for (size_t n = 0; n < 4; n++)
		{
			F[n] = freq_reg[n].get();
			nF[n] = NOT(freq_reg[n].get());
		}

		Dec1_out[0] = NOR4(F[0], F[1], F[2], F[3]);
		Dec1_out[1] = NOR4(nF[0], F[1], F[2], F[3]);
		Dec1_out[2] = NOR4(F[0], nF[1], F[2], F[3]);
		Dec1_out[3] = NOR4(nF[0], nF[1], F[2], F[3]);
		Dec1_out[4] = NOR4(F[0], F[1], nF[2], F[3]);
		Dec1_out[5] = NOR4(nF[0], F[1], nF[2], F[3]);
		Dec1_out[6] = NOR4(F[0], nF[1], nF[2], F[3]);
		Dec1_out[7] = NOR4(nF[0], nF[1], nF[2], F[3]);

		Dec1_out[8] = NOR4(F[0], F[1], F[2], nF[3]);
		Dec1_out[9] = NOR4(nF[0], F[1], F[2], nF[3]);
		Dec1_out[10] = NOR4(F[0], nF[1], F[2], nF[3]);
		Dec1_out[11] = NOR4(nF[0], nF[1], F[2], nF[3]);
		Dec1_out[12] = NOR4(F[0], F[1], nF[2], nF[3]);
		Dec1_out[13] = NOR4(nF[0], F[1], nF[2], nF[3]);
		Dec1_out[14] = NOR4(F[0], nF[1], nF[2], nF[3]);
		Dec1_out[15] = NOR4(nF[0], nF[1], nF[2], nF[3]);
	}

	void NoiseChan::sim_Decoder2()
	{
		TriState* d = Dec1_out;

		NNF[0] = NOR8(d[0], d[1], d[2], d[9], d[11], d[12], d[14], d[15]);
		NNF[1] = NOR4(d[4], d[8], d[14], d[15]);
		NNF[2] = NOR12(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[8], d[11], d[12], d[14], d[15]);
		NNF[3] = NOR10(d[0], d[5], d[6], d[7], d[9], d[10], d[11], d[12], d[14], d[15]);
		NNF[4] = NOR13(d[0], d[1], d[2], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
		NNF[5] = NOR7(d[0], d[1], d[9], d[12], d[13], d[14], d[15]);
		NNF[6] = NOR10(d[0], d[1], d[2], d[3], d[4], d[8], d[9], d[10], d[13], d[14]);
		NNF[7] = NOR13(d[0], d[1], d[4], d[5], d[6], d[7], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
		NNF[8] = NOR10(d[0], d[1], d[2], d[3], d[6], d[7], d[10], d[11], d[12], d[13]);
		NNF[9] = NOR11(d[0], d[1], d[2], d[4], d[5], d[6], d[7], d[8], d[9], d[11], d[15]);
		NNF[10] = NOR15(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[13], d[14], d[15]);
	}

	void NoiseChan::sim_FreqLFSR()
	{
		TriState ACLK = apu->wire.ACLK;
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;
		TriState n_ACLK4 = NOT(ACLK);
		TriState sout[11]{};

		for (size_t n = 0; n < 11; n++)
		{
			sout[n] = freq_lfsr[n].get_sout();
		}

		TriState NFZ = NOR11(sout[0], sout[1], sout[2], sout[3], sout[4], sout[5], sout[6], sout[7], sout[8], sout[9], sout[10]);
		TriState NFOUT = NOR11(NOT(sout[0]), sout[1], sout[2], sout[3], sout[4], sout[5], sout[6], sout[7], sout[8], sout[9], sout[10]);
		TriState step_load = NOR(NOT(NFOUT), RES);
		TriState NFLOAD = NOR(NOT(n_ACLK4), step_load);
		TriState NFSTEP = NOR(NOT(n_ACLK4), NOT(step_load));
		TriState NSIN = NOR3(AND(sout[0], sout[2]), NOR3(sout[0], sout[2], NFZ), RES);
		RSTEP = NFLOAD;

		TriState sin = NSIN;

		for (int n = 10; n >= 0; n--)
		{
			freq_lfsr[n].sim(n_ACLK, NFLOAD, NFSTEP, NNF[n], sin);
			sin = freq_lfsr[n].get_sout();
		}
	}

	void NoiseChan::sim_RandomLFSR()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W400E = apu->wire.W400E;
		TriState NORND = apu->wire.NORND;
		TriState LOCK = apu->wire.LOCK;
		TriState sout[15]{};

		rmod_reg.sim(n_ACLK, W400E, apu->GetDBBit(7));

		for (size_t n = 0; n < 15; n++)
		{
			sout[n] = rnd_lfsr[n].get_sout();
		}

		TriState RSOZ = NOR15 (sout[0], sout[1], sout[2], sout[3], sout[4], sout[5], sout[6], sout[7], sout[8], sout[9], sout[10], sout[11], sout[12], sout[13], sout[14]);
		TriState mux_out = MUX(rmod_reg.get(), sout[1], sout[6]);
		TriState RIN = NOR3 (LOCK, NOR3(RSOZ, sout[0], mux_out), AND(sout[0], mux_out));
		RNDOUT = NOR(NOR(sout[0], NORND), LOCK);

		TriState sin = RIN;

		for (int n = 14; n >= 0; n--)
		{
			rnd_lfsr[n].sim(n_ACLK, RSTEP, sin);
			sin = rnd_lfsr[n].get_sout();
		}
	}

	void FreqLFSRBit::sim(TriState n_ACLK, TriState load, TriState step, TriState val, TriState sin)
	{
		TriState d = MUX(load, MUX(step, TriState::Z, sin), val);
		in_latch.set(d, TriState::One);
		out_latch.set(in_latch.nget(), n_ACLK);
	}

	TriState FreqLFSRBit::get_sout()
	{
		return out_latch.nget();
	}

	void RandomLFSRBit::sim(TriState n_ACLK, TriState load, TriState sin)
	{
		in_reg.sim(n_ACLK, load, sin);
		out_latch.set(in_reg.nget(), n_ACLK);
	}

	TriState RandomLFSRBit::get_sout()
	{
		return out_latch.nget();
	}

	TriState NoiseChan::get_LC()
	{
		return env_unit->get_LC();
	}
}
