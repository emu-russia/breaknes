// Main module and auxiliary logic.

#include "pch.h"
#include <array>
#include <cstring>
#ifdef FASTSIM_SKIP_ORACLE
#include <cstdlib>
#endif

using namespace BaseLogic;

namespace PPUSimFast
{
	// EXT_In as four TriState bytes per EXT nibble: NOR(NOT(EXTn), /SLAVE) is EXTn in slave mode;
	// entry 0 doubles as the all-Zero word for master mode and the RGB revisions.
	static constexpr std::array<uint32_t, 16> ExtInWords = [] {
		std::array<uint32_t, 16> words{};
		for (unsigned e = 0; e < 16; e++)
			words[e] = (e & 1) | (((e >> 1) & 1) << 8) | (((e >> 2) & 1) << 16) | (((e >> 3) & 1) << 24);
		return words;
	}();
	static_assert(static_cast<unsigned>(TriState::One) == 1 && static_cast<unsigned>(TriState::Zero) == 0, "EXT_In word packing");

	static const Log::LogCategoryDesc PpuLogCategories[] =
	{
		{ Cat_Regs, "Regs" },		// CPU I/F register accesses ($2000-$2007)
		{ Cat_Events, "Events" },	// reset / VBlank NMI events
	};

	const Log::LogCategoryDesc* GetLogCategories(size_t& count)
	{
		count = sizeof(PpuLogCategories) / sizeof(PpuLogCategories[0]);
		return PpuLogCategories;
	}

	void PPU::SetLogMask(uint64_t mask)
	{
		Log::SetCategoryMask(Log::Source_PPU, mask);
	}

	/// <summary>
	/// Constructor. Creates all PPU modules.
	/// </summary>
	/// <param name="_rev">Revision of the PPU chip.</param>
	/// <param name="VideoGen">true: Create a special version of the PPU that contains only a video generator.</param>
	PPU::PPU(Revision _rev, bool VideoGen)
	{
		rev = _rev;
		is_pal = rev == Revision::RP2C07_0 || rev == Revision::UMC_UA6538;
		divider_pal = is_pal ? 256 : 0;

		if (!VideoGen)
		{
			regs = new ControlRegs(this);
			h = new HVCounter(this, 9);
			v = new HVCounter(this, 9);
			hv_dec = new HVDecoder(this);
			hv_fsm = new FSM(this);
			cram = new CRAM(this);
			mux = new Mux(this);
			eval = new ObjEval(this);
			oam = new OAM(this);
			fifo = new FIFO(this);
			data_reader = new DataReader(this);
			vram_ctrl = new VRAM_Control(this);
		}

		vid_out = new VideoOut(this);
	}

	PPU::~PPU()
	{
		if (regs)
			delete regs;
		if (h)
			delete h;
		if (v)
			delete v;
		if (hv_dec)
			delete hv_dec;
		if (hv_fsm)
			delete hv_fsm;
		if (cram)
			delete cram;
		if (vid_out)
			delete vid_out;
		if (mux)
			delete mux;
		if (eval)
			delete eval;
		if (oam)
			delete oam;
		if (fifo)
			delete fifo;
		if (data_reader)
			delete data_reader;
		if (vram_ctrl)
			delete vram_ctrl;
	}

	void PPU::sim(TriState inputs[], TriState outputs[], uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus, VideoOutSignal& vout)
	{
		// v4 idle-call class: nothing but the divider and the video phase ring can change. The idle
		// call checks the live conditions itself and commits only when they hold; see sim_IdleCall
		// for the exact argument. FASTSIM_SKIP_ORACLE builds verify it.
		if (idle_armed && sim_IdleCall(inputs, outputs, ext, data_bus, ad_bus, addrHi_bus, vout)) return;
		sim_Full(inputs, outputs, ext, data_bus, ad_bus, addrHi_bus, vout);
	}

	// FASTSIM_ABLATE_MASK (timing-only diagnostic, default 0): bit n skips module group n of
	// the pixel body to attribute its cost. Outputs are wrong; never an acceptance build.
#ifndef FASTSIM_ABLATE_MASK
#define FASTSIM_ABLATE_MASK 0
#endif

	// The pixel-edge body. Phase is the PCLK polarity of this edge (sim_PCLK always produces a
	// binary PCLK), so in each instantiation every latch enable, mux select and phase test on
	// PCLK/n_PCLK is a compile-time constant; the module order and every expression are those of
	// the reference body.
	template<int Phase> void PPU::sim_Edge(VideoOutSignal& vout)
	{
		// The edge context: same-call inter-module values travel in registers (see EdgeContext).
		// Producers also write the wire/fsm fields; the oracle build checks the two agree.
		EdgeContextT<Phase> ctx{};
		ctx.RESCL = fsm.RESCL;

		// H/V Control logic

		const uint32_t HPLA = hv_dec->sim_HDecoderPacked(hv_fsm->get_VB(), hv_fsm->get_BLNK(wire.BLACK));
		const uint16_t VPLA = hv_dec->sim_VDecoderPacked();

		if (!(FASTSIM_ABLATE_MASK & 1)) hv_fsm->sim(HPLA, VPLA, ctx);

		if (!(FASTSIM_ABLATE_MASK & 2))
		{
			h->sim(TriState::One, ctx.HC);
			TriState V_IN = static_cast<TriState>((HPLA >> 23) & 1);
			v->sim(V_IN, ctx.VC);
		}

		// The other parts

		if (!(FASTSIM_ABLATE_MASK & 4)) fifo->sim_SpriteH(ctx);

		if (!(FASTSIM_ABLATE_MASK & 8)) regs->sim_CLP(ctx);

		if (!(FASTSIM_ABLATE_MASK & 16)) vram_ctrl->sim(ctx);

		if (!(FASTSIM_ABLATE_MASK & 32)) oam->sim_OFETCH(ctx);

		if (!(FASTSIM_ABLATE_MASK & 64)) eval->sim(ctx);

		if (!(FASTSIM_ABLATE_MASK & 128)) oam->sim(ctx);

		if (!(FASTSIM_ABLATE_MASK & 256)) data_reader->sim(ctx);

		if (!(FASTSIM_ABLATE_MASK & 512)) fifo->sim(ctx);

		if (!(FASTSIM_ABLATE_MASK & 1024)) vram_ctrl->sim_TH_MUX(ctx);

		if (!(FASTSIM_ABLATE_MASK & 2048)) vram_ctrl->sim_ReadBuffer(ctx);

		if (!(FASTSIM_ABLATE_MASK & 4096)) mux->sim(ctx);

		if (!(FASTSIM_ABLATE_MASK & 8192)) cram->sim(ctx);

#ifdef FASTSIM_SKIP_ORACLE
		// The context must carry exactly what the wires carry: every consumer that read the
		// context would have read the same value from the stored field.
		{
			const EdgeContext stored = CaptureEdge(*this);
			static_assert(alignof(EdgeValues) == 1, "EdgeValues is a plain byte struct without padding (memcmp below)");
			const EdgeValues& stored_values = stored;
			const EdgeValues& context_values = ctx;
			if (std::memcmp(&stored_values, &context_values, sizeof(EdgeValues)) != 0 || stored.PCLK != ctx.PCLK || stored.n_PCLK != ctx.n_PCLK)
			{
				const unsigned char* a = reinterpret_cast<const unsigned char*>(&stored_values);
				const unsigned char* b = reinterpret_cast<const unsigned char*>(&context_values);
				for (size_t n = 0; n < sizeof(EdgeValues); n++) if (a[n] != b[n]) std::fprintf(stderr, "  edge context byte %zu: wires %02x context %02x\n", n, a[n], b[n]);
				std::fprintf(stderr, "  phase: wires %d/%d context %d/%d\n", int(stored.PCLK), int(stored.n_PCLK), int(ctx.PCLK), int(ctx.n_PCLK));
				std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: PPU edge context disagrees with the stored wires\n");
				std::abort();
			}
		}
#endif

		Prev_PCLK = wire.PCLK;

		// The video generator with the phase constant and the sampled commands/luma from the
		// context (the reference evaluates it right after the body, before the pads).
		vid_out->sim_Edge(ctx, vout);
	}

	void PPU::sim_Full(TriState inputs[], TriState outputs[], uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus, VideoOutSignal& vout)
	{
		// Input terminals and binding

		wire.RnW = inputs[(size_t)InputPad::RnW];
		wire.RS[0] = inputs[(size_t)InputPad::RS0];
		wire.RS[1] = inputs[(size_t)InputPad::RS1];
		wire.RS[2] = inputs[(size_t)InputPad::RS2];
		wire.n_DBE = inputs[(size_t)InputPad::n_DBE];

		regs->sim_RWDecoder();

		wire.RES = NOT(inputs[(size_t)InputPad::n_RES]);

		if (wire.RES != prev_RES)
		{
			LOG_PPU(Cat_Events, "RESET %s", wire.RES == TriState::One ? "asserted" : "deasserted");
			prev_RES = wire.RES;
		}

		if (wire.RES == TriState::One)
		{
			ResetPCLKCounter();
		}

		wire.CLK = inputs[(size_t)InputPad::CLK];
		wire.n_CLK = NOT(wire.CLK);

		sim_PCLK();

		hv_fsm->sim_RESCL_early();

		Reset_FF.set(NOR(fsm.RESCL, NOR(wire.RES, Reset_FF.get())));
		wire.RC = NOT(Reset_FF.nget());

		sim_BusInput(ext, data_bus, ad_bus);

		// Regs

		regs->sim();

		pixel_invoked = Prev_PCLK != wire.PCLK;
#ifdef FASTSIM_CALL_CLASSES
		FastSimStats::ppu_classes[(pixel_invoked ? 1 : 0) | (stats_prev_edge ? 2 : 0) |
			(wire.n_DBE == TriState::Zero ? 4 : 0) | (regs->IsPortPending() ? 8 : 0) |
			(wire.RES == TriState::One ? 16 : 0)]++;
		stats_prev_edge = pixel_invoked;
#endif
#ifdef FASTSIM_ABLATE_PPU_BODY
		// Timing-only diagnostic: skip the pixel body (outputs are wrong; never an acceptance build).
		if (false)
#else
		if (pixel_invoked)
#endif
		{
			if (wire.PCLK == TriState::One) sim_Edge<1>(vout); else sim_Edge<0>(vout);
		}
		else vid_out->sim(vout);

		// Output terminals

		outputs[(size_t)OutputPad::n_INT] = fsm.INT ? TriState::Zero : TriState::Z;
		outputs[(size_t)OutputPad::ALE] = NOT(wire.n_ALE);
		outputs[(size_t)OutputPad::n_RD] = NOT(wire.RD);
		outputs[(size_t)OutputPad::n_WR] = NOT(wire.WR);

		sim_BusOutput(ext, data_bus, ad_bus, addrHi_bus);

		// Arm the idle path for the next call, after edge calls too: every stage outside the pixel
		// body is at its fixed point for the current PCLK phase once this call evaluated it, so the
		// former settling "tail" call is an idle call (see sim_IdleCall). A reset call never arms
		// (RES high), and a call that left RC high or the port pending takes the full path next.
		idle_armed = wire.RES == TriState::Zero && wire.RC == TriState::Zero &&
			!regs->IsPortPending() && vid_out->IdleEligible();
		if (idle_armed) ArmIdle(inputs, outputs, vout);
	}

	void PPU::ArmIdle(const TriState inputs[], const TriState outputs[], const VideoOutSignal& vout)
	{
		// Everything the idle call publishes besides the video sample is a settled wire here: the
		// pads, the EXT/AD/PA drive values (pixel-body products and the EXT output latch, which the
		// n_PCLK=1 update re-captures unchanged) and the register decode made from these inputs.
		std::memcpy(&idle.outputs, outputs, sizeof(idle.outputs));
		std::memcpy(&idle.decode_raw, inputs, sizeof(idle.decode_raw));
		const bool has_ext = rev != Revision::RP2C04_0003;
		idle.ext_mask = has_ext && regs->get_nSLAVE() == TriState::Zero ? 15 : 0;
		idle.drive_ext = has_ext && wire.n_SLAVE == TriState::One;
		idle.ext_out = static_cast<uint8_t>(~extout_latch) & 15;
		const uint16_t address = static_cast<uint16_t>(~data_reader->pamux->get_n_address());
		idle.drive_ad = wire.RD == TriState::Zero;
		idle.ad = static_cast<uint8_t>(address);
		idle.addr_hi = static_cast<uint8_t>((address >> 8) & 0x3f);
		vid_out->ArmIdle(vout);
	}

	static_assert(size_t(InputPad::RnW) == 0 && size_t(InputPad::RS0) == 1 && size_t(InputPad::RS1) == 2 &&
		size_t(InputPad::RS2) == 3 && size_t(InputPad::n_DBE) == 4, "the port pads are the first five input pads");

	bool PPU::sim_IdleCall(const TriState inputs[], TriState outputs[], uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus, VideoOutSignal& vout)
	{
		// Exactness argument. In the idle class (no PCLK edge, CPU port released and owing no
		// release call, RES and RC low, no public mutation since the arming call) the reference
		// evaluation changes only: the PCLK divider, the video phase ring and its sample, the
		// per-call sampled wires (RnW/RS/n_DBE, n_RD/n_WR, EXT_In, PD, the RS decoder outputs) and
		// the output objects. Every other stage is at a fixed point for the current PCLK phase after
		// the arming call evaluated it, including the first call after an edge: within a phase each
		// transparent latch recaptures a value that only the pixel body or a register access can
		// change (early RESCL: vclr_latch2 on PCLK=1 from vclr_latch1, which the body writes only on
		// n_PCLK=1; the reset FF from RESCL/RES; the PAL BLACK chain, whose stages capture their
		// predecessor on alternating phases; the video input/output latches, the PAL /PICTURE and V0
		// latches and the RGB sync/selector chain from pixel-body products), and the register FFs hold
		// while the port is released. So the pads, the EXT/AD/PA drive values and the decode captured
		// at arming time are what the reference recomputes. prev_n_RD/prev_n_WR are One and prev_RES
		// is Zero throughout (the arming call had the port released and RES low), so the log edge
		// detectors are inert. FASTSIM_SKIP_ORACLE builds re-run every skipped or replaced stage from
		// the same pre-state and abort on a difference.
		//
		// Live conditions, peeked without committing: port released, RES low (NOT(n_RES) is Zero
		// for One, X and Z), a binary master clock whose divider step leaves PCLK unchanged
		// (Prev_PCLK is X after ResetPCLKCounter, so that fails here too). An X/Z clock, an edge,
		// a port access or a reset take the full path from scratch.
		const TriState clk = inputs[(size_t)InputPad::CLK];
		if (inputs[(size_t)InputPad::n_DBE] == TriState::Zero || inputs[(size_t)InputPad::n_RES] == TriState::Zero ||
			(clk != TriState::Zero && clk != TriState::One))
			return false;
		const PixelClockStep next = DividerTable()[pclk_state | (clk == TriState::One ? 64 : 0) | divider_pal];
		if (next.clock != Prev_PCLK) return false;
#ifdef FASTSIM_CALL_CLASSES
		FastSimStats::ppu_classes[0]++;
		stats_prev_edge = false;
#endif
#ifdef FASTSIM_SKIP_ORACLE
		const uint8_t ext_before = *ext, ad_before = *ad_bus;
		oracle_pclk_before = pclk_state;
		std::memcpy(oracle_video_before, static_cast<const void*>(vid_out), sizeof(VideoOut));
#endif
		// Sampled wires: the five port pads land on the five port wires (same order), n_RD/n_WR are
		// One with the port released (sim_RWDecoder), RES = NOT(n_RES), the master clock pair. The
		// divider steps without a PCLK change: PCLK/n_PCLK hold and there is no negedge to count.
		std::memcpy(&wire.RnW, inputs, 5);
		wire.n_RD = TriState::One;
		wire.n_WR = TriState::One;
		wire.RES = NOT(inputs[(size_t)InputPad::n_RES]);
		wire.CLK = clk;
		wire.n_CLK = NOT(clk);
		pclk_state = next.state;
		const uint32_t ext_in = ExtInWords[*ext & idle.ext_mask];
		std::memcpy(wire.EXT_In, &ext_in, sizeof(ext_in));
		PD = *ad_bus;
		// The RS/RnW decoders are the only register stage that reads the pads in this class, and
		// identical pad bytes give an identical pad decode; the $2005/$2006 first/second-write
		// decode also follows SCCX_FF1 one call late and is re-evaluated every call.
		uint32_t decode_raw;
		std::memcpy(&decode_raw, inputs, sizeof(decode_raw));
		regs->sim_IdleDecode(decode_raw != idle.decode_raw);
		idle.decode_raw = decode_raw;
		vid_out->sim_Idle(vout);
		std::memcpy(outputs, &idle.outputs, sizeof(idle.outputs));
		if (idle.drive_ext) *ext = idle.ext_out;
		if (idle.drive_ad) *ad_bus = idle.ad;
		*addrHi_bus = idle.addr_hi;
#ifdef FASTSIM_SKIP_ORACLE
		sim_IdleOracle(inputs, outputs, ext_before, *ext, *data_bus, ad_before, *ad_bus, *addrHi_bus, vout);
#endif
		return true;
	}

#ifdef FASTSIM_SKIP_ORACLE
	// Re-run the stages the idle path skipped or replaced, from the same pre-state, and abort if
	// any of them would have changed state or output. They are idempotent in the idle class by
	// construction, so running them afterwards is harmless when the class predicate is right and
	// fatal when it is wrong.
	void PPU::sim_IdleOracle(const TriState inputs[], const TriState outputs[], uint8_t ext_before, uint8_t ext_after, uint8_t data_after, uint8_t ad_before, uint8_t ad_after, uint8_t addr_hi_after, VideoOutSignal& vout)
	{
		const TriState rescl = fsm.RESCL, rc = wire.RC, black = wire.BLACK, pclk = wire.PCLK;
		const uint8_t db = DB;
		const size_t counter = pclk_counter;
		// Class invariants the lean path relies on, and the prefix it replaced: the sampled port
		// wires, the RW decoder, RES, the clock pair and the divider step (re-run from the state
		// before the idle call stepped it, it must land on the same bank state and PCLK).
		unsigned failed = 0;
		bool same = prev_n_RD == TriState::One && prev_n_WR == TriState::One && prev_RES == TriState::Zero &&
			Prev_PCLK == wire.PCLK && wire.RES == NOT(inputs[(size_t)InputPad::n_RES]) &&
			wire.CLK == inputs[(size_t)InputPad::CLK] && wire.n_CLK == NOT(wire.CLK) &&
			std::memcmp(&wire.RnW, inputs, 5) == 0;
		{
			const TriState n_rd = wire.n_RD, n_wr = wire.n_WR;
			regs->sim_RWDecoder();
			same = same && n_rd == wire.n_RD && n_wr == wire.n_WR;
			const uint8_t stepped = pclk_state;
			pclk_state = oracle_pclk_before;
			sim_PCLK();
			same = same && pclk_state == stepped && wire.PCLK == pclk && counter == pclk_counter;
		}
		if (!same) failed |= 1;
		// Bus input from the pre-call bus values with the reference formulas.
		TriState ext_in[4];
		std::memcpy(ext_in, wire.EXT_In, sizeof(ext_in));
		uint8_t ext_copy = ext_before, data_copy = data_after, ad_copy = ad_before;
		sim_BusInput(&ext_copy, &data_copy, &ad_copy);
		if (!(std::memcmp(ext_in, wire.EXT_In, sizeof(ext_in)) == 0 && PD == ad_before && DB == db)) failed |= 2;
		// Early RESCL, the reset FF, the whole register block (decode included) must be at a fixed point.
		unsigned char regs_before[sizeof(ControlRegs)], fsm_before[sizeof(FSM)];
		std::memcpy(regs_before, static_cast<const void*>(regs), sizeof(ControlRegs));
		std::memcpy(fsm_before, static_cast<const void*>(hv_fsm), sizeof(FSM));
		const InternalWires wires_before = wire;
		const FsmCommands commands_before = fsm;
		hv_fsm->sim_RESCL_early();
		Reset_FF.set(NOR(fsm.RESCL, NOR(wire.RES, Reset_FF.get())));
		const TriState rc_after = NOT(Reset_FF.nget());
		regs->sim();
		if (std::memcmp(regs_before, static_cast<const void*>(regs), sizeof(ControlRegs)) != 0)
		{
			failed |= 4;
			const unsigned char* now = reinterpret_cast<const unsigned char*>(regs);
			for (size_t n = 0; n < sizeof(ControlRegs); n++) if (regs_before[n] != now[n]) std::fprintf(stderr, "  regs byte %zu: %02x -> %02x\n", n, regs_before[n], now[n]);
		}
		if (std::memcmp(fsm_before, static_cast<const void*>(hv_fsm), sizeof(FSM)) != 0) failed |= 8;
		if (std::memcmp(&wires_before, &wire, sizeof(wire)) != 0)
		{
			failed |= 16;
			const unsigned char* was = reinterpret_cast<const unsigned char*>(&wires_before);
			const unsigned char* now = reinterpret_cast<const unsigned char*>(&wire);
			for (size_t n = 0; n < sizeof(wire); n++) if (was[n] != now[n]) std::fprintf(stderr, "  wire byte %zu: %02x -> %02x (n_DBE %d RnW %d RS %d%d%d)\n", n, was[n], now[n], int(wire.n_DBE), int(wire.RnW), int(wire.RS[0]), int(wire.RS[1]), int(wire.RS[2]));
		}
		if (std::memcmp(&commands_before, &fsm, sizeof(fsm)) != 0) failed |= 32;
		// Pads and bus drive with the reference formulas.
		const TriState pads[4]{ fsm.INT ? TriState::Zero : TriState::Z, NOT(wire.n_ALE), NOT(wire.RD), NOT(wire.WR) };
		if (std::memcmp(pads, outputs, sizeof(pads)) != 0) failed |= 64;
		uint8_t ext_ref = ext_before, data_ref = data_after, ad_ref = ad_before, hi_ref = 0;
		const uint8_t extout_before = extout_latch;
		sim_BusOutput(&ext_ref, &data_ref, &ad_ref, &hi_ref);
		if (!(ext_ref == ext_after && data_ref == data_after && ad_ref == ad_after && hi_ref == addr_hi_after &&
			extout_latch == extout_before)) failed |= 128;
		// The idle path already advanced the phase ring: run the full generator from the same
		// pre-state image and compare both the sample and the resulting generator image.
		VideoOutSignal check{};
		alignas(VideoOut) unsigned char after_idle[sizeof(VideoOut)];
		std::memcpy(after_idle, static_cast<const void*>(vid_out), sizeof(VideoOut));
		std::memcpy(static_cast<void*>(vid_out), oracle_video_before, sizeof(VideoOut));
		pixel_invoked = false;
		vid_out->sim(check);
		// Composite noise is outside the class (IdleEligible), so the samples must be bit-identical.
		const bool video_same = std::memcmp(&check, &vout, sizeof(check)) == 0 &&
			std::memcmp(after_idle, static_cast<const void*>(vid_out), sizeof(VideoOut)) == 0;
		if (!video_same) failed |= 256;
		if (!(rescl == fsm.RESCL && rc == rc_after && black == wire.BLACK && pclk == wire.PCLK &&
			db == DB && counter == pclk_counter && Prev_PCLK == wire.PCLK)) failed |= 512;
		if (failed != 0)
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: PPU idle class violated (components 0x%x: 1 invariants 2 bus-in 4 regs 8 fsm 16 wires 32 commands 64 pads 128 bus-out 256 video 512 misc; RESCL %d->%d RC %d->%d BLACK %d->%d video %u/%u ext %02x/%02x ad %02x/%02x hi %02x/%02x)\n",
				failed, int(rescl), int(fsm.RESCL), int(rc), int(rc_after), int(black), int(wire.BLACK),
				*reinterpret_cast<const uint32_t*>(&vout), *reinterpret_cast<const uint32_t*>(&check),
				ext_after, ext_ref, ad_after, ad_ref, addr_hi_after, hi_ref);
			std::abort();
		}
	}
#endif

	void PPU::sim_BusInput(uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus)
	{
		switch (rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
			{
				TriState nSLAVE = regs->get_nSLAVE();

				for (size_t n = 0; n < 4; n++)
				{
					TriState extIn = (((*ext) >> n) & 1) ? TriState::One : TriState::Zero;
					wire.EXT_In[n] = NOR(NOT(extIn), nSLAVE);
				}
			}
			break;

			// TBD: The other RGB PPU's appear to be similar, but there are no pictures yet, we do not speculate.

			case Revision::RP2C04_0003:
			{
				for (size_t n = 0; n < 4; n++)
				{
					wire.EXT_In[n] = TriState::Zero;
				}
			}
			break;
		}

		if (wire.n_WR == TriState::Zero)
		{
			DB = *data_bus;

			// CPU I/F register write: one line per write access ($2000-$2007).
			if (prev_n_WR != TriState::Zero)
			{
				size_t reg = (size_t)wire.RS[0] | ((size_t)wire.RS[1] << 1) | ((size_t)wire.RS[2] << 2);
				LOG_PPU(Cat_Regs, "Write $200%X = %02X", (int)reg, (int)DB);
			}
		}
		prev_n_WR = wire.n_WR;

		PD = *ad_bus;
	}

	void PPU::sim_BusOutput(uint8_t* ext, uint8_t* data_bus, uint8_t* ad_bus, uint8_t* addrHi_bus)
	{
		TriState n_PCLK = wire.n_PCLK;

		switch (rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
			{
				if (n_PCLK == TriState::One) extout_latch = static_cast<uint8_t>(
					wire.n_EXT_Out[0] | (wire.n_EXT_Out[1] << 1) | (wire.n_EXT_Out[2] << 2) | (wire.n_EXT_Out[3] << 3));

				if (wire.n_SLAVE == TriState::One)
				{
					*ext = static_cast<uint8_t>(~extout_latch) & 15;
				}
			}
			break;

			case Revision::RP2C04_0003:
			{
				// There are no EXT terminals in the RGB PPUs.
			}
			break;
		}

		if (wire.n_RD == TriState::Zero)
		{
			*data_bus = DB;

			// CPU I/F register read: one line per read access ($2000-$2007).
			if (prev_n_RD != TriState::Zero)
			{
				size_t reg = (size_t)wire.RS[0] | ((size_t)wire.RS[1] << 1) | ((size_t)wire.RS[2] << 2);
				LOG_PPU(Cat_Regs, "Read $200%X = %02X", (int)reg, (int)DB);
			}
		}
		prev_n_RD = wire.n_RD;

		if (wire.RD == TriState::Zero)
		{
			*ad_bus = static_cast<uint8_t>(~data_reader->pamux->get_n_address());
		}

		*addrHi_bus = static_cast<uint8_t>((~data_reader->pamux->get_n_address() >> 8) & 0x3f);
	}

	size_t PPU::GetPCLKCounter()
	{
		return pclk_counter;
	}

	void PPU::ResetPCLKCounter()
	{
		pclk_counter = 0;
		Prev_PCLK = TriState::X;
		Disarm();
	}

	const char* PPU::RevisionToStr(Revision rev)
	{
		switch (rev)
		{
			case Revision::RP2C02G: return "RP2C02G";
			case Revision::RP2C02H: return "RP2C02H";

			case Revision::RP2C03B: return "RP2C03B";
			case Revision::RP2C03C: return "RP2C03C";
			case Revision::RC2C03B: return "RC2C03B";
			case Revision::RC2C03C: return "RC2C03C";
			case Revision::RP2C04_0001: return "RP2C04-0001";
			case Revision::RP2C04_0002: return "RP2C04-0002";
			case Revision::RP2C04_0003: return "RP2C04-0003";
			case Revision::RP2C04_0004: return "RP2C04-0004";
			case Revision::RC2C05_01: return "RC2C05-01";
			case Revision::RC2C05_02: return "RC2C05-02";
			case Revision::RC2C05_03: return "RC2C05-03";
			case Revision::RC2C05_04: return "RC2C05-04";
			case Revision::RC2C05_99: return "RC2C05-99";

			case Revision::RP2C07_0: return "RP2C07-0";

			case Revision::UMC_UA6538: return "UMC UA6538";

			default:
				break;
		}
		return "Unknown";
	}

	TriState PPU::GetDBBit(size_t n)
	{
		TriState DBBit = (DB & (1 << n)) != 0 ? TriState::One : TriState::Zero;
		return DBBit;
	}

	void PPU::SetDBBit(size_t n, TriState bit_val)
	{
		if (bit_val != TriState::Z)
		{
			uint8_t out = DB & ~(1 << n);
			out |= (bit_val == BaseLogic::One ? 1 : 0) << n;
			DB = out;
		}
	}

	TriState PPU::GetPDBit(size_t n)
	{
		TriState PDBit = (PD & (1 << n)) != 0 ? TriState::One : TriState::Zero;
		return PDBit;
	}

	void PPU::SetPDBit(size_t n, TriState bit_val)
	{
		if (bit_val != TriState::Z)
		{
			uint8_t out = PD & ~(1 << n);
			out |= (bit_val == BaseLogic::One ? 1 : 0) << n;
			PD = out;
		}
	}

	size_t PPU::GetHCounter()
	{
		return h->get();
	}

	size_t PPU::GetVCounter()
	{
		return v->get();
	}

	void PPU::GetSignalFeatures(VideoSignalFeatures& features)
	{
		vid_out->GetSignalFeatures(features);
	}

	void PPU::SetRAWOutput(bool enable)
	{
		vid_out->SetRAWOutput(enable);
		Disarm();
	}

	void PPU::ConvertRAWToRGB(VideoOutSignal& rawIn, VideoOutSignal& rgbOut)
	{
		vid_out->ConvertRAWToRGB(rawIn, rgbOut);
	}

	void PPU::SetOamDecayBehavior(OAMDecayBehavior behavior)
	{
		if (oam != nullptr)
		{
			oam->SetOamDecayBehavior(behavior);
		}
		Disarm();
	}

	bool PPU::IsComposite()
	{
		return vid_out->IsComposite();
	}

	void PPU::SetCompositeNoise(float volts)
	{
		vid_out->SetCompositeNoise(volts);
		Disarm();
	}

	void PPU::UseExternalDacLevels(bool use, DacLevels& tab)
	{
		vid_out->UseExternalDacLevels(use, tab);
		Disarm();
	}
}
