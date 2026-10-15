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

	template<int Phase> void TileCnt::sim(const EdgeContextT<Phase>& c)
	{
		sim_CountersControl(c);
		sim_CountersCarry(c);
		sim_FVCounter(c);
		sim_NTCounters(c);
		sim_TVCounter(c);
		sim_THCounter(c);
	}

	template<int Phase> void TileCnt::sim_CountersControl(const EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;
		TriState PCLK = c.PCLK;
		TriState W6_2_Enable = NOR(ppu->wire.n_W6_2, ppu->wire.n_DBE);
		TriState SC_CNT = c.SCCNT;
		TriState RESCL = c.RESCL;
		TriState E_EV = c.EEV;
		TriState TSTEP = c.TSTEP;
		TriState F_TB = c.FTB;
		TriState H0_DD = c.H0_Dash2;

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

	template<int Phase> void TileCnt::sim_CountersCarry(const EdgeContextT<Phase>& c)
	{
		const bool blank = c.BLNK == TriState::One;
		const bool increment32 = ppu->wire.I1_32 == TriState::One;
		// These are the previous published outputs, not the current counter
		// words. Startup rails and reset pulldowns may disagree with storage.
		const bool horizontal_end = n_th == 0;
		THZB = blank && horizontal_end ? One : Zero;
		THZ = !blank && horizontal_end ? One : Zero;
		FVIN = !blank || (NTVO & 1) ? One : Zero;
		FVZ = FVIN == One && !blank && n_fv == 0 ? One : Zero;
		TVIN = FVZ == One || THZB == One || (blank && increment32) ? One : Zero;
		THIN = !blank || !increment32 ? One : Zero;
		TVZB = blank && TVIN == One && n_tv == 0 ? One : Zero;
		TVZ = !blank && TVIN == One && (n_tv & 0x1d) == 0 && (tv & 2) == 0 ? One : Zero;
		NTHIN = TVZB == One || THZ == One ? One : Zero;
		NTVIN = (blank && (NTHO & 1)) || TVZ == One ? One : Zero;
		tvz_latch1.set(TVZ, c.PCLK);
		tvz_latch2.set(tvz_latch1.nget(), c.n_PCLK);
		tvstep_latch.set(NOT(TVSTEP), c.n_PCLK);
		Z_TV = NOR(tvz_latch2.get(), tvstep_latch.get());
	}
	template<int Phase> void TileCnt::sim_FVCounter(const EdgeContextT<Phase>& c)
	{
		FVCounter.sim(c.PCLK, TVLOAD, TVSTEP, ppu->data_reader->sccx->get_FV(), FVIN, Zero, fv, n_fv);
	}
	template<int Phase> void TileCnt::sim_NTCounters(const EdgeContextT<Phase>& c)
	{
		uint8_t complement;
		NTHO = NTHCounter.sim(c.PCLK, THLOAD, THSTEP, ppu->data_reader->sccx->get_PublishedNTH(),NTHIN,Zero,nth,complement);
		NTVO = NTVCounter.sim(c.PCLK, TVLOAD, TVSTEP, ppu->data_reader->sccx->get_PublishedNTV(),NTVIN,Zero,ntv,complement);
	}
	template<int Phase> void TileCnt::sim_TVCounter(const EdgeContextT<Phase>& c)
	{
		TVCounter.sim(c.PCLK, TVLOAD, TVSTEP, ppu->data_reader->sccx->get_TV(), TVIN, Z_TV, tv, n_tv);
	}
	template<int Phase> void TileCnt::sim_THCounter(const EdgeContextT<Phase>& c)
	{
		THCounter.sim(c.PCLK, THLOAD, THSTEP, ppu->data_reader->sccx->get_TH(), THIN, Zero, th, n_th);
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

	void TileCnt::sim_CountersCarry() { sim_CountersCarry(CaptureEdge(*ppu)); }

	template void TileCnt::sim<0>(const EdgeContextT<0>&);
	template void TileCnt::sim<1>(const EdgeContextT<1>&);
}
