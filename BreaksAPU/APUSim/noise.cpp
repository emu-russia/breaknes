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
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W400E = apu->wire.W400E;
		TriState RES = apu->wire.RES;

		for (size_t n = 0; n < 4; n++)
		{
			freq_reg[n].sim(ACLK1, W400E, apu->GetDBBit(n), RES);
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

		sim_Decoder1_Calc(F, nF);
	}

	void NoiseChan::sim_Decoder1_Calc(BaseLogic::TriState* F, BaseLogic::TriState* nF)
	{
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
		TriState nACLK2 = apu->wire.nACLK2;
		TriState ACLK1 = apu->wire.ACLK1;
		TriState RES = apu->wire.RES;
		TriState ACLK4 = NOT(nACLK2);
		TriState sout[11]{};

		for (size_t n = 0; n < 11; n++)
		{
			sout[n] = freq_lfsr[n].get_sout();
		}

		TriState NFZ = NOR11(sout[0], sout[1], sout[2], sout[3], sout[4], sout[5], sout[6], sout[7], sout[8], sout[9], sout[10]);
		TriState NFOUT = NOR11(NOT(sout[0]), sout[1], sout[2], sout[3], sout[4], sout[5], sout[6], sout[7], sout[8], sout[9], sout[10]);
		TriState step_load = NOR(NOT(NFOUT), RES);
		TriState NFLOAD = NOR(NOT(ACLK4), step_load);
		TriState NFSTEP = NOR(NOT(ACLK4), NOT(step_load));
		TriState NSIN = NOR3(AND(sout[0], sout[2]), NOR3(sout[0], sout[2], NFZ), RES);
		RSTEP = NFLOAD;

		TriState shift_in = NSIN;

		for (int n = 10; n >= 0; n--)
		{
			TriState nnf = NNF[n];
			freq_lfsr[n].sim(ACLK1, NFLOAD, NFSTEP, nnf, shift_in);
			shift_in = freq_lfsr[n].get_sout();
		}
	}

	void NoiseChan::sim_RandomLFSR()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W400E = apu->wire.W400E;
		TriState NORND = apu->wire.NORND;
		TriState LOCK = apu->wire.LOCK;
		TriState sout[15]{};

		rmod_reg.sim(ACLK1, W400E, apu->GetDBBit(7));

		for (size_t n = 0; n < 15; n++)
		{
			sout[n] = rnd_lfsr[n].get_sout();
		}

		TriState RSOZ = NOR15 (sout[0], sout[1], sout[2], sout[3], sout[4], sout[5], sout[6], sout[7], sout[8], sout[9], sout[10], sout[11], sout[12], sout[13], sout[14]);
		TriState mux_out = MUX(rmod_reg.get(), sout[1], sout[6]);
		TriState RIN = NOR3 (LOCK, NOR3(RSOZ, sout[0], mux_out), AND(sout[0], mux_out));
		RNDOUT = NOR(NOR(sout[0], NORND), LOCK);

		TriState shift_in = RIN;

		for (int n = 14; n >= 0; n--)
		{
			rnd_lfsr[n].sim(ACLK1, RSTEP, shift_in);
			shift_in = rnd_lfsr[n].get_sout();
		}
	}

	void FreqLFSRBit::sim(TriState ACLK1, TriState load, TriState step, TriState val, TriState shift_in)
	{
		TriState d = MUX(load, MUX(step, TriState::Z, shift_in), val);
		in_latch.set(d, TriState::One);
		out_latch.set(in_latch.nget(), ACLK1);
	}

	TriState FreqLFSRBit::get_sout()
	{
		return out_latch.nget();
	}

	void RandomLFSRBit::sim(TriState ACLK1, TriState load, TriState shift_in)
	{
		in_reg.sim(ACLK1, load, shift_in);
		out_latch.set(in_reg.nget(), ACLK1);
	}

	TriState RandomLFSRBit::get_sout()
	{
		return out_latch.nget();
	}

	TriState NoiseChan::get_LC()
	{
		return env_unit->get_LC();
	}

#pragma region "Debug"

	uint32_t NoiseChan::Get_FreqReg()
	{
		TriState val[4]{};

		for (size_t n = 0; n < 4; n++)
		{
			val[n] = freq_reg[n].get();
		}
		return PackNibble(val);
	}

	void NoiseChan::Set_FreqReg(uint32_t value)
	{
		TriState val[4]{};
		UnpackNibble(value, val);
		for (size_t n = 0; n < 4; n++)
		{
			freq_reg[n].set(val[n]);
		}
	}

#pragma endregion "Debug"
}
