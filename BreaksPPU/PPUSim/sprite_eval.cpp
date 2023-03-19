// Sprite Comparison

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	OAMEval::OAMEval(PPU* parent)
	{
		ppu = parent;
	}

	OAMEval::~OAMEval()
	{
	}

	void OAMEval::sim()
	{
		sim_StepJohnson();
		sim_Comparator();
		sim_ComparisonFSM();

		sim_MainCounterControl();
		sim_MainCounter();

		sim_TempCounterControlBeforeCounter();
		sim_TempCounter();
		sim_TempCounterControlAfterCounter();

		sim_SpriteOVF();

		sim_OAMAddress();
	}

	void OAMEval::sim_StepJohnson()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState H0_DD = ppu->wire.H0_Dash2;

		COPY_STEP = NOR(PCLK, NOT(H0_DD));

		// The shift register must be simulated backwards.

		i2_latch[5].set(i2_latch[4].nget(), PCLK);
		i2_latch[4].set(i2_latch[3].nget(), COPY_STEP);
		i2_latch[3].set(i2_latch[2].nget(), PCLK);
		i2_latch[2].set(i2_latch[1].nget(), COPY_STEP);
		i2_latch[1].set(i2_latch[0].nget(), PCLK);

		COPY_OVF = NOT(NOR3(i2_latch[5].nget(), i2_latch[3].nget(), i2_latch[1].nget()));
	}

	void OAMEval::sim_Comparator()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState O8_16 = ppu->wire.O8_16;
		TriState carry_in = TriState::Zero;

		for (size_t n = 0; n < 8; n++)
		{
			OB_latch[n].set(ppu->oam->get_OB(n), PCLK);
			OB_Bits[n] = OB_latch[n].get();
		}

		if (fast_eval)
		{
			uint8_t V = (uint8_t)ppu->v->get();
			uint8_t OB = Pack(OB_Bits);
			uint8_t OV = (int8_t)V - (int8_t)OB;
			Unpack(OV, ppu->wire.OV);
		}
		else
		{
			for (size_t n = 0; n < 4; n++)
			{
				carry_in = cmpr[n].sim(
					OB_Bits[2 * n], ppu->v->getBit(2 * n),
					OB_Bits[2 * n + 1], ppu->v->getBit(2 * n + 1),
					carry_in, ppu->wire.OV[2 * n], ppu->wire.OV[2 * n + 1]);
			}
		}

		ovz_latch.set(ppu->oam->get_OB(7), PCLK);

		TriState temp[7]{};
		temp[0] = ppu->wire.OV[4];
		temp[1] = ppu->wire.OV[5];
		temp[2] = ppu->wire.OV[6];
		temp[3] = ppu->wire.OV[7];
		temp[4] = AND(ppu->wire.OV[3], NOT(O8_16));
		temp[5] = NOR(ovz_latch.nget(), ppu->v->getBit(7));
		temp[6] = COPY_OVF;
		OVZ = NOR7(temp);
	}

	void OAMEval::sim_ComparisonFSM()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PCLK = ppu->wire.PCLK;
		TriState nF_NT = ppu->fsm.nFNT;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState I_OAM2 = ppu->fsm.IOAM2;
		TriState n_VIS = ppu->fsm.nVIS;
		TriState SPR_OV = get_SPR_OV();
		TriState S_EV = ppu->fsm.SEV;
		TriState PAR_O = ppu->fsm.PARO;
		TriState NotUsed{};

		// PD/FIFO

		TriState n_PCLK2 = NOT(ppu->wire.PCLK);
		fnt_latch.set(NOT(NOR(nF_NT, NOT(H0_DD))), n_PCLK2);
		novz_latch.set(NOT(OVZ), n_PCLK2);
		eval_FF3.sim(n_PCLK2, fnt_latch.get(), novz_latch.nget(), ppu->wire.PD_FIFO, NotUsed);

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
		eval_FF1.sim(PCLK, NOT(PAR_O), nFF2_Out, ppu->wire.n_SPR0_EV, NotUsed);
	}

	void OAMEval::sim_MainCounterControl()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PCLK = ppu->wire.PCLK;
		TriState I_OAM2 = ppu->fsm.IOAM2;
		TriState n_VIS = ppu->fsm.nVIS;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState OFETCH = ppu->wire.OFETCH;
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

				ZOMG = ppu->wire.EvenOddOut;
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

	void OAMEval::sim_MainCounter()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState BLNK = ppu->fsm.BLNK;
		TriState OMFG = this->OMFG;
		TriState PAR_O = ppu->fsm.PARO;
		TriState OMOUT = this->OMOUT;
		TriState OMSTEP = this->OMSTEP;
		TriState n_out[8]{};
		auto Mode4 = NOR(BLNK, NOT(OMFG));
		TriState carry_in;
		TriState carry_out;

		carry_in = TriState::One;
		carry_out = MainCounter[0].sim(OMOUT, W3_Enable, OMSTEP, Mode4, PAR_O, ppu->GetDBBit(0), carry_in, OAM_x[0], n_out[0]);

		carry_in = carry_out;
		MainCounter[1].sim(OMOUT, W3_Enable, OMSTEP, Mode4, PAR_O, ppu->GetDBBit(1), carry_in, OAM_x[1], n_out[1]);

		auto out01 = NOT(NOR(n_out[0], n_out[1]));
		auto out01m = AND(out01, NOT(Mode4));

		carry_in = NAND(NOT(Mode4), out01);
		MainCounter[2].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(2), carry_in, OAM_x[2], n_out[2]);

		carry_in = NOR(out01m, n_out[2]);
		MainCounter[3].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(3), carry_in, OAM_x[3], n_out[3]);

		carry_in = NOR3(out01m, n_out[2], n_out[3]);
		MainCounter[4].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(4), carry_in, OAM_x[4], n_out[4]);

		TriState temp[6]{};

		temp[0] = out01m;
		temp[1] = n_out[2];
		temp[2] = n_out[3];
		temp[3] = n_out[4];
		carry_in = NOR4(temp);
		MainCounter[5].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(5), carry_in, OAM_x[5], n_out[5]);

		temp[4] = n_out[5];
		carry_in = NOR5(temp);
		MainCounter[6].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(6), carry_in, OAM_x[6], n_out[6]);

		temp[5] = n_out[6];
		carry_in = NOR6(temp);
		OMV = MainCounter[7].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(7), carry_in, OAM_x[7], n_out[7]);

		omv_latch.set(OMV, n_PCLK);
	}

	void OAMEval::sim_TempCounterControlBeforeCounter()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_EVAL = ppu->fsm.n_EVAL;
		TriState OMFG = this->OMFG;
		TriState I_OAM2 = ppu->fsm.IOAM2;
		TriState H0_D = ppu->wire.H0_Dash;
		TriState n_H2_D = ppu->wire.nH2_Dash;
		TriState PAR_O = ppu->fsm.PARO;

		eval_latch.set(n_EVAL, n_PCLK);
		ORES = NOR(n_PCLK, eval_latch.get());
		nomfg_latch.set(NOT(OMFG), n_PCLK);
		ioam2_latch.set(I_OAM2, n_PCLK);
		auto DontStep = NOR(NOR(NOR(nomfg_latch.get(), ioam2_latch.get()), H0_D), AND(n_H2_D, PAR_O));
		temp_latch1.set(NAND(OAMCTR2_FF.nget(), n_EVAL), n_PCLK);
		OSTEP = NOR3(temp_latch1.get(), n_PCLK, DontStep);
	}

	void OAMEval::sim_TempCounter()
	{
		if (fast_eval) {
			sim_TempCounterFast();
			return;
		}

		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState carry = TriState::One;
		TriState ORES = this->ORES;
		TriState OSTEP = this->OSTEP;
		TriState NotUsed;

		for (size_t n = 0; n < 5; n++)
		{
			carry = TempCounter[n].sim(n_PCLK, ORES, OSTEP, TriState::Zero, TriState::Zero, TriState::Zero,
				carry, OAM_Temp[n], NotUsed);
		}

		TMV = carry;			// carry_out from the most significant bit
	}

	void OAMEval::sim_TempCounterFast()
	{
		if (OSTEP == TriState::One) {
			fast_temp_counter++;
		}
		if (ORES == TriState::One) {
			fast_temp_counter = 0;
		}
		for (size_t n = 0; n < 5; n++)
		{
			OAM_Temp[n] = FromByte((fast_temp_counter >> n) & 1);
		}
		TMV = fast_temp_counter == 31 ? TriState::One : TriState::Zero;
	}

	void OAMEval::sim_TempCounterControlAfterCounter()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;

		tmv_latch.set(TMV, n_PCLK);
		OAMCTR2_FF.set(NOR(ORES, NOR(AND(tmv_latch.get(), OSTEP), OAMCTR2_FF.get())));
		ppu->wire.OAMCTR2 = OAMCTR2_FF.get();
	}

	/// <summary>
	/// Sprite Overflow logic should be simulated after the counters work. 
	/// It captures the fact of their overflow (and terminates the current sprite process)
	/// </summary>
	void OAMEval::sim_SpriteOVF()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState H0_D = ppu->wire.H0_Dash;
		TriState RESCL = ppu->fsm.RESCL;
		TriState I_OAM2 = ppu->fsm.IOAM2;
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
		ppu->wire.SPR_OV = SPR_OV_FF.get();
		ppu->SetDBBit(5, MUX(R2_Enable, TriState::Z, SPR_OV_REG_FF.get()));
	}

	void OAMEval::sim_OAMAddress()
	{
		TriState n_VIS = ppu->fsm.nVIS;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState BLNK = ppu->fsm.BLNK;
		TriState OAP{};

		switch (ppu->rev)
		{
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
			{
				TriState n_PCLK = ppu->wire.n_PCLK;
				blnk_latch.set(BLNK, n_PCLK);
				OAP = NAND(OR(n_VIS, H0_DD), blnk_latch.nget());
				break;
			}

			default:
				OAP = NAND(OR(n_VIS, H0_DD), NOT(BLNK));
				break;
		}

		ppu->wire.OAM8 = NOT(OAP);

		for (size_t n = 0; n < 3; n++)
		{
			ppu->wire.n_OAM[n] = NOT(OAM_x[n]);
		}

		for (size_t n = 0; n < 5; n++)
		{
			ppu->wire.n_OAM[n + 3] = NOT(MUX(ppu->wire.OAM8, OAM_x[n + 3], OAM_Temp[n]));
		}
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

	TriState OAMEval::get_SPR_OV()
	{
		return SPR_OV_FF.get();
	}

	uint32_t OAMEval::Debug_GetMainCounter()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = MainCounter[n].get();
		}
		return Pack(val_lo);
	}

	uint32_t OAMEval::Debug_GetTempCounter()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 5; n++)
		{
			val_lo[n] = TempCounter[n].get();
		}
		val_lo[5] = TriState::Zero;
		val_lo[6] = TriState::Zero;
		val_lo[7] = TriState::Zero;
		return Pack(val_lo);
	}

	void OAMEval::Debug_SetMainCounter(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		for (size_t n = 0; n < 8; n++)
		{
			MainCounter[n].set(val_lo[n]);
		}
	}

	void OAMEval::Debug_SetTempCounter(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		for (size_t n = 0; n < 5; n++)
		{
			TempCounter[n].set(val_lo[n]);
		}
	}

	void OAMEval::GetDebugInfo(OAMEvalWires& wires)
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
}
