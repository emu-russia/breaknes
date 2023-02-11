// Differential Pulse-code Modulation (DPCM)

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	DpcmChan::DpcmChan(APU* parent)
	{
		apu = parent;
	}

	DpcmChan::~DpcmChan()
	{
	}

	void DpcmChan::sim()
	{
		n_ACLK2 = NOT(apu->wire.ACLK);

		sim_SampleCounterReg();
		
		sim_ControlReg();
		sim_Decoder1();
		sim_Decoder2();

		sim_FreqLFSR();

		sim_IntControl();
		sim_EnableControl();
		sim_DMAControl();
		
		sim_SampleCounterControl();
		sim_SampleCounter();
		sim_SampleBitCounter();

		sim_SampleBufferControl();
		sim_SampleBuffer();

		sim_AddressReg();
		sim_AddressCounter();
		sim_Output();
	}

#pragma region "DPCM Control"

	void DpcmChan::sim_ControlReg()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W4010 = apu->wire.W4010;

		for (size_t n = 0; n < 4; n++)
		{
			freq_reg[n].sim(n_ACLK, W4010, apu->GetDBBit(n));
			Fx[n] = freq_reg[n].get();
		}

		loop_reg.sim(n_ACLK, W4010, apu->GetDBBit(6));
		LOOPMode = loop_reg.get();

		irq_reg.sim(n_ACLK, W4010, apu->GetDBBit(7));
		n_IRQEN = irq_reg.nget();
	}

	void DpcmChan::sim_IntControl()
	{
		TriState W4015 = apu->wire.W4015;
		TriState n_R4015 = apu->wire.n_R4015;
		TriState RES = apu->wire.RES;
		TriState AssertInt = ED1;

		int_ff.set (NOR4(NOR(int_ff.get(), AssertInt), W4015, LOOPMode, RES));
		apu->SetDBBit(7, NOT(n_R4015) == TriState::One ? NOT(int_ff.nget()) : TriState::Z);
		apu->wire.DMCINT = NOR(int_ff.nget(), LOOPMode);
	}

	void DpcmChan::sim_EnableControl()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W4015 = apu->wire.W4015;
		TriState n_R4015 = apu->wire.n_R4015;
		TriState RES = apu->wire.RES;
		TriState PCMDone = get_DMC1();

		sout_latch.set(SOUT, n_ACLK);
		DMC2 = sout_latch.get();
		ED1 = NOR3(n_IRQEN, sout_latch.nget(), NOT(PCMDone));
		ena_ff.sim(n_ACLK, W4015, apu->GetDBBit(4), ED1, RES);
		ED2 = ena_ff.get();
		apu->SetDBBit(4, NOT(n_R4015) == TriState::One ? NOT(ena_ff.nget()) : TriState::Z);
	}

	void DpcmChan::sim_DMAControl()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState PHI1 = apu->wire.PHI1;
		TriState RnW = apu->wire.RnW;
		TriState RES = apu->wire.RES;
		TriState nDMCEnableDelay = get_CTRL2();
		TriState nDMAStop = get_CTRL1();

		run_latch1.set(start_ff.get(), n_ACLK2);
		run_latch2.set(run_latch1.nget(), n_ACLK);
		TriState start_set = NOT(NOR3(AND(NOT(PHI1), RnW), nDMCEnableDelay, NOT(nDMAStop)));
		start_ff.set(NOR4(NOR(start_ff.get(), start_set), nDMCEnableDelay, RES, NOT(nDMAStop)));
		rdy_ff.set(NOR(NOR(rdy_ff.get(), AND(run_latch1.get(), n_ACLK)), n_ACLK2));

		//apu->wire.RUNDMC = run_latch2.nget();
		//apu->wire.n_DMC_AB = rdy_ff.nget();
		//apu->wire.DMCRDY = NOR(start_ff.get(), rdy_ff.get());

		// DEBUG: Disable DPCM for now..
		apu->wire.RUNDMC = TriState::Zero;
		apu->wire.n_DMC_AB = TriState::One;
		apu->wire.DMCRDY = TriState::One;
	}

	void DpcmChan::sim_SampleCounterControl()
	{
		TriState ACLK = apu->wire.ACLK;
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState PCMDone = get_DMC1();
		TriState DMCFinish = DMC2;
		TriState DMCEnable = ED2;

		fin_latch.set(DMCFinish, n_ACLK);
		en_latch1.set(DMCEnable, n_ACLK);
		en_latch2.set(en_latch1.nget(), n_ACLK2);
		en_latch3.set(en_latch2.nget(), n_ACLK);
		CTRL2 = en_latch3.nget();
		TriState DMC3 = NOR3(ACLK, en_latch1.nget(), en_latch3.get());

		NSTEP = NOT(NOT(DFLOAD));
		DSLOAD = NOT(NOR(AND(fin_latch.get(), PCMDone), DMC3));
		DSSTEP = NOR3(NOT(PCMDone), DMC3, DMCFinish);
	}

	void DpcmChan::sim_SampleBufferControl()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;
		TriState PHI1 = apu->wire.PHI1;
		TriState n_DMC_AB = apu->wire.n_DMC_AB;
		TriState LOCK = apu->wire.LOCK;
		TriState n_DFLOAD = NOT(DFLOAD);

		step_ff.set(NOR3(NOR(step_ff.get(), BLOAD), NOR3(NOT(stop_latch.nget()), n_DFLOAD, n_NOUT), RES));
		stop_ff.set(NOR3(NOR(stop_ff.get(), PCM), BLOAD, RES));
		CTRL1 = stop_ff.nget();
		pcm_ff.set(NOR3(NOR(pcm_ff.get(), PCM), DMC1, RES));

		dout_latch.set(DOUT, n_ACLK);
		dstep_latch.set(step_ff.nget(), n_ACLK);
		stop_latch.set(stop_ff.get(), n_ACLK);
		pcm_latch.set(pcm_ff.nget(), n_ACLK);

		PCM = NOR(PHI1, n_DMC_AB);
		DMC1 = NOR(pcm_latch.get(), NOT(n_ACLK2));
		DSTEP = NOR4(dout_latch.get(), dstep_latch.get(), n_DFLOAD, LOCK);
		BLOAD = NOR3(stop_latch.nget(), n_DFLOAD, n_NOUT);
		BSTEP = NOR(n_DFLOAD, NOT(n_NOUT));
	}

	TriState DpcmChan::get_CTRL1()
	{
		return stop_ff.nget();
	}

	TriState DpcmChan::get_CTRL2()
	{
		return en_latch3.nget();
	}

	TriState DpcmChan::get_DMC1()
	{
		return NOR(pcm_latch.get(), NOT(n_ACLK2));
	}

#pragma endregion "DPCM Control"

#pragma region "DPCM Sampling"

	void DpcmChan::sim_Decoder1()
	{
		TriState F[4]{};
		TriState nF[4]{};

		for (size_t n = 0; n < 4; n++)
		{
			F[n] = Fx[n];
			nF[n] = NOT(Fx[n]);
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

	void DpcmChan::sim_Decoder2()
	{
		TriState* d = Dec1_out;

		FR[0] = NOR5(d[1], d[4], d[9], d[14], d[15]);
		FR[1] = NOR8(d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13]);
		FR[2] = NOR9(d[0], d[1], d[2], d[3], d[7], d[8], d[10], d[11], d[13]);
		FR[3] = NOR5(d[0], d[2], d[7], d[10], d[15]);
		FR[4] = NOR7(d[1], d[2], d[4], d[8], d[12], d[13], d[14]);
		FR[5] = NOR10(d[1], d[2], d[3], d[7], d[8], d[9], d[12], d[13], d[14], d[15]);
		FR[6] = NOR6(d[1], d[5], d[8], d[12], d[13], d[14]);
		FR[7] = NOR6(d[0], d[2], d[5], d[6], d[8], d[15]);
		FR[8] = NOR9(d[1], d[3], d[5], d[6], d[8], d[9], d[10], d[11], d[12]);
	}

	void DpcmChan::sim_FreqLFSR()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;

		TriState sout[9]{};

		for (size_t n = 0; n < 9; n++)
		{
			sout[n] = lfsr[n].get_sout();
		}

		TriState nor1_out = NOR9(sout[0], sout[1], sout[2], sout[3], sout[4], sout[5], sout[6], sout[7], sout[8]);
		TriState nor2_out = NOR9(NOT(sout[0]), sout[1], sout[2], sout[3], sout[4], sout[5], sout[6], sout[7], sout[8]);

		TriState feedback = NOR3(AND(sout[0], sout[4]), RES, NOR3(sout[0], sout[4], nor1_out));
		TriState nor3_out = NOR(RES, NOT(nor2_out));
		DFLOAD = NOR(NOT(n_ACLK2), NOT(nor3_out));
		TriState DFSTEP = NOR(NOT(n_ACLK2), nor3_out);

		TriState sin = feedback;

		for (int n = 8; n >= 0; n--)
		{
			lfsr[n].sim(n_ACLK, DFLOAD, DFSTEP, FR[n], sin);
			sin = lfsr[n].get_sout();
		}
	}

	void DPCM_LFSRBit::sim(TriState n_ACLK, TriState load, TriState step, TriState val, TriState sin)
	{
		in_latch.set(MUX(load, MUX(step, TriState::Z, sin), val), TriState::One);
		out_latch.set(in_latch.nget(), n_ACLK);
	}

	TriState DPCM_LFSRBit::get_sout()
	{
		return out_latch.nget();
	}

	void DpcmChan::sim_SampleCounterReg()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W4013 = apu->wire.W4013;

		for (size_t n = 0; n < 8; n++)
		{
			scnt_reg[n].sim(n_ACLK, W4013, apu->GetDBBit(n));
		}
	}

	void DpcmChan::sim_SampleCounter()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;

		TriState carry = TriState::One;

		for (size_t n = 0; n < 12; n++)
		{
			carry = scnt[n].sim(carry, RES, DSLOAD, DSSTEP, n_ACLK, n < 4 ? TriState::Zero : scnt_reg[n - 4].get());
		}

		SOUT = carry;
	}

	void DpcmChan::sim_SampleBitCounter()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;

		TriState carry = TriState::One;

		for (size_t n = 0; n < 3; n++)
		{
			carry = sbcnt[n].sim(carry, RES, RES, NSTEP, n_ACLK, TriState::Zero);
		}

		nout_latch.set(carry, n_ACLK);
		n_NOUT = nout_latch.nget();
	}

	void DpcmChan::sim_SampleBuffer()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;

		for (size_t n = 0; n < 8; n++)
		{
			buf_reg[n].sim(n_ACLK, PCM, apu->GetDBBit(n));
		}

		TriState sin = TriState::Zero;

		for (int n = 7; n >= 0; n--)
		{
			shift_reg[n].sim(n_ACLK, RES, BLOAD, BSTEP, buf_reg[n].nget(), sin);
			sin = shift_reg[n].get_sout();
		}

		n_BOUT = NOT(shift_reg[0].get_sout());
	}

	void DPCM_SRBit::sim(TriState n_ACLK, TriState clear, TriState load, TriState step, TriState n_val, TriState sin)
	{
		TriState d = 
			MUX(clear, 
				MUX(load, 
					MUX(step, 
						MUX(n_ACLK, TriState::Z, NOT(out_latch.nget())), 
						in_latch.nget()), 
					n_val), 
				TriState::Zero);
		in_latch.set(sin, n_ACLK);
		out_latch.set(d, TriState::One);
	}

	TriState DPCM_SRBit::get_sout()
	{
		return out_latch.nget();
	}

#pragma endregion "DPCM Sampling"

#pragma region "DPCM Addressing & Output"

	void DpcmChan::sim_AddressReg()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W4012 = apu->wire.W4012;

		for (size_t n = 0; n < 8; n++)
		{
			addr_reg[n].sim(n_ACLK, W4012, apu->GetDBBit(n));
		}
	}

	void DpcmChan::sim_AddressCounter()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;

		TriState carry = TriState::One;

		for (size_t n = 0; n < 8; n++)
		{
			carry = addr_lo[n].sim(carry, RES, DSLOAD, DSSTEP, n_ACLK, n < 6 ? TriState::Zero : addr_reg[n - 6].get());
		}

		for (size_t n = 0; n < 7; n++)
		{
			carry = addr_hi[n].sim(carry, RES, DSLOAD, DSSTEP, n_ACLK, n < 6 ? addr_reg[n + 2].get() : TriState::One);
		}

		apu->DMC_Addr = 0;
		for (size_t n = 0; n < 8; n++)
		{
			apu->DMC_Addr |= (addr_lo[n].get() == TriState::One ? 1 : 0) << n;
			if (n < 7)
			{
				apu->DMC_Addr |= (addr_hi[n].get() == TriState::One ? 1 : 0) << (8 + n);
			}
		}
		apu->DMC_Addr |= 0x8000;
	}

	void DpcmChan::sim_Output()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;
		TriState W4011 = apu->wire.W4011;
		TriState CountDown = n_BOUT;

		TriState carry = TriState::One;

		for (size_t n = 0; n < 6; n++)
		{
			carry = out_cnt[n].sim(carry, CountDown, RES, W4011, DSTEP, n_ACLK, apu->GetDBBit(n + 1));
		}

		DOUT = carry;

		out_reg.sim(n_ACLK, W4011, apu->GetDBBit(0));

		apu->DMC_Out[0] = out_reg.get();
		for (size_t n = 0; n < 6; n++)
		{
			apu->DMC_Out[n + 1] = out_cnt[n].get();
		}
	}

#pragma endregion "DPCM Addressing & Output"

#pragma region "Debug"

	uint32_t DpcmChan::Get_FreqReg()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 4; n++)
		{
			val_lo[n] = freq_reg[n].get();
		}
		return PackNibble(val_lo);
	}

	uint32_t DpcmChan::Get_SampleReg()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = scnt_reg[n].get();
		}
		return Pack(val_lo);
	}

	uint32_t DpcmChan::Get_SampleCounter()
	{
		TriState val_lo[8]{};
		TriState val_hi[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = scnt[n].get();
		}
		for (size_t n = 0; n < 4; n++)
		{
			val_hi[n] = scnt[n + 8].get();
		}
		return Pack(val_lo) | ((uint32_t)PackNibble(val_hi) << 8);
	}

	uint32_t DpcmChan::Get_SampleBuffer()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = buf_reg[n].get();
		}
		return Pack(val_lo);
	}

	uint32_t DpcmChan::Get_SampleBitCounter()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 3; n++)
		{
			val_lo[n] = sbcnt[n].get();
		}
		val_lo[3] = TriState::Zero;
		return PackNibble(val_lo);
	}

	uint32_t DpcmChan::Get_AddressReg()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = addr_reg[n].get();
		}
		return Pack(val_lo);
	}

	uint32_t DpcmChan::Get_AddressCounter()
	{
		TriState val_lo[8]{};
		TriState val_hi[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = addr_lo[n].get();
		}
		for (size_t n = 0; n < 7; n++)
		{
			val_hi[n] = addr_hi[n].get();
		}
		val_hi[7] = TriState::One;
		return Pack(val_lo) | ((uint32_t)Pack(val_hi) << 8);
	}

	uint32_t DpcmChan::Get_Output()
	{
		TriState val_lo[8]{};
		val_lo[0] = out_reg.get();
		for (size_t n = 0; n < 6; n++)
		{
			val_lo[n + 1] = out_cnt[n].get();
		}
		val_lo[7] = TriState::Zero;
		return Pack(val_lo);
	}

	void DpcmChan::Set_FreqReg(uint32_t value)
	{
		TriState val_lo[8]{};
		UnpackNibble(value, val_lo);
		for (size_t n = 0; n < 4; n++)
		{
			freq_reg[n].set(val_lo[n]);
		}
	}

	void DpcmChan::Set_SampleReg(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		for (size_t n = 0; n < 8; n++)
		{
			scnt_reg[n].set(val_lo[n]);
		}
	}

	void DpcmChan::Set_SampleCounter(uint32_t value)
	{
		TriState val_lo[8]{};
		TriState val_hi[8]{};
		Unpack(value, val_lo);
		UnpackNibble(value >> 8, val_hi);
		for (size_t n = 0; n < 8; n++)
		{
			scnt[n].set(val_lo[n]);
		}
		for (size_t n = 0; n < 4; n++)
		{
			scnt[n + 8].set(val_hi[n]);
		}
	}

	void DpcmChan::Set_SampleBuffer(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		for (size_t n = 0; n < 8; n++)
		{
			buf_reg[n].set(val_lo[n]);
		}
	}

	void DpcmChan::Set_SampleBitCounter(uint32_t value)
	{
		TriState val_lo[8]{};
		UnpackNibble(value, val_lo);
		for (size_t n = 0; n < 3; n++)
		{
			sbcnt[n].set(val_lo[n]);
		}
	}

	void DpcmChan::Set_AddressReg(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		for (size_t n = 0; n < 8; n++)
		{
			addr_reg[n].set(val_lo[n]);
		}
	}

	void DpcmChan::Set_AddressCounter(uint32_t value)
	{
		TriState val_lo[8]{};
		TriState val_hi[8]{};
		Unpack(value, val_lo);
		Unpack(value >> 8, val_hi);
		for (size_t n = 0; n < 8; n++)
		{
			addr_lo[n].set(val_lo[n]);
		}
		for (size_t n = 0; n < 7; n++)
		{
			addr_hi[n].set(val_hi[n]);
		}
	}

	void DpcmChan::Set_Output(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		out_reg.set(val_lo[0]);
		for (size_t n = 0; n < 6; n++)
		{
			out_cnt[n].set(val_lo[n + 1]);
		}
	}

#pragma endregion "Debug"
}
