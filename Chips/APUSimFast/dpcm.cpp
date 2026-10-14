// Differential Pulse-code Modulation (DPCM) Channel

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
{
	static constexpr uint16_t DpcmPeriod[16] = { 0x173, 0x08A, 0x143, 0x0DB, 0x1EE, 0x03F, 0x07D, 0x1D1, 0x009, 0x0DC, 0x0F1, 0x0F9, 0x08D, 0x189, 0x18E, 0x156 };
	DpcmChan::DpcmChan(APU* parent)
	{
		apu = parent;
		// msb is not used. This is done for the convenience of packing the value in byte.
		apu->DMC_Out[7] = TriState::Zero;
	}

	DpcmChan::~DpcmChan()
	{
	}

	void DpcmChan::sim()
	{
		const auto& w = apu->wire;
		const bool captures = (w.W4010 & 1) != 0 || (w.W4011 & 1) != 0 ||
			(w.W4012 & 1) != 0 || (w.W4013 & 1) != 0 || (w.W4015 & 1) != 0 ||
			(w.PHI1 == TriState::Zero && w.n_DMC_AB == TriState::Zero);
		const std::array<uint8_t, 14> key = { uint8_t(w.ACLK1), uint8_t(w.nACLK2),
			uint8_t(w.PHI1), uint8_t(w.RnW), uint8_t(w.RES), uint8_t(w.n_R4015),
			uint8_t(w.W4010), uint8_t(w.W4011), uint8_t(w.W4012), uint8_t(w.W4013),
			uint8_t(w.W4015), uint8_t(w.LOCK), uint8_t(captures ? apu->DB : 0), 0 };
		if (!phase.begin(*this, key, apu->mutation_epoch))
		{
			// Reads drive the live bus even when no sequential state changed.
			if (w.n_R4015 == TriState::Zero)
			{
				apu->SetDBBit(7, int_ff.get());
				apu->SetDBBit(4, ena_ff.get());
			}
			return;
		}

		ACLK2 = NOT(apu->wire.nACLK2);

		sim_SampleCounterReg();
		
		sim_ControlReg();
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
		phase.end(*this);
	}

#pragma region "DPCM Control"

	void DpcmChan::sim_ControlReg()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4010 = apu->wire.W4010;

		if ((ACLK1 & 1) == 0 && (W4010 & 1) != 0) freq_reg = apu->DB & 15;
		for (unsigned n = 0; n < 4; ++n) Fx[n] = ((freq_reg >> n) & 1) ? TriState::One : TriState::Zero;

		loop_reg.sim(ACLK1, W4010, apu->GetDBBit(6));
		LOOPMode = loop_reg.get();

		irq_reg.sim(ACLK1, W4010, apu->GetDBBit(7));
		n_IRQEN = irq_reg.nget();
	}

	void DpcmChan::sim_IntControl()
	{
		TriState W4015 = apu->wire.W4015;
		TriState n_R4015 = apu->wire.n_R4015;
		TriState RES = apu->wire.RES;
		TriState AssertInt = ED1;

		int_ff.set (NOR4(NOR(int_ff.get(), AssertInt), W4015, n_IRQEN, RES));
		apu->SetDBBit(7, NOT(n_R4015) == TriState::One ? NOT(int_ff.nget()) : TriState::Z);
		apu->wire.DMCINT = NOR(int_ff.nget(), n_IRQEN);
	}

	void DpcmChan::sim_EnableControl()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4015 = apu->wire.W4015;
		TriState n_R4015 = apu->wire.n_R4015;
		TriState RES = apu->wire.RES;
		TriState PCMDone = get_DMC1();

		sout_latch.set(SOUT, ACLK1);
		DMC2 = sout_latch.get();
		ED1 = NOR3(LOOPMode, sout_latch.nget(), NOT(PCMDone));
		ena_ff.sim(ACLK1, W4015, apu->GetDBBit(4), ED1, RES);
		ED2 = ena_ff.get();
		apu->SetDBBit(4, NOT(n_R4015) == TriState::One ? NOT(ena_ff.nget()) : TriState::Z);
	}

	void DpcmChan::sim_DMAControl()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState PHI1 = apu->wire.PHI1;
		TriState RnW = apu->wire.RnW;
		TriState RES = apu->wire.RES;
		TriState nDMCEnableDelay = get_CTRL2();
		TriState nDMAStop = get_CTRL1();

		run_latch1.set(start_ff.get(), ACLK2);
		run_latch2.set(run_latch1.nget(), ACLK1);
		TriState start_set = NOR3(NAND(RnW, NOT(PHI1)), nDMCEnableDelay, NOT(nDMAStop));
		start_ff.set(NOR4(NOR(start_ff.get(), start_set), nDMCEnableDelay, RES, NOT(nDMAStop)));
		rdy_ff.set(NOR(AND(run_latch1.get(), ACLK1), NOR(rdy_ff.get(), ACLK2)));

		apu->wire.RUNDMC = run_latch2.nget();
		apu->wire.n_DMC_AB = rdy_ff.get();
		apu->wire.DMCRDY = NOR(start_ff.get(), rdy_ff.nget());
	}

	void DpcmChan::sim_SampleCounterControl()
	{
		TriState nACLK2 = apu->wire.nACLK2;
		TriState ACLK1 = apu->wire.ACLK1;
		TriState PCMDone = get_DMC1();
		TriState DMCFinish = DMC2;
		TriState DMCEnable = ED2;

		en_latch1.set(DMCEnable, ACLK1);
		en_latch2.set(en_latch1.nget(), ACLK2);
		en_latch3.set(en_latch2.nget(), ACLK1);
		CTRL2 = en_latch3.nget();
		TriState DMC3 = NOR3(nACLK2, en_latch1.nget(), en_latch3.get());

		NSTEP = NOT(NOT(DFLOAD));
		DSLOAD = NOT(NOR(AND(DMCFinish, PCMDone), DMC3));
		DSSTEP = NOR3(NOT(PCMDone), DMC3, DMCFinish);
	}

	void DpcmChan::sim_SampleBufferControl()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState RES = apu->wire.RES;
		TriState PHI1 = apu->wire.PHI1;
		TriState n_DMC_AB = apu->wire.n_DMC_AB;
		TriState LOCK = apu->wire.LOCK;
		TriState n_DFLOAD = NOT(DFLOAD);

		step_ff.set(NOR3(NOR(step_ff.get(), BLOAD), NOR3(NOT(stop_latch.nget()), n_DFLOAD, n_NOUT), RES));
		stop_ff.set(NOR3(NOR(stop_ff.get(), PCM), BLOAD, RES));
		CTRL1 = stop_ff.nget();
		pcm_ff.set(NOR3(NOR(pcm_ff.get(), PCM), DMC1, RES));

		dout_latch.set(DOUT, ACLK1);
		dstep_latch.set(step_ff.nget(), ACLK1);
		stop_latch.set(stop_ff.get(), ACLK1);
		pcm_latch.set(pcm_ff.nget(), ACLK1);

		PCM = NOR(PHI1, n_DMC_AB);
		DMC1 = NOR(pcm_latch.get(), NOT(ACLK2));
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
		return NOR(pcm_latch.get(), NOT(ACLK2));
	}

#pragma endregion "DPCM Control"

#pragma region "DPCM Sampling"

	void DpcmChan::sim_Decoder2()
	{
		const uint16_t period = DpcmPeriod[PackNibble(Fx)];
		for (size_t n = 0; n < 9; n++)
			FR[n] = ((period >> n) & 1u) != 0 ? TriState::One : TriState::Zero;
	}

	void DpcmChan::sim_FreqLFSR()
	{
		const TriState ACLK1 = apu->wire.ACLK1;
		const bool reset = apu->wire.RES == TriState::One;
		const unsigned a = lfsr_output & 1u, b = (lfsr_output >> 4) & 1u;
		const unsigned feedback = !reset && !(a & b) && (a | b | (lfsr_output == 0));
		const bool reload = !reset && lfsr_output == 1;
		DFLOAD = ACLK2 == TriState::One && reload ? TriState::One : TriState::Zero;
		const bool step = ACLK2 == TriState::One && !reload;
		if (DFLOAD == TriState::One)
		{
			lfsr_input = 0;
			for (unsigned n = 0; n < 9; ++n) lfsr_input |= uint16_t(FR[n] == TriState::One) << n;
		}
		else if (step)
			lfsr_input = ACLK1 == TriState::One ? (feedback ? 0x1ff : 0) : uint16_t((lfsr_output >> 1) | (feedback << 8));
		if (ACLK1 == TriState::One) lfsr_output = lfsr_input;
	}

	void DPCM_LFSRBit::sim(TriState ACLK1, TriState load, TriState step, TriState val, TriState shift_in)
	{
		in_latch.set(MUX(load, MUX(step, TriState::Z, shift_in), val), TriState::One);
		out_latch.set(in_latch.nget(), ACLK1);
	}

	TriState DPCM_LFSRBit::get_sout()
	{
		return out_latch.nget();
	}

	void DpcmChan::sim_SampleCounterReg()
	{
		if ((apu->wire.ACLK1 & 1) == 0 && (apu->wire.W4013 & 1) != 0) scnt_reg = apu->DB;
	}

	void DpcmChan::sim_SampleCounter()
	{
		// The original load word is register << 4, not the NES-visible length + 1.
		const uint16_t value = (DSLOAD & 1) != 0 ? (uint16_t)(Get_SampleReg() << 4) : 0;
		scnt.sim(apu->wire.RES, DSLOAD, DSSTEP, apu->wire.ACLK1, value);
		SOUT = scnt.get_carry();
	}

	void DpcmChan::sim_SampleBitCounter()
	{
		sbcnt.sim(apu->wire.RES, apu->wire.RES, NSTEP, apu->wire.ACLK1, 0);
		nout_latch.set(sbcnt.get_carry(), apu->wire.ACLK1);
		n_NOUT = nout_latch.nget();
	}

	void DpcmChan::sim_SampleBuffer()
	{
		const TriState clock = apu->wire.ACLK1;
		if ((clock & 1) == 0 && (PCM & 1) != 0) buf_reg = apu->DB;
		// Descending scalar order uses each just-updated output as the next
		// stage's input while ACLK1 is transparent. The serial high input is 0.
		if ((apu->wire.RES & 1) != 0)
		{
			shift_output = 0xff;
			if (clock == TriState::One) shift_input = 0x7f;
		}
		else if ((BLOAD & 1) != 0)
		{
			shift_output = buf_reg;
			if (clock == TriState::One) shift_input = buf_reg >> 1;
		}
		else if ((BSTEP & 1) != 0)
		{
			if (clock == TriState::One) shift_input = 0;
			shift_output = shift_input;
		}
		else if (clock == TriState::One) shift_input = shift_output >> 1;
		n_BOUT = (shift_output & 1) != 0 ? TriState::Zero : TriState::One;
	}

	void DPCM_SRBit::sim(TriState ACLK1, TriState clear, TriState load, TriState step, TriState n_val, TriState shift_in)
	{
		in_latch.set(shift_in, ACLK1);
		TriState d = 
			MUX(clear, 
				MUX(load, 
					MUX(step, 
						MUX(ACLK1, TriState::Z, NOT(out_latch.nget())), 
						in_latch.nget()), 
					n_val), 
				TriState::Zero);
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
		if ((apu->wire.ACLK1 & 1) == 0 && (apu->wire.W4012 & 1) != 0) addr_reg = apu->DB;
	}

	void DpcmChan::sim_AddressCounter()
	{
		const uint16_t value = (DSLOAD & 1) != 0
			? (uint16_t)((Get_AddressReg() << 6) | 0x4000u) : 0;
		address_counter.sim(apu->wire.RES, DSLOAD, DSSTEP, apu->wire.ACLK1, value);
		apu->DMC_Addr = address_counter.get() | 0x8000u;
	}

	void DpcmChan::sim_Output()
	{
		out_cnt.sim(n_BOUT, apu->wire.RES, apu->wire.W4011, DSTEP, apu->wire.ACLK1, apu->DB >> 1);
		DOUT = out_cnt.get_carry(n_BOUT);
		out_reg.sim(apu->wire.ACLK1, apu->wire.W4011, apu->GetDBBit(0));
		apu->audio_word = (apu->audio_word & 0xffffu) | (uint32_t(Get_Output()) << 16);
		apu->DMC_Out[0] = out_reg.get();
		for (unsigned n = 0; n < 6; ++n)
			apu->DMC_Out[n + 1] = ((out_cnt.get() >> n) & 1) ? TriState::One : TriState::Zero;
	}

#pragma endregion "DPCM Addressing & Output"

#pragma region "Debug"

	uint32_t DpcmChan::Get_FreqReg()
	{
		return freq_reg;
	}

	uint32_t DpcmChan::Get_SampleReg()
	{
		return scnt_reg;
	}

	uint32_t DpcmChan::Get_SampleCounter()
	{
		return scnt.get();
	}

	uint32_t DpcmChan::Get_SampleBuffer()
	{
		return buf_reg;
	}

	uint32_t DpcmChan::Get_SampleBitCounter()
	{
		return sbcnt.get();
	}

	uint32_t DpcmChan::Get_AddressReg()
	{
		return addr_reg;
	}

	uint32_t DpcmChan::Get_AddressCounter()
	{
		return address_counter.get() | 0x8000u;
	}

	uint32_t DpcmChan::Get_Output()
	{
		return (out_cnt.get() << 1) | (out_reg.get() == TriState::One);
	}

	void DpcmChan::Set_FreqReg(uint32_t value)
	{
		freq_reg = value & 15;
	}

	void DpcmChan::Set_SampleReg(uint32_t value)
	{
		scnt_reg = uint8_t(value);
	}

	void DpcmChan::Set_SampleCounter(uint32_t value)
	{
		scnt.set((uint16_t)value);
	}

	void DpcmChan::Set_SampleBuffer(uint32_t value)
	{
		buf_reg = uint8_t(value);
	}

	void DpcmChan::Set_SampleBitCounter(uint32_t value)
	{
		sbcnt.set(uint16_t(value));
	}

	void DpcmChan::Set_AddressReg(uint32_t value)
	{
		addr_reg = uint8_t(value);
	}

	void DpcmChan::Set_AddressCounter(uint32_t value)
	{
		address_counter.set((uint16_t)value);
	}

	void DpcmChan::Set_Output(uint32_t value)
	{
		out_cnt.set(uint16_t(value >> 1));
		out_reg.set((value & 1) ? TriState::One : TriState::Zero);
	}

	bool DpcmChan::GetDpcmEnable()
	{
		return ena_ff.get() == TriState::One;
	}

	void DpcmChan::SetDpcmEnable(bool enable)
	{
		ena_ff.set(enable ? TriState::One : TriState::Zero);
	}

#pragma endregion "Debug"
}
