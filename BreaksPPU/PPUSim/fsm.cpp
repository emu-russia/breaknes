// PPU FSM

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	FSM::FSM(PPU* parent)
	{
		ppu = parent;
	}

	FSM::~FSM()
	{
	}

	void FSM::sim(TriState* HPLA, TriState* VPLA)
	{
		sim_DelayedH();
		sim_HPosLogic(HPLA);
		sim_VPosLogic(VPLA);
		sim_VBlankInt();
		sim_EvenOdd(HPLA);
		sim_CountersControl(HPLA, VPLA);
	}

	void FSM::sim_DelayedH()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;

		for (size_t n = 0; n < 6; n++)
		{
			TriState H_bit = ppu->h->getBit(n);
			h_latch1[n].set(H_bit, n_PCLK);
			h_latch2[n].set(h_latch1[n].nget(), PCLK);
		}

		ppu->wire.H0_Dash = NOT(h_latch1[0].nget());
		ppu->wire.H0_Dash2 = h_latch2[0].nget();
		ppu->wire.nH1_Dash = h_latch1[1].nget();
		ppu->wire.H1_Dash2 = h_latch2[1].nget();
		ppu->wire.nH2_Dash = h_latch1[2].nget();
		ppu->wire.H2_Dash2 = h_latch2[2].nget();
		ppu->wire.H3_Dash2 = h_latch2[3].nget();
		ppu->wire.H4_Dash2 = h_latch2[4].nget();
		ppu->wire.H5_Dash2 = h_latch2[5].nget();
	}

	void FSM::sim_HPosLogic(TriState* HPLA)
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_OBCLIP = ppu->wire.n_OBCLIP;
		TriState n_BGCLIP = ppu->wire.n_BGCLIP;
		TriState VSYNC = vsync_latch1.get();
		TriState BLACK = ppu->wire.BLACK;

		// Front Porch appendix

		fp_latch1.set(HPLA[0], n_PCLK);
		fp_latch2.set(HPLA[1], n_PCLK);
		FPORCH_FF.set(NOR(fp_latch2.get(), NOR(fp_latch1.get(), FPORCH_FF.get())));
		TriState n_FPORCH = FPORCH_FF.get();

		// Finite States

		sev_latch1.set(HPLA[2], n_PCLK);
		sev_latch2.set(sev_latch1.nget(), PCLK);
		ppu->fsm.SEV = sev_latch2.nget();

		clpo_latch1.set(HPLA[3], n_PCLK);
		clpb_latch1.set(HPLA[4], n_PCLK);
		TriState temp1 = NOR(clpo_latch1.get(), clpb_latch1.nget());
		clpo_latch2.set(temp1, PCLK);
		clpb_latch2.set(temp1, PCLK);
		ppu->fsm.CLIP_O = NOR(n_OBCLIP, clpo_latch2.get());
		ppu->fsm.CLIP_B = NOR(n_BGCLIP, clpb_latch2.get());

		hpos_latch1.set(HPLA[5], n_PCLK);
		hpos_latch2.set(hpos_latch1.nget(), PCLK);
		ppu->fsm.ZHPOS = hpos_latch2.nget();

		eval_latch1.set(HPLA[6], n_PCLK);
		eev_latch1.set(HPLA[7], n_PCLK);
		TriState temp2 = NOR3(hpos_latch1.get(), eval_latch1.get(), eev_latch1.get());
		eval_latch2.set(temp2, PCLK);
		ppu->fsm.EVAL = NOT(eval_latch2.nget());

		eev_latch2.set(eev_latch1.nget(), PCLK);
		ppu->fsm.EEV = eev_latch2.nget();

		ioam_latch1.set(HPLA[8], n_PCLK);
		ioam_latch2.set(ioam_latch1.nget(), PCLK);
		ppu->fsm.IOAM2 = ioam_latch2.nget();

		paro_latch1.set(HPLA[9], n_PCLK);
		paro_latch2.set(paro_latch1.nget(), PCLK);
		ppu->fsm.PARO = paro_latch2.nget();

		nvis_latch1.set(HPLA[10], n_PCLK);
		nvis_latch2.set(nvis_latch1.nget(), PCLK);
		ppu->fsm.nVIS = NOT(nvis_latch2.nget());

		fnt_latch1.set(HPLA[11], n_PCLK);
		fnt_latch2.set(fnt_latch1.nget(), PCLK);
		ppu->fsm.FNT = NOT(fnt_latch2.nget());

		ftb_latch1.set(HPLA[12], n_PCLK);
		fta_latch1.set(HPLA[13], n_PCLK);
		fo_latch1.set(HPLA[14], n_PCLK);
		fo_latch2.set(HPLA[15], n_PCLK);

		ftb_latch2.set(ftb_latch1.nget(), PCLK);
		fta_latch2.set(fta_latch1.nget(), PCLK);
		TriState temp3 = NOR(fo_latch1.get(), fo_latch2.get());
		fo_latch3.set(temp3, PCLK);
		ppu->fsm.nFO = fo_latch3.nget();
		ppu->fsm.FTB = NOR(ftb_latch2.get(), fo_latch3.get());
		ppu->fsm.FTA = NOR(fta_latch2.get(), fo_latch3.get());

		fat_latch1.set(HPLA[16], n_PCLK);
		ppu->fsm.FAT = NOR(temp3, fat_latch1.nget());

		// Video signal features

		bp_latch1.set(HPLA[17], n_PCLK);
		bp_latch2.set(HPLA[18], n_PCLK);
		BPORCH_FF.set(NOR(bp_latch1.get(), NOR(bp_latch2.get(), BPORCH_FF.get())));
		ppu->fsm.BPORCH = BPORCH_FF.get();

		hb_latch1.set(HPLA[19], n_PCLK);
		hb_latch2.set(HPLA[20], n_PCLK);
		HBLANK_FF.set(NOR(hb_latch1.get(), NOR(hb_latch2.get(), HBLANK_FF.get())));
		ppu->fsm.SCCNT = NOR(HBLANK_FF.get(), BLACK);
		ppu->fsm.nHB = HBLANK_FF.nget();

		cb_latch1.set(HPLA[21], n_PCLK);
		cb_latch2.set(HPLA[22], n_PCLK);
		BURST_FF.set(NOR(cb_latch2.get(), NOR(cb_latch1.get(), BURST_FF.get())));

		sync_latch1.set(BURST_FF.get(), PCLK);
		sync_latch2.set(NOT(n_FPORCH), PCLK);
		ppu->fsm.HSYNC = NOR(sync_latch2.get(), VSYNC);
		ppu->fsm.BURST = NOR(sync_latch1.get(), ppu->fsm.HSYNC);
	}

	void FSM::sim_VPosLogic(TriState* VPLA)
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState nHB = ppu->fsm.nHB;
		TriState BPORCH = ppu->fsm.BPORCH;
		TriState BLACK = ppu->wire.BLACK;

		VSYNC_FF.set(NOR(AND(nHB, VPLA[0]), NOR(AND(nHB, VPLA[1]), VSYNC_FF.get())));
		vsync_latch1.set(NOR(nHB, VSYNC_FF.get()), PCLK);
		ppu->fsm.VSYNC = vsync_latch1.get();

		PICTURE_FF.set(NOR(AND(BPORCH, VPLA[2]), NOR(AND(BPORCH, VPLA[3]), PICTURE_FF.get())));
		pic_latch1.set(PICTURE_FF.get(), PCLK);
		pic_latch2.set(BPORCH, PCLK);
		ppu->fsm.PICTURE = NOT(NOR(pic_latch1.get(), pic_latch2.get()));

		vset_latch1.set(VPLA[4], n_PCLK);
		ppu->fsm.nVSET = vset_latch1.nget();

		vb_latch1.set(VPLA[5], n_PCLK);
		vb_latch2.set(VPLA[6], n_PCLK);
		VB_FF.set(NOR(vb_latch2.get(), NOR(vb_latch1.get(), VB_FF.get())));
		ppu->fsm.VB = NOT(VB_FF.get());

		blnk_latch1.set(VPLA[7], n_PCLK);
		BLNK_FF.set(NOR(blnk_latch1.get(), NOR(vb_latch2.get(), BLNK_FF.get())));
		ppu->fsm.BLNK = NAND(NOT(BLNK_FF.get()), NOT(BLACK));

		vclr_latch1.set(VPLA[8], n_PCLK);
		vclr_latch2.set(vclr_latch1.nget(), PCLK);
		ppu->fsm.RESCL = vclr_latch2.nget();
	}

	void FSM::sim_VBlankInt()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_VSET = ppu->fsm.nVSET;
		TriState VBL = ppu->wire.VBL;
		TriState RESCL = ppu->fsm.RESCL;	// RESCL can already be used (simulated)
		TriState n_R2 = ppu->wire.n_R2;
		TriState n_DBE = ppu->wire.n_DBE;

		TriState R2_Enable = NOR(n_R2, n_DBE);

		edge_vset_latch1.set(n_VSET, PCLK);
		edge_vset_latch2.set(edge_vset_latch1.get(), n_PCLK);
		TriState setFF = NOR3(n_PCLK, n_VSET, edge_vset_latch2.get());

		INT_FF.set(NOR3(NOR(setFF, INT_FF.get()), RESCL, R2_Enable));

		ppu->fsm.INT = NOR(NOT(VBL), NOT(INT_FF.get()));
		db_latch.set(NOT(R2_Enable), INT_FF.get());
		TriState DB7 = MUX(R2_Enable, TriState::Z, db_latch.nget());
		ppu->SetDBBit(7, DB7);
	}

	void FSM::sim_EvenOdd(TriState* HPLA)
	{
		TriState V8 = ppu->v->getBit(8);
		TriState RES = ppu->wire.RES;
		TriState RESCL = ppu->fsm.RESCL;	// RESCL can already be used (simulated)

		EvenOdd_FF1.set(NOT(NOT(MUX(V8, EvenOdd_FF2.get(), EvenOdd_FF1.get()))));
		TriState temp = NOR(RES, EvenOdd_FF2.get());
		EvenOdd_FF2.set(NOT(MUX(V8, temp, EvenOdd_FF1.get())));

		ppu->wire.EvenOddOut = NOR3(temp, NOT(HPLA[5]), NOT(RESCL));
	}

	void FSM::sim_CountersControl(TriState* HPLA, TriState* VPLA)
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState EvenOddOut = ppu->wire.EvenOddOut;

		ctrl_latch1.set(NOR(HPLA[23], EvenOddOut), n_PCLK);
		ctrl_latch2.set(VPLA[2], n_PCLK);
	}

	TriState FSM::get_RESCL()
	{
		return vclr_latch2.nget();
	}

	TriState FSM::get_HC()
	{
		return ctrl_latch1.nget();
	}

	TriState FSM::get_VC()
	{
		return NOR(NOT(get_HC()), ctrl_latch2.nget());
	}

	TriState FSM::get_VB()
	{
		return VB_FF.get();
	}

	TriState FSM::get_BLNK(TriState BLACK)
	{
		return NAND(NOT(BLNK_FF.get()), NOT(BLACK));
	}
}
