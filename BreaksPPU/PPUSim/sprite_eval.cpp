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
		sim_Comparator();
		sim_ComparisonFSM();

		sim_MainCounterControl();
		sim_MainCounter();

		sim_TempCounterControl();
		sim_TempCounter();

		sim_SpriteOVF();
		sim_OAMAddress();
	}

	void OAMEval::sim_MainCounter()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState BLNK = ppu->fsm.BLNK;
		TriState OMFG = this->OMFG;
		TriState PAR_O = ppu->fsm.PARO;
		TriState OMOUT = this->OMOUT;
		TriState OMSTEP = this->OMSTEP;
		TriState n_W3 = ppu->wire.n_W3;
		TriState n_DBE = ppu->wire.n_DBE;
		TriState n_out[8]{};
		auto Mode4 = NOR(BLNK, NOT(OMFG));
		auto W3_Enable = NOR(n_W3, n_DBE);
		TriState carry_in;
		TriState carry_out;
		TriState NotUsed;
		TriState OMV;

		carry_in = TriState::One;
		MainCounter[0].sim(OMOUT, W3_Enable, OMSTEP, Mode4, PAR_O, ppu->GetDBBit(0), carry_in, OAM_x[0], n_out[0], carry_out);

		carry_in = carry_out;
		MainCounter[1].sim(OMOUT, W3_Enable, OMSTEP, Mode4, PAR_O, ppu->GetDBBit(1), carry_in, OAM_x[1], n_out[1], NotUsed);

		auto out01 = NOT(NOR(n_out[0], n_out[1]));
		auto out01m = AND(out01, NOT(Mode4));

		carry_in = NAND(NOT(Mode4), out01);
		MainCounter[2].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(2), carry_in, OAM_x[2], n_out[2], NotUsed);

		carry_in = NOR(out01m, n_out[2]);
		MainCounter[3].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(3), carry_in, OAM_x[3], n_out[3], NotUsed);

		carry_in = NOR3(out01m, n_out[2], n_out[3]);
		MainCounter[4].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(4), carry_in, OAM_x[4], n_out[4], NotUsed);

		TriState temp[6]{};
		
		temp[0] = out01m;
		temp[1] = n_out[2];
		temp[2] = n_out[3];
		temp[3] = n_out[4];
		carry_in = NOR4(temp);
		MainCounter[5].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(5), carry_in, OAM_x[5], n_out[5], NotUsed);

		temp[4] = n_out[5];
		carry_in = NOR5(temp);
		MainCounter[6].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(6), carry_in, OAM_x[6], n_out[6], NotUsed);

		temp[5] = n_out[6];
		carry_in = NOR6(temp);
		MainCounter[7].sim(OMOUT, W3_Enable, OMSTEP, TriState::Zero, PAR_O, ppu->GetDBBit(7), carry_in, OAM_x[7], n_out[7], OMV);

		omv_latch.set(OMV, n_PCLK);
	}

	void OAMEval::sim_TempCounter()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState carry_in = TriState::One;
		TriState ORES = this->ORES;
		TriState OSTEP = this->OSTEP;
		TriState NotUsed;
		TriState TMV;

		for (size_t n = 0; n < 5; n++)
		{
			TempCounter[n].sim(n_PCLK, ORES, OSTEP, TriState::Zero, TriState::Zero, TriState::Zero,
				carry_in, OAM_Temp[n], NotUsed, carry_in);
		}

		TMV = carry_in;			// carry_out from the most significant bit
		tmv_latch.set(TMV, n_PCLK);
	}

	void OAMEval::sim_Comparator()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState O8_16 = ppu->wire.O8_16;
		TriState carry_in = TriState::Zero;
		TriState M4_OVZ = get_M4_OVZ();		// The value has not been updated yet

		for (size_t n = 0; n < 4; n++)
		{
			cmpr[2 * n].sim(PCLK,
				ppu->wire.OB[2 * n], ppu->v->getBit(2 * n),
				ppu->wire.OB[2 * n + 1], ppu->v->getBit(2 * n + 1),
				carry_in, ppu->wire.OV[2 * n], ppu->wire.OV[2 * n + 1], carry_in);
		}

		ovz_latch.set(ppu->wire.OB[7], PCLK);

		TriState temp[7]{};
		temp[0] = ppu->wire.OV[4];
		temp[1] = ppu->wire.OV[5];
		temp[2] = ppu->wire.OV[6];
		temp[3] = ppu->wire.OV[7];
		temp[4] = AND(ppu->wire.OV[3], NOT(O8_16));
		temp[5] = NOR(ovz_latch.nget(), ppu->v->getBit(7));
		temp[6] = M4_OVZ;
		OVZ = NOR7(temp);
	}

	void OAMEval::sim_MainCounterControl()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState I_OAM2 = ppu->fsm.IOAM2;
		TriState n_VIS = ppu->fsm.nVIS;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState OFETCH = ppu->wire.OFETCH;
		TriState n_W3 = ppu->wire.n_W3;
		TriState n_DBE = ppu->wire.n_DBE;

		auto W3_Enable = NOR(n_W3, n_DBE);

		init_latch.set(NAND(NOR(I_OAM2, n_VIS), H0_DD), n_PCLK);
		ofetch_latch.set(OFETCH, n_PCLK);
		OMSTEP = NAND(OR(init_latch.get(), n_PCLK), NOT(NOR(ofetch_latch.nget(), n_PCLK)));
		ASAP = AND(OMSTEP, omv_latch.get());
		OMOUT = NOR(OMSTEP, W3_Enable);
	}

	void OAMEval::sim_TempCounterControl()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState EVAL = ppu->fsm.EVAL;
		TriState OMFG = this->OMFG;
		TriState I_OAM2 = ppu->fsm.IOAM2;
		TriState H0_D = ppu->wire.H0_Dash;
		TriState n_H2_D = ppu->wire.nH2_Dash;
		TriState PAR_O = ppu->fsm.PARO;

		eval_latch.set(EVAL, n_PCLK);
		ORES = NOR(n_PCLK, eval_latch.get());
		nomfg_latch.set(NOT(OMFG), n_PCLK);
		ioam2_latch.set(I_OAM2, n_PCLK);
		auto temp = NOR(NOR(NOR(nomfg_latch.get(), ioam2_latch.get()), H0_D), AND(n_H2_D, PAR_O));
		temp_latch1.set(NAND(OAM2_CNT_FF.get(), EVAL), n_PCLK);
		OSTEP = NOR3(temp_latch1.get(), n_PCLK, temp);
		OAM2_CNT_FF.set(NOR(NOR(ORES, OAM2_CNT_FF.get()), AND(tmv_latch.get(), OSTEP)));
		ppu->wire.OAMCTR2 = OAM2_CNT_FF.nget();
	}

	void OAMEval::sim_SpriteOVF()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState OMFG = this->OMFG;
		TriState H0_D = ppu->wire.H0_Dash;
		TriState RESCL = ppu->fsm.RESCL;
		TriState I_OAM2 = ppu->fsm.IOAM2;
		TriState n_R2 = ppu->wire.n_R2;
		TriState n_DBE = ppu->wire.n_DBE;

		omfg_latch.set(OMFG, n_PCLK);
		setov_latch.set(OAM2_CNT_FF.get(), n_PCLK);

		TriState temp[4]{};
		temp[0] = omfg_latch.get();
		temp[1] = setov_latch.get();
		temp[2] = H0_D;
		temp[3] = n_PCLK;
		auto setov = NOR4(temp);

		SPR_OV_REG_FF.set(NOR(NOR(setov, SPR_OV_REG_FF.get()), RESCL));
		SPR_OV_FF.set(NOR(NOR3(ASAP, setov, SPR_OV_FF.get()), I_OAM2));
		ppu->wire.SPR_OV = SPR_OV_FF.get();
		auto R2_Enable = NOT(NOT(NOR(n_R2, n_DBE)));
		ppu->SetDBBit(5, MUX(R2_Enable, TriState::Z, SPR_OV_REG_FF.get()));
	}

	void OAMEval::sim_OAMAddress()
	{
		TriState n_VIS = ppu->fsm.nVIS;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState BLNK = ppu->fsm.BLNK;

		auto OAP = NAND(OR(n_VIS, H0_DD), NOT(BLNK));
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

	void OAMEval::sim_ComparisonFSM()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState nF_NT = ppu->fsm.nFNT;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState I_OAM2 = ppu->fsm.IOAM2;
		TriState n_VIS = ppu->fsm.nVIS;
		TriState SPR_OV = get_SPR_OV();		// The value has not been updated yet
		TriState S_EV = ppu->fsm.SEV;
		TriState PAR_O = ppu->fsm.PARO;
		TriState OVZ = this->OVZ;
		TriState NotUsed{};

		TriState n_PCLK2 = NOT(ppu->wire.PCLK);
		fnt_latch.set(NOT(NOR(nF_NT, NOT(H0_DD))), n_PCLK2);
		novz_latch.set(NOT(OVZ), n_PCLK2);
		eval_FF3.sim(n_PCLK2, fnt_latch.get(), novz_latch.get(), ppu->wire.Z_FIFO, NotUsed);

		TriState temp[4]{};
		temp[0] = I_OAM2;
		temp[1] = n_VIS;
		temp[2] = SPR_OV;
		temp[3] = NOT(OVZ);
		auto n_I2 = NOR4(temp);
		auto DDD = NOR(PCLK, NOT(H0_DD));

		i2_latch[0].set(n_I2, DDD);
		i2_latch[1].set(i2_latch[0].nget(), PCLK);
		i2_latch[2].set(i2_latch[1].nget(), DDD);
		i2_latch[3].set(i2_latch[2].nget(), PCLK);
		i2_latch[4].set(i2_latch[3].nget(), DDD);
		i2_latch[5].set(i2_latch[4].nget(), PCLK);

		OMFG = NOR(get_M4_OVZ(), n_I2);

		TriState nFF2_Out{};
		eval_FF2.sim(PCLK, NOT(S_EV), n_I2, NotUsed, nFF2_Out);
		eval_FF1.sim(PCLK, NOT(PAR_O), nFF2_Out, NotUsed, ppu->wire.I2SEV);
	}

	void OAMCounterBit::sim (
		TriState Clock,
		TriState Load,
		TriState Step,
		TriState BlockCount,
		TriState Reset,
		TriState val_in,
		TriState carry_in,
		TriState& val_out,
		TriState& n_val_out,
		TriState& carry_out )
	{
		cnt_latch.set(MUX(carry_in, keep_ff.nget(), NOR(keep_ff.nget(), Reset)), Clock);
		keep_ff.set(MUX(Step,
			MUX(Load, MUX(Clock, TriState::Z, NOR(Reset, keep_ff.nget())), val_in),
			NOR(cnt_latch.get(), BlockCount)) );
		carry_out = NOR(keep_ff.nget(), NOT(carry_in));
		val_out = NOT(keep_ff.nget());
		n_val_out = keep_ff.nget();
	}

	void OAMCmprBit::sim (
		TriState PCLK,
		TriState OB_Even,
		TriState V_Even,
		TriState OB_Odd,
		TriState V_Odd,
		TriState carry_in,
		TriState& OV_Even,
		TriState& OV_Odd,
		TriState& carry_out )
	{
		even_latch.set(OB_Even, PCLK);
		odd_latch.set(OB_Odd, PCLK);

		auto e0 = NAND(even_latch.nget(), V_Even);
		auto e1 = NOR(even_latch.nget(), V_Even);
		auto o0 = NOT(NAND(odd_latch.nget(), V_Odd));
		auto o1 = NOR(odd_latch.nget(), V_Odd);

		auto z1 = NOR(NOT(e0), e1);
		auto z2 = NOR(NOT(e0), NOR(carry_in, e1));
		auto z3 = NOR(o0, o1);

		OV_Even = NOT(NOR(NOR(carry_in, z1), AND(carry_in, z1)));
		OV_Odd = NOT(NOR(NOR(z2, z3), AND(z2, z3)));
		carry_out = NOR3(NOR(e0, o1), o0, NOR3(e1, o1, carry_in));
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

	TriState OAMEval::get_M4_OVZ()
	{
		return NOT(NOR3(i2_latch[5].nget(), i2_latch[3].nget(), i2_latch[1].nget()));
	}

	TriState OAMEval::get_SPR_OV()
	{
		return SPR_OV_FF.get();
	}

	uint32_t OAMEval::Debug_GetMainCounter()
	{
		uint32_t val = 0;

		for (size_t n = 0; n < 8; n++)
		{
			val |= (OAM_x[n] == TriState::One ? 1ULL : 0) << n;
		}

		return val;
	}

	uint32_t OAMEval::Debug_GetTempCounter()
	{
		uint32_t val = 0;

		for (size_t n = 0; n < 5; n++)
		{
			val |= (OAM_Temp[n] == TriState::One ? 1ULL : 0) << n;
		}

		return val;
	}
}
