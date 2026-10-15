// Differential Pulse-code Modulation (DPCM) Channel

#include "pch.h"
#include <cstring>
#ifdef FASTSIM_SKIP_ORACLE
#include <cstdlib>
#endif

using namespace BaseLogic;

namespace APUSimFast
{
	// One ordered transition of the four binary DMA storage cells. Inputs are
	// ACLK1, ACLK2, PHI1, RnW, RES, /enable-delay, /DMA-stop (low to high).
	static constexpr uint8_t DpcmDMATransition(unsigned state, unsigned input)
	{
		const unsigned a1 = input & 1u, a2 = (input >> 1) & 1u;
		unsigned run1 = state & 1u, run2 = (state >> 1) & 1u;
		unsigned start = (state >> 2) & 1u, ready = (state >> 3) & 1u;
		if (a2) run1 = start;
		if (a1) run2 = run1 ^ 1u;
		const unsigned stop = (input >> 6) & 1u, delay = (input >> 5) & 1u;
		const unsigned request = ((input >> 3) & 1u) & (((input >> 2) & 1u) ^ 1u) & (delay ^ 1u) & stop;
		start = (start | request) & (delay ^ 1u) & (((input >> 4) & 1u) ^ 1u) & stop;
		ready = ((run1 & a1) ^ 1u) & (ready | a2);
		return uint8_t(run1 | (run2 << 1) | (start << 2) | (ready << 3) |
			((run2 ^ 1u) << 4) | (ready << 5) | (((start ^ 1u) & ready) << 6));
	}
	static constexpr std::array<uint8_t, 2048> MakeDpcmDMATable()
	{
		std::array<uint8_t, 2048> result{};
		for (unsigned i = 0; i < result.size(); ++i) result[i] = DpcmDMATransition(i & 15u, i >> 4);
		return result;
	}
	#ifdef APUSIMFAST_TABLE_DMA_CONTROL
	// Permit ordinary static initialization on compilers with small constexpr
	// step limits. This immutable table is shared and outside measured chip calls.
	static const auto DpcmDMATable = MakeDpcmDMATable();
	#endif

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
		// Two bits per TriState: X and Z stay distinct from 0 and 1 and cannot spill into the
		// neighbouring fields (round 2; the binary keys are unchanged up to bit position).
		const uint32_t key = (unsigned(w.ACLK1) & 3u) | ((unsigned(w.nACLK2) & 3u) << 2) |
			((unsigned(w.PHI1) & 3u) << 4) | ((unsigned(w.RnW) & 3u) << 6) | ((unsigned(w.RES) & 3u) << 8) |
			((unsigned(w.n_R4015) & 3u) << 10) | ((unsigned(w.W4010) & 3u) << 12) | ((unsigned(w.W4011) & 3u) << 14) |
			((unsigned(w.W4012) & 3u) << 16) | ((unsigned(w.W4013) & 3u) << 18) | ((unsigned(w.W4015) & 3u) << 20) |
			((unsigned(w.LOCK) & 3u) << 22) | (uint32_t(captures ? apu->DB : 0) << 24);
		#ifndef APUSIMFAST_EVALUATE_EVERY_CALL
		if (!phase.begin(*this, key, apu->mutation_epoch))
		{
#ifdef FASTSIM_SKIP_ORACLE
			const uint8_t bus = apu->DB;
#endif
			// Reads drive the live bus even when no sequential state changed.
			if (w.n_R4015 == TriState::Zero)
			{
				apu->SetDBBit(7, int_ff.get());
				apu->SetDBBit(4, ena_ff.get());
			}
#ifdef FASTSIM_SKIP_ORACLE
			sim_SkipOracle(bus);
#endif
			return;
		}
		#endif

		const bool fixed_point = sim_Evaluate();
		#ifndef APUSIMFAST_EVALUATE_EVERY_CALL
		phase.end(*this, fixed_point);
		#endif
	}

	// One evaluation of the block in the original stage order. Returns true when a replay with
	// the same inputs would be the identity (round 2). Common conditions: the DMA controller took
	// its binary path; the six temporaries an evaluation reads before it rewrites them (ED1, SOUT,
	// BLOAD, PCM, DMC1, DOUT) did not change, so the flip-flop equations, absorbing in their own
	// value (int' = (int | ED1) & mask, step' = (step | BLOAD) & mask, stop' = (stop | PCM) & mask,
	// pcm' = (pcm | PCM) & mask), store the same values again; stop_ff, read by the DMA controller
	// as CTRL1 before SampleBufferControl rewrites it, did not change, so request and start' =
	// (start | request) & mask repeat themselves. Then either
	//  - ACLK1 is Zero: no ACLK1 latch captured, the counters' carry banks, the LFSR output bank
	//    and the sample shifter's input bank are frozen, the ACLK1-low register loads rewrite the
	//    same data-bus value, and start, read by run1 under ACLK2 before it is rewritten, did not
	//    change (ready' = ready | ACLK2 is absorbing); or
	//  - ACLK1 is One and ACLK2 Zero: PCMDone, DMC1, DMC3, DFLOAD and with them DSLOAD, DSSTEP,
	//    NSTEP, DSTEP, BLOAD and BSTEP are Zero, so no counter loads or steps, the shifter only
	//    recopies its output and the carry banks are recomputed from unchanged main banks; the
	//    ACLK1 latches took other cells' new values (dstep/stop/pcm latches), the unchanged carried
	//    temporaries (sout_latch, dout_latch), the unchanged ED2 (en_latch1) and the frozen
	//    en_latch2 (en_latch3), which a replay captures identically; run1 is not clocked, run2 and
	//    ready' = !run1 & ready are absorbing; en_latch3, read by the DMA controller as CTRL2 before
	//    SampleCounterControl rewrites it, did not change.
	// Every other value is a function of the frozen or unchanged state and of the same inputs.
	bool DpcmChan::sim_Evaluate()
	{
		const TriState carried_before[6] = { ED1, SOUT, BLOAD, PCM, DMC1, DOUT };
		const TriState stop_before = stop_ff.get();
		const TriState en3_before = en_latch3.get();
		const uint8_t start_before = uint8_t(dma_control & 4u);

		ACLK2 = NOT(apu->wire.nACLK2);

		sim_SampleCounterReg();

		sim_ControlReg();
		sim_Decoder2();

		sim_FreqLFSR();

		sim_IntControl();
		sim_EnableControl();
		const bool binary = sim_DMAControl();

		sim_SampleCounterControl();
		sim_SampleCounter();
		sim_SampleBitCounter();

		sim_SampleBufferControl();
		sim_SampleBuffer();

		sim_AddressReg();
		sim_AddressCounter();
		sim_Output();

		const TriState carried_after[6] = { ED1, SOUT, BLOAD, PCM, DMC1, DOUT };
		if (!binary || std::memcmp(carried_before, carried_after, sizeof(carried_before)) != 0 || stop_before != stop_ff.get())
			return false;
		if (apu->wire.ACLK1 == TriState::Zero) return start_before == uint8_t(dma_control & 4u);
		return apu->wire.ACLK1 == TriState::One && ACLK2 == TriState::Zero && en3_before == en_latch3.get();
	}

#ifdef FASTSIM_SKIP_ORACLE
	// A skipped evaluation must be a replay: re-run it from the bus the call started with (`bus`,
	// before the skip path's own status drive, as the register loads sample DB before the status
	// read drives bits 7 and 4) and abort if the block (snapshot and dead temporaries alike), the
	// wires, the driven bus, the address or the audio differ.
	void DpcmChan::sim_SkipOracle(uint8_t bus)
	{
		unsigned char before_block[sizeof(DpcmChan)];
		std::memcpy(before_block, this, sizeof(DpcmChan));
		const auto before_wires = apu->wire;
		const uint8_t driven = apu->DB; const uint16_t addr = apu->DMC_Addr; const uint32_t word = apu->audio_word;
		TriState out[8]; std::memcpy(out, apu->DMC_Out, sizeof(out));
		apu->DB = bus;
		sim_Evaluate();
		const bool same = std::memcmp(before_block, this, sizeof(DpcmChan)) == 0 &&
			std::memcmp(&before_wires, &apu->wire, sizeof(before_wires)) == 0 && driven == apu->DB &&
			addr == apu->DMC_Addr && word == apu->audio_word && std::memcmp(out, apu->DMC_Out, sizeof(out)) == 0;
		if (!same)
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: DPCM skipped evaluation was not a replay (DB %02x->%02x addr %04x->%04x word %06x->%06x)\n",
				driven, apu->DB, addr, apu->DMC_Addr, word, apu->audio_word);
			std::abort();
		}
	}
#endif

#pragma region "DPCM Control"

	void DpcmChan::sim_ControlReg()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4010 = apu->wire.W4010;

		if ((ACLK1 & 1) == 0 && (W4010 & 1) != 0) freq_reg = apu->DB & 15;

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

	bool DpcmChan::sim_DMAControl()
	{
		const auto& w = apu->wire;
		const unsigned input = unsigned(w.ACLK1) | (unsigned(ACLK2) << 1) |
			(unsigned(w.PHI1) << 2) | (unsigned(w.RnW) << 3) | (unsigned(w.RES) << 4) |
			(unsigned(get_CTRL2()) << 5) | (unsigned(get_CTRL1()) << 6);
		if (input >= 128)
		{
			// Canonical X/Z diagnostics use the original primitive semantics;
			// they must never become an out-of-range transition-table index.
			TriState run1 = TriState(dma_control & 1u), run2 = TriState((dma_control >> 1) & 1u);
			TriState start = TriState((dma_control >> 2) & 1u), ready = TriState((dma_control >> 3) & 1u);
			if (ACLK2 == TriState::One) run1 = start;
			if (w.ACLK1 == TriState::One) run2 = NOT(run1);
			const TriState request = NOR3(NAND(w.RnW, NOT(w.PHI1)), get_CTRL2(), NOT(get_CTRL1()));
			start = NOR4(NOR(start, request), get_CTRL2(), w.RES, NOT(get_CTRL1()));
			ready = NOR(AND(run1, w.ACLK1), NOR(ready, ACLK2));
			dma_control = uint8_t(unsigned(run1) | (unsigned(run2) << 1) | (unsigned(start) << 2) | (unsigned(ready) << 3));
			apu->wire.RUNDMC = NOT(run2); apu->wire.n_DMC_AB = ready;
			apu->wire.DMCRDY = NOR(start, NOT(ready));
			return false;
		}
		#ifndef APUSIMFAST_TABLE_DMA_CONTROL
		const uint8_t result = DpcmDMATransition(dma_control, input);
		#else
		const uint8_t result = DpcmDMATable[(input << 4) | dma_control];
		#endif
		dma_control = result & 15u;
		apu->wire.RUNDMC = TriState((result >> 4) & 1u);
		apu->wire.n_DMC_AB = TriState((result >> 5) & 1u);
		apu->wire.DMCRDY = TriState((result >> 6) & 1u);
		return true;
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
		frequency_reload = DpcmPeriod[freq_reg];
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
			lfsr_input = frequency_reload;
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
