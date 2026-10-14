// Tile Counters (nesdev `v`)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	TileCnt::TileCnt(PPU* parent)
	{
		ppu = parent;
	}

	TileCnt::~TileCnt()
	{
	}

	void TileCnt::sim()
	{
		sim_CountersControl();
		sim_CountersCarry();
		sim_FVCounter();
		sim_NTCounters();
		sim_TVCounter();
		sim_THCounter();
	}

	void TileCnt::sim_CountersControl()
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
		W62_FF1.set(NOR(AND(n_PCLK, Temp93), NOR(W62_FF1.get(), W6_2_Enable)));
		W62_FF2.set(MUX(PCLK, NOR(W62_FF1.nget(), W6_2_Enable), NOT(NOT(W62_FF2.get()))));
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

	void TileCnt::sim_CountersCarry()
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
		THZB = NOR6(temp);

		temp[0] = BLNK;
		THZ = NOR6(temp);

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
		FVZ = NOR5(fvz);

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
		TVZB = NOR7(temp);

		temp[0] = BLNK;
		temp[3] = ppu->wire.TVO[1];			// !!!
		TVZ = NOR7(temp);

		// NTHIN/NTVIN

		NTHIN = NOT(NOR(TVZB, THZ));
		NTVIN = NOT(NOR(AND(NTHO, NOT(NOT(BLNK))), TVZ));

		// 0/TV

		tvz_latch1.set(TVZ, PCLK);
		tvz_latch2.set(tvz_latch1.nget(), n_PCLK);
		tvstep_latch.set(NOT(TVSTEP), n_PCLK);
		Z_TV = NOR(tvz_latch2.get(), tvstep_latch.get());
	}

	template<unsigned Width> static void CounterOutputs(uint8_t value, uint8_t complement,
		TriState* out, TriState* nout)
	{
		for (unsigned n = 0; n < Width; ++n)
		{
			out[n] = (value & (1u << n)) ? TriState::One : TriState::Zero;
			nout[n] = (complement & (1u << n)) ? TriState::One : TriState::Zero;
		}
	}
	void TileCnt::sim_FVCounter()
	{
		uint8_t value, complement;
		FVCounter.sim(ppu->wire.PCLK, TVLOAD, TVSTEP, Pack3(ppu->wire.FV), FVIN, TriState::Zero, value, complement);
		CounterOutputs<3>(value, complement, ppu->wire.FVO, ppu->wire.n_FVO);
	}
	void TileCnt::sim_NTCounters()
	{
		uint8_t value, complement;
		NTHO = NTHCounter.sim(ppu->wire.PCLK, THLOAD, THSTEP, ppu->wire.NTH == TriState::One,
			NTHIN, TriState::Zero, value, complement);
		ppu->wire.NTHOut = value ? TriState::One : TriState::Zero;
		NTVO = NTVCounter.sim(ppu->wire.PCLK, TVLOAD, TVSTEP, ppu->wire.NTV == TriState::One,
			NTVIN, TriState::Zero, value, complement);
		ppu->wire.NTVOut = value ? TriState::One : TriState::Zero;
	}
	void TileCnt::sim_TVCounter()
	{
		uint8_t value, complement;
		TVCounter.sim(ppu->wire.PCLK, TVLOAD, TVSTEP, Pack5(ppu->wire.TV), TVIN, Z_TV, value, complement);
		CounterOutputs<5>(value, complement, ppu->wire.TVO, ppu->wire.n_TVO);
	}
	void TileCnt::sim_THCounter()
	{
		uint8_t value, complement;
		THCounter.sim(ppu->wire.PCLK, THLOAD, THSTEP, Pack5(ppu->wire.TH), THIN, TriState::Zero, value, complement);
		CounterOutputs<5>(value, complement, ppu->wire.THO, ppu->wire.n_THO);
	}

	TriState TileCounterBit::sim(TriState Clock, TriState Load, TriState Step,
		TriState val_in, TriState carry_in,
		TriState& val_out, TriState& n_val_out)
	{
		auto val = MUX(Step, MUX(Load, MUX(Clock, TriState::Z, ff.get()), val_in), step_latch.nget());
		ff.set(val);
		step_latch.set(MUX(carry_in, ff.nget(), ff.get()), Clock);
		val_out = ff.get();
		n_val_out = ff.nget();
		TriState carry_out = NOR(n_val_out, NOT(carry_in));
		return carry_out;
	}

	TriState TileCounterBit::sim_res(TriState Clock, TriState Load, TriState Step,
		TriState val_in, TriState carry_in, TriState Reset,		// Reset: clears not only the contents of the counter's input FF in keep state, but also pulldowns its output value (but NOT complement output)
		TriState& val_out, TriState& n_val_out)
	{
		auto val = MUX(Step, MUX(Load, MUX(Clock, TriState::Z, AND(ff.get(), NOT(Reset))), val_in), step_latch.nget());
		ff.set(val);
		step_latch.set(MUX(carry_in, ff.nget(), ff.get()), Clock);
		val_out = AND(ff.get(), NOT(Reset));
		n_val_out = ff.nget();
		TriState carry_out = NOR(n_val_out, NOT(carry_in));
		return carry_out;
	}
}
