// Sprite Comparison

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	ObjEval::ObjEval(PPU* parent)
	{
		ppu = parent;
	}

	ObjEval::~ObjEval()
	{
	}

	template<int Phase> void ObjEval::sim(EdgeContextT<Phase>& c)
	{
		sim_StepJohnson(c);
		sim_Comparator(c);
		sim_ComparisonFSM(c);

		sim_MainCounterControl(c);
		sim_MainCounter(c);

		sim_TempCounterControlBeforeCounter(c);
		sim_TempCounter(c);
		sim_TempCounterControlAfterCounter(c);

		sim_SpriteOVF(c);

		sim_OAMAddress(c);
	}

	template<int Phase> void ObjEval::sim_StepJohnson(const EdgeContextT<Phase>& c)
	{
		TriState PCLK = c.PCLK;
		TriState H0_DD = c.H0_Dash2;

		COPY_STEP = NOR(PCLK, NOT(H0_DD));

		// The shift register must be simulated backwards.

		i2_latch[5].set(i2_latch[4].nget(), PCLK);
		i2_latch[4].set(i2_latch[3].nget(), COPY_STEP);
		i2_latch[3].set(i2_latch[2].nget(), PCLK);
		i2_latch[2].set(i2_latch[1].nget(), COPY_STEP);
		i2_latch[1].set(i2_latch[0].nget(), PCLK);

		COPY_OVF = NOT(NOR3(i2_latch[5].nget(), i2_latch[3].nget(), i2_latch[1].nget()));
	}

	template<int Phase> void ObjEval::sim_Comparator(const EdgeContextT<Phase>& c)
	{
		// Evaluation precedes this call's OAM transaction. Capture the old OB
		// buffer only on the transparent high phase; low phase reuses that Y.
		if (c.PCLK == TriState::One)
			latched_y = static_cast<uint8_t>(ppu->oam->Dbg_Get_OAMBuffer());
		const uint8_t v = static_cast<uint8_t>(ppu->v->get());
		const int delta = static_cast<int>(v) - static_cast<int>(latched_y);
		row = static_cast<uint8_t>(delta);
		// The original bit-7 guard rejects wrapped negative differences. A
		// widened subtraction expresses the same range without false top hits.
		const int height = ppu->wire.O8_16 == TriState::One ? 16 : 8;
		OVZ = delta >= 0 && delta < height && COPY_OVF == TriState::Zero ?
			TriState::One : TriState::Zero;
	}

	template<int Phase> void ObjEval::sim_ComparisonFSM(EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;
		TriState PCLK = c.PCLK;
		TriState nF_NT = c.nFNT;
		TriState H0_DD = c.H0_Dash2;
		TriState I_OAM2 = c.IOAM2;
		TriState n_VIS = c.nVIS;
		TriState SPR_OV = get_SPR_OV();
		TriState S_EV = c.SEV;
		TriState OBJ_READ = c.OBJ_READ;
		TriState NotUsed{};

		// PD/FIFO

		TriState n_PCLK2 = NOT(c.PCLK);
		fnt_latch.set(NOT(NOR(nF_NT, NOT(H0_DD))), n_PCLK2);
		novz_latch.set(NOT(OVZ), n_PCLK2);
		TriState pd_fifo{};
		eval_FF3.sim(n_PCLK2, fnt_latch.get(), novz_latch.nget(), pd_fifo, NotUsed);
		c.PD_FIFO = ppu->wire.PD_FIFO = pd_fifo;

		// Set the command to copy the sprite if it is found.

		TriState temp[4]{};
		temp[0] = I_OAM2;
		temp[1] = n_VIS;
		temp[2] = SPR_OV;
		temp[3] = NOT(OVZ);
		DO_COPY = NOR4(temp);

		// Reload Johnson counter

		i2_latch[0].set(DO_COPY, COPY_STEP);

		// Set Mode4

		OMFG = NOR(COPY_OVF, DO_COPY);

		// Handle finding sprite 0 on the current line for the STRIKE circuit (Spr0 Hit).

		TriState nFF2_Out{};
		eval_FF2.sim(PCLK, NOT(S_EV), DO_COPY, NotUsed, nFF2_Out);
		TriState n_spr0_ev{};
		eval_FF1.sim(PCLK, NOT(OBJ_READ), nFF2_Out, n_spr0_ev, NotUsed);
		c.n_SPR0_EV = ppu->wire.n_SPR0_EV = n_spr0_ev;
	}

	template<int Phase> void ObjEval::sim_MainCounterControl(const EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;
		TriState PCLK = c.PCLK;
		TriState I_OAM2 = c.IOAM2;
		TriState n_VIS = c.nVIS;
		TriState H0_DD = c.H0_Dash2;
		TriState OFETCH = c.OFETCH;
		TriState n_W3 = ppu->wire.n_W3;
		TriState n_DBE = ppu->wire.n_DBE;
		TriState ZOMG{};

		switch (ppu->rev)
		{
			// For the PAL PPU, the $2003 write delay is screwed on. This is most likely how they fight OAM Corruption.

			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
			{
				auto W3 = NOR(n_W3, n_DBE);
				W3_FF1.set(NOR(NOR(W3, W3_FF1.get()), w3_latch3.nget()));
				auto w3_ff1_out = NOR(W3_FF1.nget(), W3);
				W3_FF2.set(NOT(NOT(MUX(PCLK, W3_FF2.get(), w3_ff1_out))));
				w3_latch1.set(W3_FF2.nget(), n_PCLK);
				w3_latch2.set(w3_latch1.nget(), PCLK);
				w3_latch3.set(w3_latch2.nget(), n_PCLK);
				w3_latch4.set(w3_latch3.nget(), PCLK);
				W3_Enable = NOR(w3_latch4.get(), w3_latch2.nget());

				// ZOMG comes from the circuit located in the same place as the EVEN/ODD circuit for the NTSC PPU (to the right of the V Decoder).

				ZOMG = c.EvenOddOut;
				break;
			}

			default:
				W3_Enable = NOR(n_W3, n_DBE);
				// In order not to change the logic below the pseudo-ZOMG is made equal to 0 and NOR becomes NOT.
				ZOMG = TriState::Zero;
				break;
		}

		init_latch.set(NAND(NOR(I_OAM2, n_VIS), H0_DD), n_PCLK);
		ofetch_latch.set(OFETCH, n_PCLK);
		OMSTEP = NAND(OR(init_latch.get(), n_PCLK), NOR(NOR(ofetch_latch.nget(), n_PCLK), ZOMG));
		OMOUT = NOR(OMSTEP, W3_Enable);
	}

	template<int Phase> void ObjEval::sim_MainCounter(const EdgeContextT<Phase>& c)
	{
		const bool mode4=NOR(c.BLNK,NOT(OMFG))==One;
		if (OMSTEP & 1) main_counter.keep=static_cast<uint8_t>(~main_counter.latch) & (mode4 ? 0xfc : 0xff);
		else if (W3_Enable & 1) main_counter.keep=ppu->DB;
		else if ((OMOUT & 1) && (c.OBJ_READ & 1)) main_counter.keep=0;
		const uint8_t value=main_counter.keep;
		const uint8_t next=mode4 ? static_cast<uint8_t>(((value+4)&0xfc)|((value+1)&3)) : static_cast<uint8_t>(value+1);
		const uint8_t carry_mask=value^next;
		if (OMOUT==One) main_counter.latch=(c.OBJ_READ & 1) ?
			static_cast<uint8_t>(~value & ~carry_mask) : static_cast<uint8_t>(~next);
		OMV=(mode4 ? (value & 0xfc)==0xfc : value==0xff) ? One : Zero;

		omv_latch.set(OMV,c.n_PCLK);
	}

	template<int Phase> void ObjEval::sim_TempCounterControlBeforeCounter(const EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;
		TriState n_EVAL = c.n_EVAL;
		TriState OMFG = this->OMFG;
		TriState I_OAM2 = c.IOAM2;
		TriState H0_D = c.H0_Dash;
		TriState n_H2_D = c.nH2_Dash;
		TriState OBJ_READ = c.OBJ_READ;

		eval_latch.set(n_EVAL, n_PCLK);
		ORES = NOR(n_PCLK, eval_latch.get());
		nomfg_latch.set(NOT(OMFG), n_PCLK);
		ioam2_latch.set(I_OAM2, n_PCLK);
		auto DontStep = NOR(NOR(NOR(nomfg_latch.get(), ioam2_latch.get()), H0_D), AND(n_H2_D, OBJ_READ));
		temp_latch1.set(NAND(OAMCTR2_FF.nget(), n_EVAL), n_PCLK);
		OSTEP = NOR3(temp_latch1.get(), n_PCLK, DontStep);
	}

	template<int Phase> void ObjEval::sim_TempCounter(const EdgeContextT<Phase>& c)
	{
		if (OSTEP & 1) temp_counter.keep=static_cast<uint8_t>(~temp_counter.latch)&31;
		else if (ORES & 1) temp_counter.keep=0;
		if (c.n_PCLK==One) temp_counter.latch=static_cast<uint8_t>(~(temp_counter.keep+1))&31;

		TMV=temp_counter.keep==31 ? One : Zero;
	}

	template<int Phase> void ObjEval::sim_TempCounterControlAfterCounter(EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;

		tmv_latch.set(TMV, n_PCLK);
		OAMCTR2_FF.set(NOR(ORES, NOR(AND(tmv_latch.get(), OSTEP), OAMCTR2_FF.get())));
		c.OAMCTR2 = ppu->wire.OAMCTR2 = OAMCTR2_FF.get();
	}

	/// <summary>
	/// Sprite Overflow logic should be simulated after the counters work.
	/// It captures the fact of their overflow (and terminates the current sprite process)
	/// </summary>
	template<int Phase> void ObjEval::sim_SpriteOVF(EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;
		TriState H0_D = c.H0_Dash;
		TriState RESCL = c.RESCL;
		TriState I_OAM2 = c.IOAM2;
		TriState n_R2 = ppu->wire.n_R2;
		TriState n_DBE = ppu->wire.n_DBE;
		auto R2_Enable = NOR(n_R2, n_DBE);

		omfg_latch.set(OMFG, n_PCLK);
		setov_latch.set(OAMCTR2_FF.nget(), n_PCLK);

		TriState temp[4]{};
		temp[0] = omfg_latch.get();
		temp[1] = setov_latch.get();
		temp[2] = H0_D;
		temp[3] = n_PCLK;
		auto TempOVF = NOR4(temp);

		auto MainOVF = AND(OMSTEP, omv_latch.get());
		SPR_OV_REG_FF.set(NOR(NOR(TempOVF, SPR_OV_REG_FF.get()), RESCL));
		SPR_OV_FF.set(NOR(NOR3(MainOVF, TempOVF, SPR_OV_FF.get()), I_OAM2));
		c.SPR_OV = ppu->wire.SPR_OV = SPR_OV_FF.get();
		ppu->SetDBBit(5, MUX(R2_Enable, TriState::Z, SPR_OV_REG_FF.get()));
	}

	template<int Phase> void ObjEval::sim_OAMAddress(EdgeContextT<Phase>& c)
	{
		TriState n_VIS = c.nVIS;
		TriState H0_DD = c.H0_Dash2;
		TriState BLNK = c.BLNK;
		TriState OAP{};

		switch (ppu->rev)
		{
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
			{
				TriState n_PCLK = c.n_PCLK;
				blnk_latch.set(BLNK, n_PCLK);
				OAP = NAND(OR(n_VIS, H0_DD), blnk_latch.nget());
				break;
			}

			default:
				OAP = NAND(OR(n_VIS, H0_DD), NOT(BLNK));
				break;
		}

		const TriState OAM8 = c.OAM8 = ppu->wire.OAM8 = NOT(OAP);

		address = OAM8 == TriState::One ? uint8_t((main_counter.keep & 7) | (temp_counter.keep << 3)) : main_counter.keep;
	}

	TriState OAMCounterBit::sim (
		TriState Clock,
		TriState Load,
		TriState Step,
		TriState BlockCount,
		TriState Reset,
		TriState val_in,
		TriState carry_in,
		TriState& val_out,
		TriState& n_val_out )
	{
		keep_ff.set(MUX(Step,
			MUX(Load, MUX(Clock, TriState::Z, NOR(Reset, keep_ff.nget())), val_in),
			NOR(cnt_latch.get(), BlockCount)));
		cnt_latch.set(MUX(carry_in, keep_ff.nget(), NOR(keep_ff.nget(), Reset)), Clock);
		TriState carry_out = NOR(keep_ff.nget(), NOT(carry_in));
		val_out = NOT(keep_ff.nget());
		n_val_out = keep_ff.nget();
		return carry_out;
	}

	TriState OAMCounterBit::get()
	{
		return keep_ff.get();
	}

	void OAMCounterBit::set(TriState value)
	{
		keep_ff.set(value);
	}

	TriState OAMCmprBit::sim (
		TriState OB_Even,
		TriState V_Even,
		TriState OB_Odd,
		TriState V_Odd,
		TriState carry_in,
		TriState& OV_Even,
		TriState& OV_Odd )
	{
		auto e0 = NAND(NOT(OB_Even), V_Even);
		auto e1 = NOR(NOT(OB_Even), V_Even);
		auto o0 = NOT(NAND(NOT(OB_Odd), V_Odd));
		auto o1 = NOR(NOT(OB_Odd), V_Odd);

		auto z1 = NOR(NOT(e0), e1);
		auto z2 = NOR(NOT(e0), NOR(carry_in, e1));
		auto z3 = NOR(o0, o1);

		OV_Even = NOT(NOR(NOR(carry_in, z1), AND(carry_in, z1)));
		OV_Odd = NOT(NOR(NOR(z2, z3), AND(z2, z3)));
		TriState carry_out = NOR3(NOR(e0, o1), o0, NOR3(e1, o1, carry_in));
		return carry_out;
	}

	void OAMPosedgeDFFE::sim(
		TriState CLK,
		TriState n_EN,
		TriState val_in,
		TriState& Q,
		TriState& n_Q)
	{
		if (CLK == TriState::One)
		{
			// Keep
			ff.set(NOT(NOT(ff.get())));
		}
		else if (CLK == TriState::Zero)
		{
			if (NOR(CLK, n_EN) == TriState::One)
			{
				ff.set(NOT(NOT(val_in)));
			}
		}

		Q = ff.get();
		n_Q = NOT(Q);
	}

	TriState ObjEval::get_SPR_OV()
	{
		return SPR_OV_FF.get();
	}

	uint32_t ObjEval::Debug_GetMainCounter() { return main_counter.keep; }
	uint32_t ObjEval::Debug_GetTempCounter() { return temp_counter.keep; }
	void ObjEval::Debug_SetMainCounter(uint32_t value) { main_counter.keep=static_cast<uint8_t>(value); }
	void ObjEval::Debug_SetTempCounter(uint32_t value) { temp_counter.keep=static_cast<uint8_t>(value)&31; }

	void ObjEval::GetDebugInfo(OAMEvalWires& wires)
	{
		wires.OMFG = ToByte(OMFG);
		wires.OMSTEP = ToByte(OMSTEP);
		wires.OMOUT = ToByte(OMOUT);
		wires.OMV = ToByte(OMV);
		wires.OSTEP = ToByte(OSTEP);
		wires.ORES = ToByte(ORES);
		wires.TMV = ToByte(TMV);
		wires.OAP = ToByte(NOT(ppu->wire.OAM8));
		wires.COPY_STEP = ToByte(COPY_STEP);
		wires.DO_COPY = ToByte(DO_COPY);
		wires.COPY_OVF = ToByte(COPY_OVF);
		wires.OVZ = ToByte(OVZ);
	}

	void ObjEval::sim_Comparator() { sim_Comparator(CaptureEdge(*ppu)); }
	void ObjEval::sim_MainCounter() { sim_MainCounter(CaptureEdge(*ppu)); }
	void ObjEval::sim_TempCounter() { sim_TempCounter(CaptureEdge(*ppu)); }

	template void ObjEval::sim<0>(EdgeContextT<0>&);
	template void ObjEval::sim<1>(EdgeContextT<1>&);
}
