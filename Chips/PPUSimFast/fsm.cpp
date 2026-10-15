// PPU FSM

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	static TriState Event(uint32_t word, unsigned bit) { return static_cast<TriState>((word >> bit) & 1); }
	FSM::FSM(PPU* parent)
	{
		ppu = parent;
	}

	FSM::~FSM()
	{
	}

	template<int Phase> void FSM::sim(uint32_t HPLA, uint16_t VPLA, EdgeContextT<Phase>& c)
	{
		sim_DelayedH(c);
		sim_HPosLogic(HPLA, VPLA, c);
		sim_VPosLogic(VPLA, c);
		sim_VBlankInt(c);
		sim_EvenOdd(HPLA, VPLA, c);
		sim_CountersControl(HPLA, VPLA, c);

		prev_hpla = HPLA;
		prev_vpla = VPLA;
		Prev_n_OBCLIP = ppu->wire.n_OBCLIP;
		Prev_n_BGCLIP = ppu->wire.n_BGCLIP;
		Prev_BLACK = ppu->wire.BLACK;
	}

	template<int Phase> void FSM::sim_DelayedH(EdgeContextT<Phase>& c)
	{
		if (c.n_PCLK == TriState::One) delayed_h_input = uint8_t(ppu->h->get()) & 63;
		if (c.PCLK == TriState::One) delayed_h_output = uint8_t(~delayed_h_input) & 63;
		c.H0_Dash = ppu->wire.H0_Dash = Event(delayed_h_input, 0);
		c.H0_Dash2 = ppu->wire.H0_Dash2 = Event(~delayed_h_output, 0);
		c.nH1_Dash = ppu->wire.nH1_Dash = Event(~delayed_h_input, 1);
		c.H1_Dash2 = ppu->wire.H1_Dash2 = Event(~delayed_h_output, 1);
		c.nH2_Dash = ppu->wire.nH2_Dash = Event(~delayed_h_input, 2);
		c.H2_Dash2 = ppu->wire.H2_Dash2 = Event(~delayed_h_output, 2);
		c.H3_Dash2 = ppu->wire.H3_Dash2 = Event(~delayed_h_output, 3);
		c.H4_Dash2 = ppu->wire.H4_Dash2 = Event(~delayed_h_output, 4);
		c.H5_Dash2 = ppu->wire.H5_Dash2 = Event(~delayed_h_output, 5);
	}

	template<int Phase> void FSM::sim_HPosLogic(uint32_t HPLA, uint16_t VPLA, EdgeContextT<Phase>& c)
	{
		if (c.n_PCLK == TriState::One) horizontal_input = HPLA;
		const uint32_t h = horizontal_input;
		const unsigned sets = (h & 1) | (((h >> 17) & 1) << 1) | (((h >> 19) & 1) << 2) | (((h >> 21) & 1) << 3);
		const unsigned resets = ((h >> 1) & 1) | (((h >> 18) & 1) << 1) | (((h >> 20) & 1) << 2) | (((h >> 22) & 1) << 3);
		horizontal_sr = uint8_t((horizontal_sr | sets) & ~resets);
		if (c.PCLK == TriState::One)
		{
			constexpr uint32_t passive = (1U << 2) | (1U << 5) | (1U << 7) | (1U << 8) | (1U << 9) |
				(1U << 10) | (1U << 11) | (1U << 12) | (1U << 13);
			const unsigned clipping = (h & (1U << 3)) == 0 && (h & (1U << 4)) != 0;
			const unsigned evaluation = (h & ((1U << 5) | (1U << 6) | (1U << 7))) == 0;
			const unsigned fetch = (h & ((1U << 14) | (1U << 15))) == 0;
			horizontal_output = (~h & passive) | (clipping << 3) | (evaluation << 6) | (fetch << 14) |
				(((horizontal_sr >> 3) & 1) << 21) | (((horizontal_sr & 1) ^ 1) << 22);
		}
		const uint32_t out = horizontal_output;
		c.SEV = ppu->fsm.SEV = Event(~out, 2);
		c.CLIP_O = ppu->fsm.CLIP_O = NOR(ppu->wire.n_OBCLIP, Event(out, 3));
		c.CLIP_B = ppu->fsm.CLIP_B = NOR(ppu->wire.n_BGCLIP, Event(out, 3));
		c.ZHPOS = ppu->fsm.ZHPOS = Event(~out, 5);
		c.n_EVAL = ppu->fsm.n_EVAL = Event(out, 6);
		c.EEV = ppu->fsm.EEV = Event(~out, 7);
		c.IOAM2 = ppu->fsm.IOAM2 = Event(~out, 8);
		c.OBJ_READ = ppu->fsm.OBJ_READ = Event(~out, 9);
		c.nVIS = ppu->fsm.nVIS = Event(out, 10);
		c.nFNT = ppu->fsm.nFNT = Event(out, 11);
		c.nFO = ppu->fsm.nFO = Event(~out, 14);
		c.FTB = ppu->fsm.FTB = NOR(Event(out, 12), Event(out, 14));
		c.FTA = ppu->fsm.FTA = NOR(Event(out, 13), Event(out, 14));
		c.FAT = ppu->fsm.FAT = (h & ((1U << 14) | (1U << 15))) != 0 && (h & (1U << 16)) != 0 ? TriState::One : TriState::Zero;
		c.BPORCH = ppu->fsm.BPORCH = Event(horizontal_sr, 1);
		c.SCCNT = ppu->fsm.SCCNT = NOR(Event(~horizontal_sr, 2), ppu->wire.BLACK);
		c.nHB = ppu->fsm.nHB = Event(horizontal_sr, 2);
		// The vertical feedback consumes this call's new horizontal blank.
		sim_VSYNCEarly(VPLA, c);
		c.SYNC = ppu->fsm.SYNC = NOR(Event(out, 22), c.VSYNC);
		c.BURST = ppu->fsm.BURST = NOR(Event(out, 21), c.SYNC);
	}

	/// <summary>
	/// The VSYNC signal is a uroboros that must be propagated as soon as the /HB signal is applied.
	/// </summary>
	/// <param name="VPLA"></param>
	template<int Phase> void FSM::sim_VSYNCEarly(uint16_t VPLA, EdgeContextT<Phase>& c)
	{
		TriState PCLK = c.PCLK;
		TriState nHB = c.nHB;

		VSYNC_FF.set(NOR(AND(nHB, Event(VPLA, 0)), NOR(AND(nHB, Event(VPLA, 1)), VSYNC_FF.get())));
		vsync_latch1.set(NOR(nHB, VSYNC_FF.get()), PCLK);
		c.VSYNC = ppu->fsm.VSYNC = vsync_latch1.get();
	}

	template<int Phase> void FSM::sim_VPosLogic(uint16_t VPLA, EdgeContextT<Phase>& c)
	{
		TriState PCLK = c.PCLK;
		TriState n_PCLK = c.n_PCLK;
		TriState BPORCH = c.BPORCH;
		TriState BLACK = ppu->wire.BLACK;

		PICTURE_FF.set(NOR(AND(BPORCH, Event(VPLA, 2)), NOR(AND(BPORCH, Event(VPLA, 3)), PICTURE_FF.get())));
		pic_latch1.set(PICTURE_FF.get(), PCLK);
		pic_latch2.set(BPORCH, PCLK);
		c.n_PICTURE = ppu->fsm.n_PICTURE = NOT(NOR(pic_latch1.get(), pic_latch2.get()));

		vset_latch1.set(Event(VPLA, 4), n_PCLK);
		c.nVSET = ppu->fsm.nVSET = vset_latch1.nget();

		vb_latch1.set(Event(VPLA, 5), n_PCLK);
		vb_latch2.set(Event(VPLA, 6), n_PCLK);
		VB_FF.set(NOR(vb_latch1.get(), NOR(vb_latch2.get(), VB_FF.get())));
		c.VB = ppu->fsm.VB = NOT(VB_FF.nget());

		blnk_latch1.set(Event(VPLA, 7), n_PCLK);
		BLNK_FF.set(NOR(blnk_latch1.get(), NOR(vb_latch2.get(), BLNK_FF.get())));
		c.BLNK = ppu->fsm.BLNK = NAND(NOT(BLNK_FF.get()), NOT(BLACK));

		vclr_latch1.set(Event(VPLA, 8), n_PCLK);
		// The second half for propagation to PCLK=1 is in sim_RESCL_early
	}

	template<int Phase> void FSM::sim_VBlankInt(EdgeContextT<Phase>& c)
	{
		TriState PCLK = c.PCLK;
		TriState n_PCLK = c.n_PCLK;
		TriState n_VSET = c.nVSET;
		TriState VBL = ppu->wire.VBL;
		TriState RESCL = c.RESCL;	// RESCL can already be used (simulated)
		TriState n_R2 = ppu->wire.n_R2;
		TriState n_DBE = ppu->wire.n_DBE;

		TriState R2_Enable = NOR(n_R2, n_DBE);

		edge_vset_latch1.set(n_VSET, PCLK);
		edge_vset_latch2.set(edge_vset_latch1.nget(), n_PCLK);
		TriState setFF = NOR3(n_PCLK, n_VSET, edge_vset_latch2.get());

		if (setFF == TriState::One)
		{
			LOG_PPU(Cat_Events, "VBlank NMI set");
		}

		INT_FF.set(NOR3(NOR(setFF, INT_FF.get()), RESCL, R2_Enable));

		ppu->fsm.INT = NOR(NOT(VBL), NOT(INT_FF.get()));

		// The $2002[7] read buffer stores the *inverted* flag (die node /vbl_flag_read_buffer):
		// the schematic has an inverter on the latch D input, and its inverting output drives the
		// D7 pad driver, plus one more inverter to the pad. Two inversions => true polarity on DB7.
		// See HDL/PPU/fsm.v (VBlankInt): dlatch db_latch(.d(~int_ff_q), ...), DB7Out = ... db_latch_nq.
		db_latch.set(NOT(INT_FF.get()), NOT(R2_Enable));
		TriState DB7 = MUX(R2_Enable, TriState::Z, db_latch.nget());
		ppu->SetDBBit(7, DB7);
	}

	/// <summary>
	/// The Even/Odd circuit is to the right of the V Decoder and does different things in different PPUs.
	/// </summary>
	template<int Phase> void FSM::sim_EvenOdd(uint32_t HPLA, uint16_t VPLA, EdgeContextT<Phase>& c)
	{
		switch (ppu->rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
			{
				TriState V8 = ppu->v->getBit(8);
				TriState RES = ppu->wire.RES;
				TriState RESCL = c.RESCL;	// RESCL can already be used (simulated)

				EvenOdd_FF1.set(NOT(NOT(MUX(V8, EvenOdd_FF2.get(), EvenOdd_FF1.get()))));
				TriState temp = NOR(RES, EvenOdd_FF2.get());
				EvenOdd_FF2.set(NOT(MUX(V8, temp, EvenOdd_FF1.get())));

				c.EvenOddOut = ppu->wire.EvenOddOut = NOR3(temp, NOT(Event(HPLA, 5)), NOT(RESCL));
				break;
			}

			case Revision::RP2C07_0:
			{
				TriState n_PCLK = c.n_PCLK;
				TriState PCLK = c.PCLK;
				TriState BLNK = c.BLNK;
				TriState H0_D = c.H0_Dash;

				EvenOdd_latch1.set(Event(VPLA, 9), n_PCLK);
				EvenOdd_latch2.set(Event(VPLA, 8), n_PCLK);
				EvenOdd_FF1.set(NOR(EvenOdd_latch2.get(), NOR(EvenOdd_FF1.get(), EvenOdd_latch1.get())));
				EvenOdd_latch3.set(EvenOdd_FF1.nget(), PCLK);

				c.EvenOddOut = ppu->wire.EvenOddOut = NOR3(NOT(H0_D), NAND(BLNK, EvenOdd_latch3.nget()), n_PCLK);
				break;
			}

			// TBD: Only RP2C04-0003 has a photo so far. The Even/Odd circuit is obviously switched off, as the Dot Crawl is not required.
			// In other RGB PPU probably similar, but until there is no photo, we will not engage in speculation.

			case Revision::RP2C04_0003:
			{
				c.EvenOddOut = ppu->wire.EvenOddOut = TriState::Zero;
				break;
			}

			case Revision::UMC_UA6538:
			{
				c.EvenOddOut = ppu->wire.EvenOddOut = TriState::Zero;
				break;
			}
		}
	}

	template<int Phase> void FSM::sim_CountersControl(uint32_t HPLA, uint16_t VPLA, EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;
		TriState EvenOddOut = c.EvenOddOut;

		switch (ppu->rev)
		{
			// PAL PPU does not use EvenOddOut.

			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
			{
				ctrl_latch1.set(NOT(Event(HPLA, 23)), n_PCLK);
				ctrl_latch2.set(Event(VPLA, 8), n_PCLK);
			}
			break;

			default:
			{
				ctrl_latch1.set(NOR(Event(HPLA, 23), EvenOddOut), n_PCLK);
				ctrl_latch2.set(Event(VPLA, 2), n_PCLK);
			}
			break;
		}

		c.HC = ppu->wire.HC = ctrl_latch1.nget();
		c.VC = ppu->wire.VC = NOR(NOT(c.HC), ctrl_latch2.nget());
	}

	/// <summary>
	/// The RESCL signal is required very early for Reset_FF, so this part of the circuit is called as early as possible.
	/// </summary>
	void FSM::sim_RESCL_early()
	{
		vclr_latch2.set(vclr_latch1.nget(), ppu->wire.PCLK);
		ppu->fsm.RESCL = vclr_latch2.nget();
	}

	TriState FSM::get_VB()
	{
		return NOT(VB_FF.nget());
	}

	TriState FSM::get_BLNK(TriState BLACK)
	{
		return NAND(NOT(BLNK_FF.get()), NOT(BLACK));
	}

	TriState FSM::get_VBLFlag()
	{
		return INT_FF.get();
	}

	// Field-driven entry points for the kernel tests.
	void FSM::sim_DelayedH() { EdgeContext c = CaptureEdge(*ppu); sim_DelayedH(c); }
	void FSM::sim_HPosLogic(uint32_t HPLA, uint16_t VPLA) { EdgeContext c = CaptureEdge(*ppu); sim_HPosLogic(HPLA, VPLA, c); }

	// The two edge polarities of the pixel body.
	template void FSM::sim<0>(uint32_t, uint16_t, EdgeContextT<0>&);
	template void FSM::sim<1>(uint32_t, uint16_t, EdgeContextT<1>&);
}
