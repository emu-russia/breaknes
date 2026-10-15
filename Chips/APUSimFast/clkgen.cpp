// Timing Generator

#include "pch.h"
#include <cstring>
#ifdef FASTSIM_SKIP_ORACLE
#include <cstdlib>
#endif

using namespace BaseLogic;

namespace APUSimFast
{
	// The frame sequencer's product terms: one exact 15-bit output-bank word each.
	static constexpr uint16_t FramePoints[6] = { 0x1061, 0x3603, 0x2cd3, 0x0a1f, 0x7185, 0 };

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
		// Two bits per TriState: X and Z stay distinct from 0 and 1 and cannot spill into the
		// neighbouring fields (round 2; the binary keys are unchanged up to bit position).
		const uint32_t key = (unsigned(w.PHI1) & 3u) | ((unsigned(w.PHI2) & 3u) << 2) |
			((unsigned(w.RES) & 3u) << 4) | ((unsigned(w.W4017) & 3u) << 6) |
			((unsigned(w.DMCINT) & 3u) << 8) | ((unsigned(w.n_R4015) & 3u) << 10) |
			((w.W4017 & 1) != 0 ? unsigned(apu->DB & 0xc0) << 12 : 0u);
		#ifndef APUSIMFAST_FRAME_EVERY_CALL
		if (phase_stable && phase_key == key && phase_epoch == apu->mutation_epoch)
		{
			clock_changed = false;
#ifdef FASTSIM_SKIP_ORACLE
			const uint8_t bus = apu->DB;
#endif
			if (w.n_R4015 == TriState::Zero) apu->SetDBBit(6, TriState(1u ^ ((control >> 6) & 1u)));
#ifdef FASTSIM_SKIP_ORACLE
			sim_SkipOracle(bus);
#endif
			return;
		}
		const uint64_t before = feedback_state();
		#endif
		const TriState aclk1_before = w.ACLK1, naclk2_before = w.nACLK2;
		sim_Evaluate();
		clock_changed = aclk1_before != w.ACLK1 || naclk2_before != w.nACLK2;
		#ifndef APUSIMFAST_FRAME_EVERY_CALL
		phase_key = key;
		phase_epoch = apu->mutation_epoch;
		// Settled when the banks did not change, or when the evaluation is provably a fixed point
		// of the same inputs (IsFixedPoint): a replay would then leave banks, counter and outputs
		// unchanged, so the next call with this key may skip it.
		phase_stable = before == feedback_state() || IsFixedPoint();
		#endif
	}

	// One evaluation of the block in the original stage order.
	void CLKGen::sim_Evaluate()
	{
		sim_ACLK();
		sim_SoftCLK_Mode();
		sim_SoftCLK_PLA();
		sim_SoftCLK_Control();
		sim_SoftCLK_LFSR();
	}

	// True after an evaluation whose repetition with the same inputs would be the identity.
	// The block has no temporary that an evaluation reads before it writes, so a replay can
	// differ only through state the first evaluation changed and then re-read:
	//  - sim_ACLK: with PHI1 and PHI2 not both One, the phase it updated (bit0 under PHI1, bit1
	//    under PHI2) is not the phase it reads (bit1, respectively bit0), so the replay stores the
	//    same values; nACLK2/ACLK1 follow, and the ACLK counter sees no second negative edge;
	//  - the flip-flops are OR-absorbing in their own value: IRQ' = (IRQ | set) & mask and
	//    Z' = (Z | set) & mask with set/mask unchanged, so a second application is the identity;
	//  - with ACLK1 Zero every transparent latch (mode bit3, IRQ bit6, Z1/Z2 bits 8-9, the LFSR
	//    output bank) is closed, so nothing captured and nothing can ripple; the ACLK1-low register
	//    loads (bits 2 and 4, from DB with W4017) rewrite the same value; the LFSR input bank is a
	//    function of the unchanged output bank and of F1/F2, which follow Z1/Z2/PLA (unchanged);
	//  - with ACLK1 One and ACLK2 Zero (nACLK2 One) the latches did capture: bit3 and bit9 took
	//    n_mode and bit6 took !IRQ, which a replay recomputes identically; Z1 (bit8) took the Z
	//    flip-flop's old value and the LFSR output bank took the input bank. A replay reads the
	//    new Z through Z1 and decodes the new output through the PLA, so it is the identity when
	//    Z1 already equals Z (the flip-flop did not move) and the product terms of the new output
	//    equal the ones the evaluation decoded; F1/F2 are Zero without ACLK2, so the input bank is
	//    unchanged and the second copy stores the same output.
	// X/Z inputs are turned into fixed booleans by the stage code and behave the same way on a
	// replay; the PHI1/PHI2 overlap is the one input combination the first bullet needs.
	bool CLKGen::IsFixedPoint() const
	{
		const auto& w = apu->wire;
		if (w.PHI1 == TriState::One && w.PHI2 == TriState::One) return false;
		if (w.ACLK1 == TriState::Zero) return true;
		if (w.ACLK1 != TriState::One || w.nACLK2 != TriState::One) return false;
		if (((control >> 8) & 1u) != ((control >> 7) & 1u)) return false;
		for (unsigned n = 0; n < 6; ++n)
		{
			TriState term = lfsr_output == FramePoints[n] ? TriState::One : TriState::Zero;
			if (n == 3) term = AND(term, NOT(mode));
			if (term != pla[n]) return false;
		}
		return true;
	}

#ifdef FASTSIM_SKIP_ORACLE
	// A skipped evaluation must be a replay: re-run it from the bus the call started with (`bus`,
	// before the skip path's own status drive, since a $4017 write samples DB6/DB7 before the
	// status read drives DB6) and abort if the block (banks and temporaries), the wires, the
	// driven bus or the ACLK counter differ.
	void CLKGen::sim_SkipOracle(uint8_t bus)
	{
		unsigned char before_block[sizeof(CLKGen)];
		std::memcpy(before_block, this, sizeof(CLKGen));
		const auto before_wires = apu->wire;
		const uint8_t driven = apu->DB; const size_t aclk = apu->aclk_counter;
		apu->DB = bus;
		sim_Evaluate();
		if (std::memcmp(before_block, this, sizeof(CLKGen)) != 0 || std::memcmp(&before_wires, &apu->wire, sizeof(before_wires)) != 0 ||
			driven != apu->DB || aclk != apu->aclk_counter)
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: frame/clock skipped evaluation was not a replay (DB %02x->%02x)\n", driven, apu->DB);
			std::abort();
		}
	}
#endif

	void CLKGen::sim_ACLK()
	{
		const bool p1 = apu->wire.PHI1 == TriState::One;
		const bool p2 = apu->wire.PHI2 == TriState::One;
		const bool reset = apu->wire.RES == TriState::One;
		const TriState previous = apu->wire.nACLK2;
		unsigned state = control;
		const unsigned temp = !reset && ((state >> 1) & 1u);
		if (p1) state = (state & ~1u) | temp;
		if (p2) state = (state & ~2u) | ((1u ^ (state & 1u)) << 1);
		control = uint16_t(state);
		const TriState next = TriState(!p1 || temp);
		apu->wire.nACLK2 = next;
		apu->wire.ACLK1 = TriState(p1 && ((state >> 1) & 1u));
		if (IsNegedge(previous, next)) ++apu->aclk_counter;
	}

	void CLKGen::sim_SoftCLK_Mode()
	{
		unsigned state = control;
		const bool a1 = apu->wire.ACLK1 == TriState::One;
		if (!a1 && apu->wire.W4017 == TriState::One)
			state = (state & ~(1u << 2)) | (unsigned(apu->DB >> 7) << 2);
		n_mode = TriState(1u ^ ((state >> 2) & 1u));
		if (a1) state = (state & ~(1u << 3)) | (unsigned(n_mode) << 3);
		mode = TriState(1u ^ ((state >> 3) & 1u));
		control = uint16_t(state);
	}

	void CLKGen::sim_SoftCLK_Control()
	{
		unsigned state = control;
		const bool a1 = apu->wire.ACLK1 == TriState::One;
		const bool a2 = apu->wire.nACLK2 == TriState::Zero;
		const bool write = apu->wire.W4017 == TriState::One;
		const bool reset = apu->wire.RES == TriState::One;
		const bool read = apu->wire.n_R4015 == TriState::Zero;
		if (!a1 && write) state = (state & ~(1u << 4)) | (((unsigned(apu->DB) >> 6) & 1u) << 4);
		unsigned irq = (state >> 5) & 1u;
		irq = (irq | (unsigned(n_mode) & unsigned(pla[3]))) &
			unsigned(!((read && apu->wire.PHI1 == TriState::Zero) || ((state >> 4) & 1u) || reset));
		state = (state & ~(1u << 5)) | (irq << 5);
		if (a1) state = (state & ~(1u << 6)) | ((irq ^ 1u) << 6);
		if (read) apu->SetDBBit(6, TriState(1u ^ ((state >> 6) & 1u)));
		apu->wire.INT = TriState(irq | unsigned(apu->wire.DMCINT));

		// Preserve register/mode/IRQ, then Z1/Z2 sampling, then Z feedback.
		if (a1) state = (state & ~((1u << 8) | (1u << 9))) |
			(((state >> 7) & 1u) << 8) | (unsigned(n_mode) << 9);
		const unsigned first = (state >> 8) & 1u, second = (state >> 9) & 1u;
		const unsigned z = (((state >> 7) & 1u) | ((first ^ 1u) & unsigned(a2))) & unsigned(!write && !reset);
		state = (state & ~(1u << 7)) | (z << 7);
		Z1 = TriState(first ^ 1u);
		Z2 = TriState((first ^ 1u) & (second ^ 1u));
		const unsigned ftemp = first & (unsigned(pla[3]) ^ 1u) & (unsigned(pla[4]) ^ 1u);
		F1 = TriState((ftemp ^ 1u) & unsigned(a2));
		F2 = TriState(ftemp & unsigned(a2));
		apu->wire.n_LFO1 = TriState(!((pla[0] | pla[1] | pla[2] | pla[3] | pla[4] | Z2) && a2));
		apu->wire.n_LFO2 = TriState(!((pla[1] | pla[3] | pla[4] | Z2) && a2));
		control = uint16_t(state);
	}

	void CLKGen::sim_SoftCLK_PLA()
	{
		// Each product term accepts one exact 15-bit output-bank word.
		for (unsigned n = 0; n < 6; ++n)
			pla[n] = lfsr_output == FramePoints[n] ? TriState::One : TriState::Zero;
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
		return TriState((control >> 5) & 1u);
	}
}
