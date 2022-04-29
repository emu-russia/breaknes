// Picture Address Register

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	PAR::PAR(PPU* parent)
	{
		ppu = parent;
	}

	PAR::~PAR()
	{
	}

	void PAR::sim()
	{
		sim_CountersControl();
		sim_CountersCarry();
		sim_Control();
		sim_FVCounter();
		sim_NTCounters();
		sim_TVCounter();
		sim_THCounter();
		sim_PARInputs();
		sim_PAR();
	}

	void PAR::sim_CountersControl()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PCLK = ppu->wire.PCLK;
		TriState W6_2_Enable = NOR(ppu->wire.n_W6_2, ppu->wire.n_DBE);
		TriState SC_CNT = ppu->fsm.SCCNT;
		TriState RESCL = ppu->fsm.RESCL;
		TriState E_EV = ppu->fsm.EEV;
		TriState TSTEP = ppu->wire.TSTEP;
		TriState F_TB = ppu->fsm.FTB;
		TriState H0_DD = ppu->wire.H0_Dash2;

		auto Temp93 = NOT(w62_latch.nget());
		W62_FF1.set(NOR(NOR(AND(n_PCLK, Temp93), W62_FF1.get()), W6_2_Enable));
		W62_FF2.set(MUX(PCLK, NOR(W62_FF1.get(), W6_2_Enable), NOT(NOT(W62_FF2.get()))));
		w62_latch.set(NOT(NOT(W62_FF2.get())), PCLK);
		Temp93 = NOT(w62_latch.nget());

		sccnt_latch.set(SC_CNT, PCLK);
		TVLOAD = NOR(NOR(AND(NOT(sccnt_latch.nget()), RESCL), Temp93), NOT(n_PCLK));
		
		eev_latch1.set(E_EV, n_PCLK);
		eev_latch2.set(eev_latch1.nget(), PCLK);
		THLOAD = NOR(NOR(Temp93, eev_latch2.nget()), PCLK);

		THSTEP = NOR(NOR(TSTEP, AND(F_TB, H0_DD)), PCLK);
		TVSTEP = NOR(NOR(E_EV, TSTEP), PCLK);
	}

	void PAR::sim_CountersCarry()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PCLK = ppu->wire.PCLK;
		TriState BLNK = ppu->fsm.BLNK;
		TriState I1_32 = ppu->wire.I1_32;
		TriState temp[7]{};

		// THZ/THZB

		temp[0] = NOT(BLNK);
		temp[1] = ppu->wire.n_THO[0];
		temp[2] = ppu->wire.n_THO[1];
		temp[3] = ppu->wire.n_THO[2];
		temp[4] = ppu->wire.n_THO[3];
		temp[5] = ppu->wire.n_THO[4];
		auto THZB = NOR6(temp);

		temp[0] = BLNK;
		auto THZ = NOR6(temp);

		// FVIN

		auto n_FVIN = NOR(NOT(BLNK), AND(NTVO, NOT(NOT(BLNK))));
		FVIN = NOT(n_FVIN);

		// TVIN/THIN

		TriState fvz[5]{};
		fvz[0] = n_FVIN;
		fvz[1] = BLNK;
		fvz[2] = ppu->wire.n_FVO[0];
		fvz[3] = ppu->wire.n_FVO[1];
		fvz[4] = ppu->wire.n_FVO[2];
		auto FVZ = NOR5(fvz);

		TVIN = NOT(NOR3(FVZ, THZB, AND(BLNK, I1_32)));
		THIN = NAND(BLNK, I1_32);

		// TVZ/TVZB

		temp[0] = NOT(BLNK);
		temp[1] = NOT(TVIN);
		temp[2] = ppu->wire.n_TVO[0];
		temp[3] = ppu->wire.n_TVO[1];
		temp[4] = ppu->wire.n_TVO[2];
		temp[5] = ppu->wire.n_TVO[3];
		temp[6] = ppu->wire.n_TVO[4];
		auto TVZB = NOR7(temp);

		temp[0] = BLNK;
		auto TVZ = NOR7(temp);

		// NTHIN/NTVIN

		NTHIN = NOT(NOR(TVZB, THZ));
		NTVIN = NOT( NOR( AND(NTHO, NOT(NOT(BLNK))) , TVZ) );

		// 0/TV

		tvz_latch1.set(TVZ, PCLK);
		tvz_latch2.set(tvz_latch1.nget(), n_PCLK);
		tvstep_latch.set(NOT(TVSTEP), n_PCLK);
		Z_TV = NOR(tvz_latch2.get(), tvstep_latch.get());
	}

	void PAR::sim_Control()
	{
		TriState nH2_D = ppu->wire.nH2_Dash;
		TriState BLNK = ppu->fsm.BLNK;
		TriState F_AT = ppu->fsm.FAT;
		TriState DB_PAR = ppu->wire.DB_PAR;

		PARR = NOR(nH2_D, BLNK);
		PAH = NOR(PARR, F_AT);
		PAL = NOR(NOT(PAH), DB_PAR);
	}

	void PAR::sim_FVCounter()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState carry = FVIN;

		for (size_t n = 0; n < 3; n++)
		{
			FVCounter[n].sim(PCLK, TVLOAD, TVSTEP, ppu->wire.FV[n], carry, ppu->wire.FVO[n], ppu->wire.n_FVO[n], carry);
		}
	}

	void PAR::sim_NTCounters()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState unused;

		NTHCounter.sim(PCLK, THLOAD, THSTEP, ppu->wire.NTH, NTHIN, NTHOut, unused, NTHO);
		NTVCounter.sim(PCLK, TVLOAD, TVSTEP, ppu->wire.NTV, NTVIN, NTVOut, unused, NTVO);
	}

	void PAR::sim_TVCounter()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState carry = TVIN;

		for (size_t n = 0; n < 5; n++)
		{
			TVCounter[n].sim_res(PCLK, TVLOAD, TVSTEP, ppu->wire.TV[n], carry, Z_TV, ppu->wire.TVO[n], ppu->wire.n_TVO[n], carry);
		}
	}

	void PAR::sim_THCounter()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState carry = THIN;

		for (size_t n = 0; n < 5; n++)
		{
			THCounter[n].sim(PCLK, THLOAD, THSTEP, ppu->wire.TH[n], carry, ppu->wire.THO[n], ppu->wire.n_THO[n], carry);
		}
	}

	void PAR::sim_PARInputs()
	{
		TriState BLNK = ppu->fsm.BLNK;

		FAT_in[0] = ppu->wire.THO[2];
		FAT_in[1] = ppu->wire.THO[3];
		FAT_in[2] = ppu->wire.THO[4];
		FAT_in[3] = ppu->wire.TVO[2];
		FAT_in[4] = ppu->wire.TVO[3];
		FAT_in[5] = ppu->wire.TVO[4];
		FAT_in[6] = TriState::One;
		FAT_in[7] = TriState::One;
		FAT_in[8] = TriState::One;
		FAT_in[9] = TriState::One;

		PAR_in[0] = ppu->wire.THO[0];
		PAR_in[1] = ppu->wire.THO[1];
		PAR_in[2] = ppu->wire.THO[2];
		PAR_in[3] = ppu->wire.THO[3];
		PAR_in[4] = ppu->wire.THO[4];
		PAR_in[5] = ppu->wire.TVO[0];
		PAR_in[6] = ppu->wire.TVO[1];
		PAR_in[7] = ppu->wire.TVO[2];
		PAR_in[8] = ppu->wire.TVO[3];
		PAR_in[9] = ppu->wire.TVO[4];
			
		FAT_in[10] = PAR_in[10] = NTHOut;
		FAT_in[11] = PAR_in[11] = NTVOut;
		FAT_in[12] = PAR_in[12] = NOR(NOT(ppu->wire.n_FVO[0]), NOT(BLNK));
		FAT_in[13] = PAR_in[13] = NOT(NOR(ppu->wire.FVO[1], NOT(BLNK)));

		for (size_t n = 0; n < 13; n++)
		{
			PAD_in[n] = ppu->wire.PAD[n];
		}
		PAD_in[13] = TriState::Zero;
	}

	void PAR::sim_PAR()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState DB_PAR = ppu->wire.DB_PAR;
		TriState F_AT = ppu->fsm.FAT;

		for (size_t n = 0; n < 8; n++)
		{
			par_lo[n].sim(PCLK, PARR, DB_PAR, PAL, F_AT, FAT_in[n], PAR_in[n], PAD_in[n], ppu->GetDBBit(n), ppu->wire.n_PA_Bot[n]);
		}

		for (size_t n = 0; n < 6; n++)
		{
			par_hi[n].sim(PCLK, PARR, PAH, F_AT, FAT_in[8 + n], PAR_in[8 + n], PAD_in[8 + n], ppu->wire.n_PA_Top[n]);
		}
	}

	void PAR_CounterBit::sim(TriState Clock, TriState Load, TriState Step,
		TriState val_in, TriState carry_in,
		TriState& val_out, TriState& n_val_out, TriState& carry_out)
	{
		auto val = MUX(Step, MUX(Load, MUX(Clock, TriState::Z, ff.get()), val_in), step_latch.nget());
		ff.set(val);
		step_latch.set(MUX(carry_in, ff.nget(), ff.get()), Clock);
		val_out = ff.get();
		n_val_out = ff.nget();
		carry_out = NOR(val_out, NOT(carry_in));
	}

	void PAR_CounterBit::sim_res(TriState Clock, TriState Load, TriState Step,
		TriState val_in, TriState carry_in, TriState Reset,
		TriState& val_out, TriState& n_val_out, TriState& carry_out)
	{
		auto val = MUX(Step, MUX(Load, MUX(Clock, TriState::Z, AND(ff.get(), NOT(Reset))), val_in), step_latch.nget());
		ff.set(val);
		step_latch.set(MUX(carry_in, ff.nget(), ff.get()), Clock);
		val_out = ff.get();
		n_val_out = ff.nget();
		carry_out = NOR(val_out, NOT(carry_in));
	}

	void PAR_LowBit::sim(TriState PCLK, TriState PARR, TriState DB_PAR, TriState PAL, TriState F_AT,
		TriState FAT_in, TriState PAL_in, TriState PAD_in, TriState DB_in,
		TriState& n_PAx)
	{
		auto val = MUX(DB_PAR, 
			MUX(PARR, 
				MUX(PAL, MUX(F_AT, TriState::Z, FAT_in), PAL_in), PAD_in), DB_in);
		in_latch.set(val, TriState::One);
		out_latch.set(in_latch.nget(), PCLK);
		n_PAx = out_latch.get();
	}

	void PAR_HighBit::sim(TriState PCLK, TriState PARR, TriState PAH, TriState F_AT,
		TriState FAT_in, TriState PAH_in, TriState PAD_in,
		TriState& n_PAx)
	{
		auto val = MUX(PARR, 
			MUX (PAH, MUX(F_AT, TriState::Z, FAT_in), PAH_in), PAD_in);
		in_latch.set(val, TriState::One);
		out_latch.set(in_latch.nget(), PCLK);
		n_PAx = out_latch.get();
	}
}
