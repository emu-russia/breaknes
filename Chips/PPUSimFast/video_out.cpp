// Video Signal Generator

#include "pch.h"
#include <array>

using namespace BaseLogic;

#ifndef PPUSIM_FAST_VIDEO_TABLE
#define PPUSIM_FAST_VIDEO_TABLE 1
#endif
#ifndef PPUSIM_FAST_DAC_TABLE
#define PPUSIM_FAST_DAC_TABLE 1
#endif

namespace PPUSimFast
{
	struct PhaseStep { uint8_t input, output; uint16_t lines; uint32_t pal_gate; };
	static constexpr PhaseStep StepPhase(uint8_t input, uint8_t output, bool high, bool reset, bool low=true)
	{
		const uint8_t old_output = output;
		if (high) input = static_cast<uint8_t>((input & 0x1f) | ((old_output & 8) << 2));
		uint8_t values = reset ? 0 : static_cast<uint8_t>(~input) & 0x3f;
		const unsigned out5 = high || !low ? (old_output >> 5) & 1 : (values >> 5) & 1;
		const unsigned p1 = out5 == 0 && (((old_output >> 4) & 1) == 0 || (old_output & 8) != 0);
		if (high)
		{
			input = static_cast<uint8_t>(((static_cast<uint8_t>(~old_output) >> 1) & 0x0f) | (p1 << 4) | ((old_output & 8) << 2));
			values = reset ? 0 : static_cast<uint8_t>(~input) & 0x3f;
		}
		else if (low) output = values;
		const unsigned inverted = static_cast<uint8_t>(~output);
		const uint16_t lines = static_cast<uint16_t>(((values >> 5) & 1) | (p1 << 1) |
			(((values >> 4) & 1) << 2) | (((inverted >> 4) & 1) << 3) |
			(((values >> 3) & 1) << 5) | (((inverted >> 3) & 1) << 6) |
			(((values >> 2) & 1) << 7) | (((inverted >> 2) & 1) << 8) |
			(((values >> 1) & 1) << 9) | (((inverted >> 1) & 1) << 10) |
			((values & 1) << 11) | ((inverted & 1) << 12));
		const uint32_t gate = ((lines & 1) == 0 ? 3U : 0) | ((lines & 2) == 0 ? 3U << 2 : 0) |
			((lines & 4) == 0 ? 3U << 4 : 0) | ((lines & 8) == 0 ? 3U << 6 : 0) |
			((lines & 32) == 0 ? 3U << 8 : 0) | (1U << 10) |
			((lines & 64) == 0 ? 3U << 11 : 0) | ((lines & 128) == 0 ? 3U << 13 : 0) |
			((lines & 256) == 0 ? 3U << 15 : 0) | ((lines & 512) == 0 ? 3U << 17 : 0) |
			((lines & 1024) == 0 ? 3U << 19 : 0) | ((lines & 2048) == 0 ? 3U << 21 : 0) |
			((lines & 4096) == 0 ? 3U << 23 : 0);
		return {input,output,lines,gate};
	}
	struct CompactPhaseStep { uint8_t next; uint16_t lines; uint32_t pal_gate; };
	struct CompactPhaseData
	{
		std::array<PhaseStep,33> states{};
		std::array<CompactPhaseStep,198> steps{};
		unsigned count = 1;
	};
	static constexpr CompactPhaseData BuildCompactPhase()
	{
		CompactPhaseData result{};
		for (unsigned state = 0; state < result.count; state++)
		{
			for (unsigned control = 0; control < 6; control++)
			{
				const auto from = result.states[state];
				const auto next = StepPhase(from.input, from.output, control % 3 == 1, control >= 3, control % 3 == 0);
				unsigned id = 0;
				for (; id < result.count; id++) if (result.states[id].input == next.input && result.states[id].output == next.output) break;
				if (id == result.count)
				{
					if (result.count == result.states.size()) { result.count++; return result; }
					result.states[result.count++] = next;
				}
				result.steps[state * 6 + control] = {uint8_t(id),next.lines,next.pal_gate};
			}
		}
		return result;
	}
	static constexpr auto CompactPhase = BuildCompactPhase();
	static_assert(CompactPhase.count == 33, "phase transducer must close over exactly 33 bank pairs");
	// NTSC chroma: the phase-shifter line selected by each (burst-masked) color; -1 is gray
	// (/PZ = 1), -2 is color 15 (/PZ = 0).
	static constexpr int8_t NtscPhaseLine[16]{-1,-1,-1,0,8,2,10,5,12,7,1,9,3,11,6,-2};
	uint8_t VideoOut::get_PhaseInput() const { return CompactPhase.states[phase_state].input; }
	uint8_t VideoOut::get_PhaseOutput() const { return CompactPhase.states[phase_state].output; }

	VideoOut::VideoOut(PPU* parent)
	{
		ppu = parent;

		switch (ppu->rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:			// TBD
				SyncLevel[0] = 0.f;
				SyncLevel[1] = 0.525f;
				BurstLevel[0] = 0.300f;
				BurstLevel[1] = 0.841f;
				LumaLevel[0][0] = 0.366f;
				LumaLevel[0][1] = 1.091f;
				LumaLevel[1][0] = 0.525f;
				LumaLevel[1][1] = 1.500f;
				LumaLevel[2][0] = 0.966f;
				LumaLevel[2][1] = 1.941f;
				LumaLevel[3][0] = 1.558f;
				LumaLevel[3][1] = 1.941f;
				EmphasizedLumaLevel[0][0] = 0.266f;
				EmphasizedLumaLevel[0][1] = 0.825f;
				EmphasizedLumaLevel[1][0] = 0.391f;
				EmphasizedLumaLevel[1][1] = 1.133f;
				EmphasizedLumaLevel[2][0] = 0.733f;
				EmphasizedLumaLevel[2][1] = 1.466f;
				EmphasizedLumaLevel[3][0] = 1.166f;
				EmphasizedLumaLevel[3][1] = 1.466f;
				composite = true;
				break;

			case Revision::RP2C07_0:
				SyncLevel[0] = 0.f;
				SyncLevel[1] = 0.566f;
				BurstLevel[0] = 0.300f;
				BurstLevel[1] = 0.900f;
				LumaLevel[0][0] = 0.391f;
				LumaLevel[0][1] = 1.175f;
				LumaLevel[1][0] = 0.566f;
				LumaLevel[1][1] = 1.566f;
				LumaLevel[2][0] = 1.041f;
				LumaLevel[2][1] = 2.033f;
				LumaLevel[3][0] = 1.633f;
				LumaLevel[3][1] = 2.033f;
				EmphasizedLumaLevel[0][0] = 0.300f;
				EmphasizedLumaLevel[0][1] = 0.900f;
				EmphasizedLumaLevel[1][0] = 0.433f;
				EmphasizedLumaLevel[1][1] = 1.208f;
				EmphasizedLumaLevel[2][0] = 0.791f;
				EmphasizedLumaLevel[2][1] = 1.566f;
				EmphasizedLumaLevel[3][0] = 1.266f;
				EmphasizedLumaLevel[3][1] = 1.566f;
				composite = true;
				break;

			// The DAC is significantly redesigned, so the 6538 signal levels are different (a little brighter/more saturated).

			case Revision::UMC_UA6538:
				SyncLevel[0] = 0.f;
				SyncLevel[1] = 0.458f;
				BurstLevel[0] = 0.300f;
				BurstLevel[1] = 0.700f;
				LumaLevel[0][0] = 0.341f;
				LumaLevel[0][1] = 0.916f;
				LumaLevel[1][0] = 0.466f;
				LumaLevel[1][1] = 1.400f;
				LumaLevel[2][0] = 0.816f;
				LumaLevel[2][1] = 1.916f;
				LumaLevel[3][0] = 1.441f;
				LumaLevel[3][1] = 1.916f;
				EmphasizedLumaLevel[0][0] = 0.258f;
				EmphasizedLumaLevel[0][1] = 0.700f;
				EmphasizedLumaLevel[1][0] = 0.350f;
				EmphasizedLumaLevel[1][1] = 1.066f;
				EmphasizedLumaLevel[2][0] = 0.625f;
				EmphasizedLumaLevel[2][1] = 1.475f;
				EmphasizedLumaLevel[3][0] = 1.100f;
				EmphasizedLumaLevel[3][1] = 1.475f;
				composite = true;
				break;

			case Revision::RP2C04_0003:
				composite = false;
				SetupColorMatrix();
				break;

			// TBD: others (RGB, clones)
		}

		// For PAL-like composite PPUs, the Chroma decoder simulation is done using PLA, since the decoder there is big enough (so you don't have to bother with it).

		switch (ppu->rev)
		{
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
				SetupChromaDecoderPAL();
				break;
		}
		RebuildDacSamples();
	}

	VideoOut::~VideoOut()
	{
		if (chroma_decoder != nullptr)
		{
			delete chroma_decoder;
		}

		if (color_matrix != nullptr)
		{
			delete color_matrix;
		}
	}

	void VideoOut::sim(VideoOutSignal& vout)
	{
		// Pixel context reaches a fixed point within each actual PCLK phase for the
		// composite generator. The RGB generator does not: RGB_SEL12x3 absorbs an
		// emphasis change on any PCLK=0 call (ff |= ~n_Tx) and its output latches are
		// open on every PCLK=0 call, so a $2001 write landing between pixel edges
		// changes the reference output on that very call (v4 review finding
		// 2026-10-14, --test-rgb-emphasis: RP2C04-0003 reference red 245 vs stale 140).
		// RGB therefore evaluates every call, exactly as the reference does.
		// The caller includes reset/counter-reset forced repeats in pixel_invoked.
		// VideoGen-only instances remain arbitrary per-call input transducers.
		VideoInputsT<-1> in{};
		in.PCLK = ppu->wire.PCLK; in.n_PCLK = ppu->wire.n_PCLK;
		in.SYNC = ppu->fsm.SYNC; in.BURST = ppu->fsm.BURST; in.n_PICTURE = ppu->fsm.n_PICTURE;
		in.n_LL0 = ppu->wire.n_LL[0]; in.n_LL1 = ppu->wire.n_LL[1];
		in.context = ppu->cram == nullptr || ppu->pixel_invoked || !composite;
		sim_T(in, vout);
	}

	template<int Phase> void VideoOut::sim_Edge(const EdgeContextT<Phase>& c, VideoOutSignal& vout)
	{
		// An edge call has pixel_invoked set, so this is sim() with context = true; the commands
		// and luma bits are the ones the edge body just produced into the context.
		VideoInputsT<Phase> in{};
		in.SYNC = c.SYNC; in.BURST = c.BURST; in.n_PICTURE = c.n_PICTURE;
		in.n_LL0 = c.n_LL0; in.n_LL1 = c.n_LL1;
		in.context = true;
		sim_T(in, vout);
	}

	template<int Phase> void VideoOut::sim_T(const VideoInputsT<Phase>& in, VideoOutSignal& vout)
	{
		if (in.context) { sim_nPICTURE(in); sim_InputLatches(in); }
		if (raw) { sim_RAWOutput(vout); return; }
		const bool publish = in.context || mode_dirty;
		if (composite)
		{
			sim_PhaseShifter(ppu->wire.n_CLK, ppu->wire.CLK, ppu->wire.RES, in.n_PCLK);
			sim_ChromaDecoder();
			if (publish)
			{
				sim_OutputLatches(in);
				sim_LumaDecoder(in.n_LL0, in.n_LL1);
				dac_context = uint8_t(sync_latch.nget() | (black_latch.nget() << 1) | (cb_latch.nget() << 2) |
					(n_LU[0] << 3) | (n_LU[1] << 4) | (n_LU[2] << 5) | (n_LU[3] << 6));
			}
			sim_Emphasis(ppu->wire.n_TR, ppu->wire.n_TG, ppu->wire.n_TB);
#if PPUSIM_FAST_DAC_TABLE
			float value = dac_samples[dac_context | (unsigned(n_PZ) << 7) | (unsigned(TINT) << 8)];
			if (noise_enable && VidOut_n_PICTURE == TriState::Zero) value += GetNoise();
			vout.composite = value;
#else
			sim_CompositeDAC(vout);
#endif
		}
		else if (publish)
		{
			sim_ColorMatrix(in); sim_Select12To3(in); sim_OutputLatches(in); sim_RGB_DAC(vout);
		}
		else sim_RGB_DAC(vout);
		mode_dirty = false;
	}

	void VideoOut::sim_IdleComposite(VideoOutSignal& vout)
	{
		// Identical to the non-publishing composite branch of sim(): master-phase recurrence and
		// the output sample continue on every call; the input/output latches are settled.
		sim_PhaseShifter(ppu->wire.n_CLK, ppu->wire.CLK, ppu->wire.RES);
		sim_ChromaDecoder();
		sim_Emphasis(ppu->wire.n_TR, ppu->wire.n_TG, ppu->wire.n_TB);
#if PPUSIM_FAST_DAC_TABLE
		float value = dac_samples[dac_context | (unsigned(n_PZ) << 7) | (unsigned(TINT) << 8)];
		if (noise_enable && VidOut_n_PICTURE == TriState::Zero) value += GetNoise();
		vout.composite = value;
#else
		sim_CompositeDAC(vout);
#endif
	}

	/// <summary>
	/// Memorize the signal values on all the auxiliary latches.
	/// </summary>
	void VideoOut::sim_InputLatches()
	{
		VideoInputsT<-1> in{};
		in.PCLK = ppu->wire.PCLK; in.n_PCLK = ppu->wire.n_PCLK;
		in.SYNC = ppu->fsm.SYNC; in.BURST = ppu->fsm.BURST; in.n_PICTURE = ppu->fsm.n_PICTURE;
		in.n_LL0 = ppu->wire.n_LL[0]; in.n_LL1 = ppu->wire.n_LL[1];
		in.context = true;
		sim_InputLatches(in);
	}

	template<int Phase> void VideoOut::sim_InputLatches(const VideoInputsT<Phase>& in)
	{
		TriState PCLK = in.PCLK;
		TriState n_PCLK = in.n_PCLK;
		TriState SYNC = in.SYNC;

		if (PCLK == TriState::One) cc_input = ppu->cram != nullptr ? ppu->cram->get_n_Chroma() : static_cast<uint8_t>(
			ppu->wire.n_CC[0] | (ppu->wire.n_CC[1] << 1) | (ppu->wire.n_CC[2] << 2) | (ppu->wire.n_CC[3] << 3));
		if (n_PCLK == TriState::One) cc_output = static_cast<uint8_t>(~cc_input) & 15;

		if (composite)
		{
			TriState BURST = in.BURST;

			cc_burst_latch.set(BURST, n_PCLK);
			sync_latch.set(NOT(SYNC), n_PCLK);
			cb_latch.set(BURST, n_PCLK);
		}
		else
		{
			rgb_sync_latch[0].set(SYNC, n_PCLK);
			rgb_sync_latch[1].set(rgb_sync_latch[0].nget(), PCLK);
			rgb_sync_latch[2].set(rgb_sync_latch[1].nget(), n_PCLK);
		}
	}

	void VideoOut::sim_PhaseShifter(TriState n_CLK, TriState CLK, TriState RES)
	{
		sim_PhaseShifter(n_CLK, CLK, RES, ppu->wire.n_PCLK);
	}

	void VideoOut::sim_PhaseShifter(TriState n_CLK, TriState CLK, TriState RES, TriState n_PCLK)
	{
		const bool high = CLK == TriState::One, reset = RES == TriState::One;
		const unsigned control = (high ? 1U : n_CLK == TriState::One ? 0U : 2U) + (reset ? 3U : 0U);
#if PPUSIM_FAST_VIDEO_TABLE
		const auto& state = CompactPhase.steps[phase_state * 6 + control];
		phase_state = state.next; phase_lines = state.lines; pal_phase_gate = state.pal_gate;
#else
		const auto state = StepPhase(get_PhaseInput(),get_PhaseOutput(),high,reset,n_CLK == TriState::One);
		unsigned id = 0;
		for (; id < CompactPhase.count; id++) if (CompactPhase.states[id].input == state.input && CompactPhase.states[id].output == state.output) break;
		assert(id < CompactPhase.count);
		phase_state = uint8_t(id); phase_lines = state.lines; pal_phase_gate = state.pal_gate;
#endif
		if (ppu->rev == Revision::RP2C07_0 || ppu->rev == Revision::UMC_UA6538)
		{
			v0_latch.set(ppu->v == nullptr ? TriState::Zero : ppu->v->getBit(0),n_PCLK);
			const bool odd = v0_latch.get() == TriState::One;
			n_PR=static_cast<TriState>(((phase_lines >> (odd ? 7 : 2)) & 1) ^ 1);
			n_PG=static_cast<TriState>(((phase_lines >> (odd ? 2 : 7)) & 1) ^ 1);
		}
		else { n_PR=static_cast<TriState>(phase_lines & 1); n_PG=static_cast<TriState>((phase_lines >> 9) & 1); }
		n_PB=static_cast<TriState>((phase_lines >> 5) & 1);
	}

	void VideoOut::sim_ChromaDecoder()
	{
		if (chroma_decoder != nullptr)
		{
			sim_ChromaDecoder_PAL();
		}
		else
		{
			sim_ChromaDecoder_NTSC();
		}
	}

	void VideoOut::sim_ChromaDecoder_PAL()
	{
		const unsigned color = static_cast<uint8_t>(~cc_output) & (cc_burst_latch.get() == TriState::One ? 5 : 15);
		const unsigned key = (color << 1) | (v0_latch.get() == TriState::One ? 1U : 0U);
		PBLACK = (color & 14) == 0 ? TriState::One : TriState::Zero;
		n_PZ = (pal_colors[key] & pal_phase_gate) == 0 ? TriState::One : TriState::Zero;
	}

	void VideoOut::sim_ChromaDecoder_NTSC()
	{
		const unsigned color = static_cast<uint8_t>(~cc_output) & (cc_burst_latch.get() == TriState::One ? 7 : 15);
		PBLACK = (color & 14) == 0 ? TriState::One : TriState::Zero;
		const int line = NtscPhaseLine[color];
		n_PZ = line == -2 ? TriState::Zero : line < 0 ? TriState::One : static_cast<TriState>((phase_lines >> line) & 1);
	}

	void VideoOut::ArmIdle(const VideoOutSignal& vout)
	{
		// Called at the end of a full call whose video stages settled the pixel context. In the idle
		// class cc_output, the burst latch, the PAL V0 latch, /POUT, PBLACK, the emphasis bits and
		// dac_context do not change (pixel-body and register products); only the phase ring
		// advances, so /PZ, /PR, /PG, /PB and TINT become fixed functions of the phase lines.
		idle.rgb = vout;
		if (!composite) return;
		idle.pal = chroma_decoder != nullptr;
		const bool burst = cc_burst_latch.get() == TriState::One;
		const unsigned color = static_cast<uint8_t>(~cc_output) & (burst ? (idle.pal ? 5U : 7U) : 15U);
		if (idle.pal)
		{
			// sim_ChromaDecoder_PAL: /PZ = (pal_colors[key] & pal_phase_gate) == 0.
			const bool odd = v0_latch.get() == TriState::One;
			idle.pz_mask = pal_colors[(color << 1) | (odd ? 1U : 0U)];
			idle.pz_xor = 1;
			// sim_PhaseShifter (PAL): /PR = NOT(line[odd ? 7 : 2]), /PG = NOT(line[odd ? 2 : 7]).
			idle.pr_shift = odd ? 7 : 2;
			idle.pg_shift = odd ? 2 : 7;
			idle.prg_xor = 1;
		}
		else
		{
			// sim_ChromaDecoder_NTSC: /PZ = line[n] for n >= 0, One for gray, Zero for color 15.
			const int line = NtscPhaseLine[color];
			idle.pz_mask = line >= 0 ? 1U << line : 0U;
			idle.pz_xor = line == -1 ? 1 : 0;
			// sim_PhaseShifter (NTSC): /PR = line[0], /PG = line[9].
			idle.pr_shift = 0;
			idle.pg_shift = 9;
			idle.prg_xor = 0;
		}
		// sim_Emphasis: TINT = OR over (R,G,B) of NOR3(/POUT, /Px, /Tx); /PB = line[5] always.
		const bool visible = n_POUT == TriState::Zero;
		const bool tr = ppu->wire.n_TR == TriState::Zero, tg = ppu->wire.n_TG == TriState::Zero, tb = ppu->wire.n_TB == TriState::Zero;
		idle.tint_neg = !visible ? 0U : (tb ? 1U << 5 : 0U) | (!idle.pal && tg ? 1U << 9 : 0U) | (!idle.pal && tr ? 1U : 0U);
		idle.tint_pos = !visible || !idle.pal ? 0U : (tg ? 1U << idle.pg_shift : 0U) | (tr ? 1U << idle.pr_shift : 0U);
		idle.dac = dac_context;
	}

	void VideoOut::sim_Idle(VideoOutSignal& vout)
	{
		if (!composite)
		{
			// The RGB selector FFs and output latches are at their fixed point (ff |= ~n_Tx and the
			// color-matrix selection are idempotent within a PCLK phase without register traffic).
			vout = idle.rgb;
			return;
		}
#if PPUSIM_FAST_VIDEO_TABLE && PPUSIM_FAST_DAC_TABLE
		// Master-phase recurrence (RES is low in this class; an X/Z clock closes both banks).
		const unsigned control = ppu->wire.CLK == TriState::One ? 1U : ppu->wire.n_CLK == TriState::One ? 0U : 2U;
		const auto& step = CompactPhase.steps[phase_state * 6 + control];
		phase_state = step.next; phase_lines = step.lines; pal_phase_gate = step.pal_gate;
		const uint32_t lines = step.lines;
		n_PR = static_cast<TriState>(((lines >> idle.pr_shift) & 1) ^ idle.prg_xor);
		n_PG = static_cast<TriState>(((lines >> idle.pg_shift) & 1) ^ idle.prg_xor);
		n_PB = static_cast<TriState>((lines >> 5) & 1);
		const uint32_t source = idle.pal ? step.pal_gate : lines;
		n_PZ = static_cast<TriState>(((source & idle.pz_mask) != 0) ^ idle.pz_xor);
		TINT = static_cast<TriState>(((~lines & idle.tint_neg) | (lines & idle.tint_pos)) != 0);
		vout.composite = dac_samples[idle.dac | (unsigned(n_PZ) << 7) | (unsigned(TINT) << 8)];
#else
		sim_IdleComposite(vout);
#endif
	}

	/// <summary>
	/// Call after Chroma Decoder / SEL 12x3.
	/// </summary>
	template<int Phase> void VideoOut::sim_OutputLatches(const VideoInputsT<Phase>& in)
	{
		TriState n_PCLK = in.n_PCLK;
		TriState n_PICTURE = VidOut_n_PICTURE;

		if (composite)
		{
			TriState BURST = in.BURST;
			TriState SYNC = in.SYNC;

			// If /POUT = 1 - then the visible part of the signal is not output at all

			pic_out_latch.set(NOR(PBLACK, n_PICTURE), n_PCLK);
			n_POUT = pic_out_latch.nget();

			// For DAC

			black_latch.set(NOT(NOR3(NOR(PBLACK, n_PICTURE), SYNC, BURST)), n_PCLK);
		}
		else
		{
			if (n_PCLK != TriState::One) return;
			TriState red_in[3]{};
			TriState green_in[3]{};
			TriState blue_in[3]{};

			red_sel.getOut(red_in);
			green_sel.getOut(green_in);
			blue_sel.getOut(blue_in);

			rgb_red = n_PICTURE == TriState::Zero ? Pack3(red_in) : 0;
			rgb_green = n_PICTURE == TriState::Zero ? Pack3(green_in) : 0;
			rgb_blue = n_PICTURE == TriState::Zero ? Pack3(blue_in) : 0;
		}
	}

	void VideoOut::sim_LumaDecoder(TriState n_LL0, TriState n_LL1)
	{
		n_LU[3] = NOT(NOR3(n_POUT, n_LL0, n_LL1));
		n_LU[2] = NOT(NOR3(n_POUT, NOT(n_LL0), n_LL1));
		n_LU[1] = NOT(NOR3(n_POUT, n_LL0, NOT(n_LL1)));
		n_LU[0] = NOT(NOR3(n_POUT, NOT(n_LL0), NOT(n_LL1)));
	}

	void VideoOut::sim_Emphasis(TriState n_TR, TriState n_TG, TriState n_TB)
	{
		TriState n[3]{};
		n[0] = NOR3(n_POUT, n_PB, n_TB);
		n[1] = NOR3(n_POUT, n_PG, n_TG);
		n[2] = NOR3(n_POUT, n_PR, n_TR);
		TINT = NOT(NOR3(n[0], n[1], n[2]));
	}

	void VideoOut::sim_CompositeDAC(VideoOutSignal& vout)
	{
#if PPUSIM_FAST_DAC_TABLE
		const unsigned key = static_cast<unsigned>(sync_latch.nget()) |
			(static_cast<unsigned>(black_latch.nget()) << 1) |
			(static_cast<unsigned>(cb_latch.nget()) << 2) |
			(static_cast<unsigned>(n_LU[0]) << 3) | (static_cast<unsigned>(n_LU[1]) << 4) |
			(static_cast<unsigned>(n_LU[2]) << 5) | (static_cast<unsigned>(n_LU[3]) << 6) |
			(static_cast<unsigned>(n_PZ) << 7) | (static_cast<unsigned>(TINT) << 8);
		float v = dac_samples[key];
#else
		TriState tmp = TriState::Zero;
		float v = use_ext_levels ? ext_levels.LumaLevel[3][1] : LumaLevel[3][1];		// White level

		// Synch Low Level

		if (sync_latch.nget() == TriState::One)
		{
			v = std::min(v, use_ext_levels ? ext_levels.SyncLevel[0] : SyncLevel[0]);
		}

		// Synch High Level

		if (black_latch.nget() == TriState::One)
		{
			v = std::min(v, use_ext_levels ? ext_levels.SyncLevel[1] : SyncLevel[1]);
		}

		// Colorburst phase level

		v = PhaseSwing(v, cb_latch.nget(), use_ext_levels ? ext_levels.BurstLevel[0] : BurstLevel[0], use_ext_levels ? ext_levels.BurstLevel[1] : BurstLevel[1]);

		// Luminance phase levels

		if (TINT == TriState::One)
		{
			v = PhaseSwing(v, n_LU[0], use_ext_levels ? ext_levels.EmphasizedLumaLevel[0][0] : EmphasizedLumaLevel[0][0], use_ext_levels ? ext_levels.EmphasizedLumaLevel[0][1] : EmphasizedLumaLevel[0][1]);
			v = PhaseSwing(v, n_LU[1], use_ext_levels ? ext_levels.EmphasizedLumaLevel[1][0] : EmphasizedLumaLevel[1][0], use_ext_levels ? ext_levels.EmphasizedLumaLevel[1][1] : EmphasizedLumaLevel[1][1]);
			v = PhaseSwing(v, n_LU[2], use_ext_levels ? ext_levels.EmphasizedLumaLevel[2][0] : EmphasizedLumaLevel[2][0], use_ext_levels ? ext_levels.EmphasizedLumaLevel[2][1] : EmphasizedLumaLevel[2][1]);
			v = PhaseSwing(v, n_LU[3], use_ext_levels ? ext_levels.EmphasizedLumaLevel[3][0] : EmphasizedLumaLevel[3][0], use_ext_levels ? ext_levels.EmphasizedLumaLevel[3][1] : EmphasizedLumaLevel[3][1]);
		}
		else
		{
			v = PhaseSwing(v, n_LU[0], use_ext_levels ? ext_levels.LumaLevel[0][0] : LumaLevel[0][0], use_ext_levels ? ext_levels.LumaLevel[0][1] : LumaLevel[0][1]);
			v = PhaseSwing(v, n_LU[1], use_ext_levels ? ext_levels.LumaLevel[1][0] : LumaLevel[1][0], use_ext_levels ? ext_levels.LumaLevel[1][1] : LumaLevel[1][1]);
			v = PhaseSwing(v, n_LU[2], use_ext_levels ? ext_levels.LumaLevel[2][0] : LumaLevel[2][0], use_ext_levels ? ext_levels.LumaLevel[2][1] : LumaLevel[2][1]);
			v = PhaseSwing(v, n_LU[3], use_ext_levels ? ext_levels.LumaLevel[3][0] : LumaLevel[3][0], use_ext_levels ? ext_levels.LumaLevel[3][1] : LumaLevel[3][1]);
		}
#endif

		// In order not to torture the video decoder too much we will mix the noise only in the visible part of the line.

		if (noise_enable && VidOut_n_PICTURE == TriState::Zero)
		{
			v += GetNoise();
		}

		vout.composite = v;
	}

	void VideoOut::RebuildDacSamples()
	{
		// Preserve the scalar DAC's minimum order, including arbitrary external
		// levels. Only the digital selection is moved out of the half-call path.
		for (unsigned key=0;key<512;key++)
		{
			float value=use_ext_levels ? ext_levels.LumaLevel[3][1] : LumaLevel[3][1];
			if (key & 1) value=std::min(value,use_ext_levels ? ext_levels.SyncLevel[0] : SyncLevel[0]);
			if (key & 2) value=std::min(value,use_ext_levels ? ext_levels.SyncLevel[1] : SyncLevel[1]);
			const unsigned phase=(key>>7)&1;
			if ((key & 4)==0) value=std::min(value,use_ext_levels ? ext_levels.BurstLevel[phase^1] : BurstLevel[phase^1]);
			for (unsigned n=0;n<4;n++) if ((key & (8U<<n))==0)
			{
				const float level=(key & 256) ?
					(use_ext_levels ? ext_levels.EmphasizedLumaLevel[n][phase^1] : EmphasizedLumaLevel[n][phase^1]) :
					(use_ext_levels ? ext_levels.LumaLevel[n][phase^1] : LumaLevel[n][phase^1]);
				value=std::min(value,level);
			}
			dac_samples[key]=value;
		}
	}

	float VideoOut::PhaseSwing(float v, TriState sel, float level_from, float level_to)
	{
		auto tmp = NOR(sel, n_PZ);
		if (tmp == TriState::One)
		{
			v = std::min(v, level_to);
		}

		tmp = NOR(sel, tmp);
		if (tmp == TriState::One)
		{
			v = std::min(v, level_from);
		}
		return v;
	}

	void VideoOut::sim_RAWOutput(VideoOutSignal& vout)
	{
		if (VidOut_n_PICTURE == TriState::Zero)
		{
			vout.RAW.CC0 = cc_output & 1;
			vout.RAW.CC1 = (cc_output >> 1) & 1;
			vout.RAW.CC2 = (cc_output >> 2) & 1;
			vout.RAW.CC3 = (cc_output >> 3) & 1;
			vout.RAW.LL0 = ToByte(NOT(ppu->wire.n_LL[0]));
			vout.RAW.LL1 = ToByte(NOT(ppu->wire.n_LL[1]));
			vout.RAW.TR = ToByte(NOT(ppu->wire.n_TR));
			vout.RAW.TG = ToByte(NOT(ppu->wire.n_TG));
			vout.RAW.TB = ToByte(NOT(ppu->wire.n_TB));
		}
		else
		{
			vout.RAW.raw = 0;
		}

		if (composite)
		{
			vout.RAW.Sync = ToByte(sync_latch.nget());
		}
		else
		{
			vout.RAW.Sync = ToByte(NOT(rgb_sync_latch[2].nget()));
		}
	}

	void VideoOutSRBit::sim(TriState n_shift_in, TriState n_CLK, TriState CLK, TriState RES,
		TriState& shift_out, TriState& n_shift_out, TriState& val)
	{
		in_latch.set(n_shift_in, CLK);
		val = get_Out(RES);
		out_latch.set(val, n_CLK);
		shift_out = out_latch.get();
		n_shift_out = out_latch.nget();
	}

	TriState VideoOutSRBit::get_Out(TriState RES)
	{
		return NOR(in_latch.get(), RES);
	}

	TriState VideoOutSRBit::get_ShiftOut()
	{
		return out_latch.get();
	}

	TriState VideoOutSRBit::getn_ShiftOut()
	{
		return out_latch.nget();
	}

	/// <summary>
	/// The PAL PPU contains an additional DLatch chain for /PICTURE signal propagation.
	/// </summary>
	/// <returns></returns>
	template<int Phase> void VideoOut::sim_nPICTURE(const VideoInputsT<Phase>& in)
	{
		TriState PCLK = in.PCLK;
		TriState n_PCLK = in.n_PCLK;
		TriState n_PICTURE = in.n_PICTURE;

		switch (ppu->rev)
		{
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
				npicture_latch1.set(NOT(n_PICTURE), n_PCLK);
				npicture_latch2.set(npicture_latch1.nget(), PCLK);
				n_PICTURE = npicture_latch2.get();
				break;
		}

		VidOut_n_PICTURE = n_PICTURE;
	}

	void VideoOut::GetSignalFeatures(VideoSignalFeatures& features)
	{
		switch (ppu->rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:			// TBD
				features.SamplesPerPCLK = 8;
				features.PixelsPerScan = 341;
				features.ScansPerField = 262;
				features.BackPorchSize = 40;
				features.BlackLevel = 0.525f;
				features.WhiteLevel = 1.941f;
				features.SyncLevel = 0.f;
				features.PhaseAlteration = false;
				break;

			case Revision::RP2C07_0:
				features.SamplesPerPCLK = 10;
				features.PixelsPerScan = 341;
				features.ScansPerField = 312;
				features.BackPorchSize = 42;
				features.BlackLevel = 0.566f;
				features.WhiteLevel = 2.033f;
				features.SyncLevel = 0.f;
				features.PhaseAlteration = true;
				break;

			// The DAC is significantly redesigned, so the 6538 signal levels are different (a little brighter/more saturated).

			case Revision::UMC_UA6538:
				features.SamplesPerPCLK = 10;
				features.PixelsPerScan = 341;
				features.ScansPerField = 312;
				features.BackPorchSize = 42;
				features.BlackLevel = 0.458f;
				features.WhiteLevel = 1.916f;
				features.SyncLevel = 0.f;
				features.PhaseAlteration = true;
				break;

			case Revision::RP2C04_0003:
				features.SamplesPerPCLK = 8;
				features.PixelsPerScan = 341;
				features.ScansPerField = 262;
				features.BackPorchSize = 40;
				break;
		}

		features.Composite = IsComposite();
	}

	void VideoOut::SetRAWOutput(bool enable)
	{
		if (raw != enable) mode_dirty = true;
		raw = enable;
	}

	void VideoOut::ConvertRAWToRGB_Composite(VideoOutSignal& rawIn, VideoOutSignal& rgbOut)
	{
		VideoSignalFeatures features{};
		GetSignalFeatures(features);

		// Using a temporary PPU of the same revision, to simulate the video generator

		PPU vppu(ppu->rev, true);

		size_t warmup_halfcycles = 6;
		size_t num_phases = 12;
		VideoOutSignal unused{};

		// Latch all important signals

		vppu.wire.n_CC[0] = NOT(FromByte(rawIn.RAW.CC0));
		vppu.wire.n_CC[1] = NOT(FromByte(rawIn.RAW.CC1));
		vppu.wire.n_CC[2] = NOT(FromByte(rawIn.RAW.CC2));
		vppu.wire.n_CC[3] = NOT(FromByte(rawIn.RAW.CC3));
		vppu.wire.n_LL[0] = NOT(FromByte(rawIn.RAW.LL0));
		vppu.wire.n_LL[1] = NOT(FromByte(rawIn.RAW.LL1));
		vppu.wire.n_TR = NOT(FromByte(rawIn.RAW.TR));
		vppu.wire.n_TG = NOT(FromByte(rawIn.RAW.TG));
		vppu.wire.n_TB = NOT(FromByte(rawIn.RAW.TB));

		vppu.fsm.BURST = TriState::Zero;
		vppu.fsm.SYNC = FromByte(rawIn.RAW.Sync);
		vppu.fsm.n_PICTURE = TriState::Zero;

		vppu.wire.n_PCLK = TriState::Zero;
		vppu.wire.PCLK = TriState::One;
		vppu.vid_out->sim(unused);

		vppu.wire.n_PCLK = TriState::One;
		vppu.wire.PCLK = TriState::Zero;
		vppu.vid_out->sim(unused);

		// And again, for PAL PPU

		vppu.wire.n_PCLK = TriState::Zero;
		vppu.wire.PCLK = TriState::One;
		vppu.vid_out->sim(unused);

		vppu.wire.n_PCLK = TriState::One;
		vppu.wire.PCLK = TriState::Zero;
		vppu.vid_out->sim(unused);

		// Warm up the Phase Shifter, because once it starts, it gives out garbage.

		TriState CLK = TriState::Zero;

		for (size_t n = 0; n < warmup_halfcycles; n++)
		{
			vppu.wire.n_CLK = NOT(CLK);
			vppu.wire.CLK = CLK;

			vppu.vid_out->sim(unused);

			CLK = NOT(CLK);
		}

		// Get a video signal batch equal to the number of phases

		VideoOutSignal *batch = new VideoOutSignal[num_phases];

		for (size_t n = 0; n < num_phases; n++)
		{
			vppu.wire.n_CLK = NOT(CLK);
			vppu.wire.CLK = CLK;

			vppu.vid_out->sim(batch[n]);

			CLK = NOT(CLK);
		}

		// Process the batch

		const float pi = 3.14159265358979323846f;
		float normalize_factor = 1.2f / features.WhiteLevel;	// 120 IRE  -- correction for measurements by HardWareMan
		float Y = 0, I = 0, Q = 0;
		size_t cb_phase = 9;

		for (size_t n = 0; n < num_phases; n++)
		{
			float level = ((batch[n].composite - features.BlackLevel) * normalize_factor) / num_phases;
			Y += level;
			I += level * cos((cb_phase + n) * (2 * pi / num_phases));
			Q += level * sin((cb_phase + n) * (2 * pi / num_phases)) * +1.0f;
		}

		// Note to PAL researchers. Read math behind PAL, or just use these:
		//		cb_phase = 1
		//		Q += level * sin((cb_phase + n) * (2 * π / num_phases)) * -1.0f;    <----   Minus

		delete[] batch;

		// 6500K color temperature

		float R = Y + (0.956f * I) + (0.623f * Q);
		float G = Y - (0.272f * I) - (0.648f * Q);
		float B = Y - (1.105f * I) + (1.705f * Q);

		rgbOut.RGB.RED = (uint8_t)(Clamp(R, 0.f, 1.f) * 255);
		rgbOut.RGB.GREEN = (uint8_t)(Clamp(G, 0.f, 1.f) * 255);
		rgbOut.RGB.BLUE = (uint8_t)(Clamp(B, 0.f, 1.f) * 255);
	}

	float VideoOut::Clamp(float val, float min, float max)
	{
		return std::min(max, std::max(val, min));
	}

	void VideoOut::SetupChromaDecoderPAL()
	{
		chroma_decoder = new PLA(chroma_decoder_inputs, chroma_decoder_outputs, (char *)"PALChromaDecoder.bin");

		// Set matrix

		static size_t bitmask[] = {
			0b1001100110,
			0b0110100101,
			0b1010100101,
			0b0101100110,
			0b1001010110,
			0b0110011001,
			0b1010011010,
			0b0101011010,
			0b1001101001,
			0b0110101001,
			0b0010101010,
			0b1010100110,
			0b0101101010,
			0b1001011001,
			0b0110010110,
			0b1010010110,
			0b0101011001,
			0b1001101010,
			0b0110100110,
			0b1010101001,
			0b0101101001,
			0b1001011010,
			0b0110011010,
			0b1010011001,
			0b0101010110,
		};

		chroma_decoder->SetMatrix(bitmask);
		for (unsigned key=0;key<32;key++)
		{
			PALChromaInputs in{};
			in.V0=key&1; in.n_V0=in.V0^1;
			in.CC0=(key>>1)&1; in.n_CC0=in.CC0^1;
			in.CC1=(key>>2)&1; in.n_CC1=in.CC1^1;
			in.CC2=(key>>3)&1; in.n_CC2=in.CC2^1;
			in.CC3=(key>>4)&1; in.n_CC3=in.CC3^1;
			TriState* out; chroma_decoder->sim(in.packed_bits,&out);
			for(unsigned n=0;n<25;n++) if(out[n]==TriState::One) pal_colors[key] |= 1U<<n;
		}
	}

#pragma region "RGB PPU Stuff"

	void VideoOut::SetupColorMatrix()
	{
		char colorMatrixName[0x20] = { 0 };

		// Generate PLA dump name according to the PPU revision.

		sprintf(colorMatrixName, "ColorMatrix_%s.bin", ppu->RevisionToStr(ppu->rev));

		color_matrix = new PLA(color_matrix_inputs, color_matrix_outputs, colorMatrixName);

		// Set matrix

		size_t* bitmask = nullptr;

		switch (ppu->rev)
		{
			case Revision::RP2C04_0003:
			{
				static size_t RP2C04_0003_ColorMatrix[] = {		// Undefined behavior in GCC if you do not specify static
					0b1010100001110110,
					0b1111000101010100,
					0b0000110101000000,
					0b0101111001100110,
					0b1000000001111011,
					0b0110110110110100,
					0b0001100101010011,
					0b1111101000110111,
					0b1010000001111000,
					0b1111110100111100,
					0b0110100101101001,
					0b1111100100010110,

					0b1010110001010110,
					0b0101010111100101,
					0b0000100101110000,
					0b1110101111001101,
					0b0001110011100001,
					0b0101111010010100,
					0b0100111111011000,
					0b1000001010111111,
					0b1010010001011110,
					0b0101110101011100,
					0b1010101111010110,
					0b1010010010111011,

					0b1110100100100110,
					0b0111011110100100,
					0b1011101101111001,
					0b0110011011100010,
					0b0000100110000111,
					0b0111101110110100,
					0b1010101001100100,
					0b0110011010101110,
					0b1010100101000110,
					0b1111101111111100,
					0b1011101001100100,
					0b0010110010101110,
				};

				bitmask = RP2C04_0003_ColorMatrix;
			}
			break;
		}

		if (bitmask != nullptr)
		{
			color_matrix->SetMatrix(bitmask);
		}
	}

	template<int Phase> void VideoOut::sim_ColorMatrix(const VideoInputsT<Phase>& in)
	{
		TriState PCLK = in.PCLK;
		TriState col[4]{};
		size_t packed = 0;

		if (PCLK == TriState::One)
		{
			TriState unpacked[16]{};
			col[0] = static_cast<TriState>(((cc_output >> 2) & 1) ^ 1);
			col[1] = static_cast<TriState>(((cc_output >> 3) & 1) ^ 1);
			col[2] = in.n_LL0;
			col[3] = in.n_LL1;

			DMX4(col, unpacked);
			packed = Pack(unpacked) | ((size_t)Pack(&unpacked[8]) << 8);
		}

		color_matrix->sim(packed, &rgb_output);
	}

	template<int Phase> void VideoOut::sim_Select12To3(const VideoInputsT<Phase>& in)
	{
		TriState n_PCLK = in.n_PCLK;
		TriState PCLK = in.PCLK;
		TriState n_TR = ppu->wire.n_TR;
		TriState n_TG = ppu->wire.n_TG;
		TriState n_TB = ppu->wire.n_TB;
		TriState lum_in[2]{};

		lum_in[0] = NOR(n_PCLK, static_cast<TriState>(cc_output & 1));
		lum_in[1] = NOR(n_PCLK, static_cast<TriState>((cc_output >> 1) & 1));

		red_sel.sim(PCLK, n_TR, &rgb_output[12 * 0], lum_in);
		green_sel.sim(PCLK, n_TG, &rgb_output[12 * 1], lum_in);
		blue_sel.sim(PCLK, n_TB, &rgb_output[12 * 2], lum_in);
	}

	void VideoOut::sim_RGB_DAC(VideoOutSignal& vout)
	{
		// By multiplying by 35 we are just a little bit short of 255 for a component of 7, but that will do.
		vout.RGB.RED = static_cast<uint8_t>(rgb_red * 35);
		vout.RGB.GREEN = static_cast<uint8_t>(rgb_green * 35);
		vout.RGB.BLUE = static_cast<uint8_t>(rgb_blue * 35);

		vout.RGB.nSYNC = rgb_sync_latch[2].nget();
	}

	void RGB_SEL12x3::sim(TriState PCLK, TriState n_Tx, TriState col_in[12], TriState lum_in[2])
	{
		TriState mux_in[4]{};
		TriState mux_out{};

		mux_in[0] = col_in[8];
		mux_in[1] = col_in[10];
		mux_in[2] = col_in[9];
		mux_in[3] = col_in[11];
		mux_out = MUX2(lum_in, mux_in);
		ff[0].set(NAND(NOT(MUX(PCLK, ff[0].get(), mux_out)), n_Tx));

		mux_in[0] = col_in[4];
		mux_in[1] = col_in[6];
		mux_in[2] = col_in[5];
		mux_in[3] = col_in[7];
		mux_out = MUX2(lum_in, mux_in);
		ff[1].set(NAND(NOT(MUX(PCLK, ff[1].get(), mux_out)), n_Tx));

		mux_in[0] = col_in[0];
		mux_in[1] = col_in[2];
		mux_in[2] = col_in[1];
		mux_in[3] = col_in[3];
		mux_out = MUX2(lum_in, mux_in);
		ff[2].set(NAND(NOT(MUX(PCLK, ff[2].get(), mux_out)), n_Tx));
	}

	void RGB_SEL12x3::getOut(TriState col_out[3])
	{
		for (size_t n = 0; n < 3; n++)
		{
			col_out[n] = ff[n].get();
		}
	}

	void VideoOut::ConvertRAWToRGB_Component(VideoOutSignal& rawIn, VideoOutSignal& rgbOut)
	{
		// Using a temporary PPU of the same revision, to simulate the video generator

		PPU vppu(ppu->rev, true);

		// Latch all important signals

		vppu.wire.n_CC[0] = NOT(FromByte(rawIn.RAW.CC0));
		vppu.wire.n_CC[1] = NOT(FromByte(rawIn.RAW.CC1));
		vppu.wire.n_CC[2] = NOT(FromByte(rawIn.RAW.CC2));
		vppu.wire.n_CC[3] = NOT(FromByte(rawIn.RAW.CC3));
		vppu.wire.n_LL[0] = NOT(FromByte(rawIn.RAW.LL0));
		vppu.wire.n_LL[1] = NOT(FromByte(rawIn.RAW.LL1));
		vppu.wire.n_TR = NOT(FromByte(rawIn.RAW.TR));
		vppu.wire.n_TG = NOT(FromByte(rawIn.RAW.TG));
		vppu.wire.n_TB = NOT(FromByte(rawIn.RAW.TB));

		vppu.fsm.SYNC = FromByte(rawIn.RAW.Sync);
		vppu.fsm.n_PICTURE = TriState::Zero;

		size_t numHalfs = 6;

		vppu.wire.n_PCLK = TriState::One;
		vppu.wire.PCLK = TriState::Zero;

		for (size_t n = 0; n < numHalfs; n++)
		{
			vppu.vid_out->sim(rgbOut);
			vppu.wire.n_PCLK = NOT(vppu.wire.n_PCLK);
			vppu.wire.PCLK = NOT(vppu.wire.PCLK);
		}
	}

#pragma endregion "RGB PPU Stuff"

	bool VideoOut::IsComposite()
	{
		return composite;
	}

	float VideoOut::GetNoise()
	{
		float a = -noise;
		float b = noise;
		float r = a + (float)rand() / ((float)RAND_MAX / (b - a));
		return r;
	}

	void VideoOut::SetCompositeNoise(float volts)
	{
		if (volts != 0.0f)
		{
			srand((unsigned)time(0));
			noise = volts;
			noise_enable = true;
		}
		else
		{
			noise_enable = false;
		}
	}

	void VideoOut::ConvertRAWToRGB(VideoOutSignal& rawIn, VideoOutSignal& rgbOut)
	{
		if (composite)
		{
			ConvertRAWToRGB_Composite(rawIn, rgbOut);
		}
		else
		{
			ConvertRAWToRGB_Component(rawIn, rgbOut);
		}
	}

	void VideoOut::UseExternalDacLevels(bool use, DacLevels& tab)
	{
		use_ext_levels = use;
		ext_levels = tab;
		RebuildDacSamples();
	}

	// The two edge polarities of the video generator (called from PPU::sim_Edge).
	template void VideoOut::sim_Edge<0>(const EdgeContextT<0>&, VideoOutSignal&);
	template void VideoOut::sim_Edge<1>(const EdgeContextT<1>&, VideoOutSignal&);
}
