// OAM FIFO (Motion picture buffer memory)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	FIFO::FIFO(PPU* parent)
	{
		ppu = parent;

		for (size_t n = 0; n < 8; n++)
		{
			lane[n] = new FIFOLane(ppu);
		}
	}

	FIFO::~FIFO()
	{
		for (size_t n = 0; n < 8; n++)
		{
			delete lane[n];
		}
	}

	void FIFO::sim()
	{
		sim_HInv();
		BitRev(n_TX);
		sim_Lanes();
		sim_Prio();
	}

	void FIFO::sim_HInv()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_SH2 = ppu->wire.n_SH2;
		TriState OB6 = ppu->wire.OB[6];
		TriState PD_FIFO = ppu->wire.PD_FIFO;

		HINV_FF.set(MUX(NOR(n_PCLK, n_SH2), NOT(NOT(HINV_FF.get())), OB6));

		auto HINV = HINV_FF.get();

		for (size_t n = 0; n < 8; n++)
		{
			tout_latch[n].set(MUX(HINV, ppu->GetPDBit(n), ppu->GetPDBit(7 - n)), n_PCLK);
			n_TX[n] = NAND(tout_latch[n].get(), PD_FIFO);
		}
	}

	/// <summary>
	/// Generate LaneOut outputs for the priority circuit.
	/// </summary>
	void FIFO::sim_Lanes()
	{
		TriState in[3]{};
		TriState HSel[8];

		in[0] = ppu->wire.H3_Dash2;
		in[1] = ppu->wire.H4_Dash2;
		in[2] = ppu->wire.H5_Dash2;

		DMX3(in, HSel);

		for (size_t n = 0; n < 8; n++)
		{
			lane[n]->sim(HSel[n], n_TX, LaneOut[n]);
		}
	}

	/// <summary>
	/// Based on the priorities, select one of the LaneOut values.
	/// </summary>
	void FIFO::sim_Prio()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState CLPO = ppu->wire.CLPO;
		TriState Z[8]{};
		TriState in[10]{};

		Z[0] = NOR3(AND(get_nZCOL0(0), get_nZCOL1(0)), CLPO, get_nxEN(0));

		// You can get the `/SPR0HIT` signal immediately

		s0_latch.set(Z[0], PCLK);
		ppu->wire.n_SPR0HIT = s0_latch.nget();

		in[0] = AND(get_nZCOL0(1), get_nZCOL1(1));
		in[1] = CLPO;
		in[2] = get_nxEN(1);
		in[3] = Z[0];
		Z[1] = NOR4(in);

		in[0] = AND(get_nZCOL0(2), get_nZCOL1(2));
		in[1] = CLPO;
		in[2] = get_nxEN(2);
		in[3] = Z[0];
		in[4] = Z[1];
		Z[2] = NOR5(in);

		in[0] = AND(get_nZCOL0(3), get_nZCOL1(3));
		in[1] = CLPO;
		in[2] = get_nxEN(3);
		in[3] = Z[0];
		in[4] = Z[1];
		in[5] = Z[2];
		Z[3] = NOR6(in);

		in[0] = AND(get_nZCOL0(4), get_nZCOL1(4));
		in[1] = CLPO;
		in[2] = get_nxEN(4);
		in[3] = Z[0];
		in[4] = Z[1];
		in[5] = Z[2];
		in[6] = Z[3];
		Z[4] = NOR7(in);

		in[0] = AND(get_nZCOL0(5), get_nZCOL1(5));
		in[1] = CLPO;
		in[2] = get_nxEN(5);
		in[3] = Z[0];
		in[4] = Z[1];
		in[5] = Z[2];
		in[6] = Z[3];
		in[7] = Z[4];
		Z[5] = NOR8(in);

		in[0] = AND(get_nZCOL0(6), get_nZCOL1(6));
		in[1] = CLPO;
		in[2] = get_nxEN(6);
		in[3] = Z[0];
		in[4] = Z[1];
		in[5] = Z[2];
		in[6] = Z[3];
		in[7] = Z[4];
		in[8] = Z[5];
		Z[6] = NOR9(in);

		in[0] = AND(get_nZCOL0(7), get_nZCOL1(7));
		in[1] = CLPO;
		in[2] = get_nxEN(7);
		in[3] = Z[0];
		in[4] = Z[1];
		in[5] = Z[2];
		in[6] = Z[3];
		in[7] = Z[4];
		in[8] = Z[5];
		in[9] = Z[6];
		Z[7] = NOR10(in);

		// The multiplexer locomotive is simulated by the ordinary for-loop.

		size_t run = 0;

		for (run = 0; run < 8; run++)
		{
			if (Z[run] == TriState::One)
			{
				ppu->wire.n_ZCOL0 = get_nZCOL0(run);
				ppu->wire.n_ZCOL1 = get_nZCOL1(run);
				ppu->wire.ZCOL2 = LaneOut[run][(size_t)FIFOLaneOutput::Z_COL2];
				ppu->wire.ZCOL3 = LaneOut[run][(size_t)FIFOLaneOutput::Z_COL3];
				ppu->wire.n_ZPRIO = LaneOut[run][(size_t)FIFOLaneOutput::nZ_PRIO];

				break;
			}
		}

		// When no Z is active, hidden latches are activated, which are between FIFO and MUX.
		// `/ZCOL0/1` are forcibly set to `1` (there is a corresponding nor).

		if (run == 8)
		{
			ppu->wire.n_ZCOL0 = TriState::One;
			ppu->wire.n_ZCOL1 = TriState::One;
			ppu->wire.ZCOL2 = col2_latch.get();
			ppu->wire.ZCOL3 = col3_latch.get();
			ppu->wire.n_ZPRIO = prio_latch.get();
		}

		col2_latch.set(ppu->wire.ZCOL2, TriState::One);
		col3_latch.set(ppu->wire.ZCOL3, TriState::One);
		prio_latch.set(ppu->wire.n_ZPRIO, TriState::One);
	}

	TriState FIFO::get_nZCOL0(size_t lane)
	{
		return LaneOut[lane][(size_t)FIFOLaneOutput::nZ_COL0];
	}

	TriState FIFO::get_nZCOL1(size_t lane)
	{
		return LaneOut[lane][(size_t)FIFOLaneOutput::nZ_COL1];
	}

	TriState FIFO::get_nxEN(size_t lane)
	{
		return LaneOut[lane][(size_t)FIFOLaneOutput::n_xEN];
	}

	void FIFO::sim_SpriteH()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PAR_O = ppu->fsm.PARO;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState H1_DD = ppu->wire.H1_Dash2;
		TriState H2_DD = ppu->wire.H2_Dash2;

		TriState in[3]{};
		TriState dec_out[8]{};
		in[0] = H0_DD;
		in[1] = H1_DD;
		in[2] = H2_DD;

		DMX3(in, dec_out);

		sh2_latch.set(MUX(PAR_O, dec_out[2], TriState::Zero), n_PCLK);
		sh3_latch.set(MUX(PAR_O, dec_out[3], TriState::Zero), n_PCLK);
		sh5_latch.set(MUX(PAR_O, dec_out[5], TriState::Zero), n_PCLK);
		sh7_latch.set(MUX(PAR_O, dec_out[7], TriState::Zero), n_PCLK);

		ppu->wire.n_SH2 = sh2_latch.nget();
		ppu->wire.n_SH3 = sh3_latch.nget();
		ppu->wire.n_SH5 = sh5_latch.nget();
		ppu->wire.n_SH7 = sh7_latch.nget();
	}

#pragma region "FIFO Lane"

	/// <summary>
	/// You need to get the value of the signal `/x_EN` beforehand
	/// </summary>
	void FIFOLane::sim_Enable()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_VIS = ppu->fsm.nVIS;

		en_latch.set(NOR(ZH_FF.nget(), n_VIS), n_PCLK);
		n_EN = en_latch.nget();
	}

	void FIFOLane::sim_LaneControl(TriState HSel)
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_SH2 = ppu->wire.n_SH2;
		TriState n_SH3 = ppu->wire.n_SH3;
		TriState n_SH5 = ppu->wire.n_SH5;
		TriState n_SH7 = ppu->wire.n_SH7;

		hsel_latch.set(HSel, n_PCLK);

		auto LDAT = NOR3(n_PCLK, hsel_latch.nget(), n_SH2);
		LOAD = NOR3(n_PCLK, hsel_latch.nget(), n_SH3);
		T_SR0 = NOR3(n_PCLK, hsel_latch.nget(), n_SH5);
		T_SR1 = NOR3(n_PCLK, hsel_latch.nget(), n_SH7);

		SR_EN = NOR(n_PCLK, n_EN);

		ob0_latch[0].set(ppu->wire.OB[0], LDAT);
		ob1_latch[0].set(ppu->wire.OB[1], LDAT);
		ob5_latch[0].set(ppu->wire.OB[5], LDAT);

		ob0_latch[1].set(ob0_latch[0].nget(), n_PCLK);
		ob1_latch[1].set(ob1_latch[0].nget(), n_PCLK);
		ob5_latch[1].set(ob5_latch[0].nget(), n_PCLK);

		Z_COL2 = ob0_latch[1].nget();
		Z_COL3 = ob1_latch[1].nget();
		nZ_PRIO = ob5_latch[1].nget();
	}

	void FIFOLane::sim_CounterControl()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PCLK = ppu->wire.PCLK;
		TriState Z_HPOS = ppu->fsm.ZHPOS;

		zh_latch1.set(Z_HPOS, n_PCLK);
		zh_latch2.set(zh_latch1.nget(), PCLK);
		zh_latch3.set(zh_latch2.nget(), n_PCLK);

		STEP = NOR(PCLK, ZH_FF.get());
		UPD = NOR(LOAD, STEP);
		// LOAD is obtained in the Lane Control circuit.
	}

	void FIFOLane::sim_PairedSR(TriState n_TX[8])
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState shift_out[2]{};

		shift_out[0] = TriState::One;
		shift_out[1] = TriState::One;

		for (int n = 7; n >= 0; n--)
		{
			shift_out[0] = paired_sr[0][n].sim(n_PCLK, T_SR0, SR_EN, n_TX[n], shift_out[0]);
			shift_out[1] = paired_sr[1][n].sim(n_PCLK, T_SR1, SR_EN, n_TX[n], shift_out[1]);
		}

		nZ_COL0 = shift_out[0];
		nZ_COL1 = shift_out[1];
	}

	/// <summary>
	/// The optimized carry chain IS used.
	/// </summary>
	/// <returns></returns>
	TriState FIFOLane::sim_Counter()
	{
		TriState carry = TriState::One;
		TriState val_out[5]{};
		TriState unused{};

		for (size_t n = 0; n < 5; n++)
		{
			carry = down_cnt[n].sim(UPD, LOAD, STEP, ppu->wire.OB[n], carry, val_out[n]);
		}

		carry = NOR5(val_out);

		for (size_t n = 5; n < 8; n++)
		{
			carry = down_cnt[n].sim(UPD, LOAD, STEP, ppu->wire.OB[n], carry, unused);
		}

		return carry;
	}

	/// <summary>
	/// Switch the state of the circuit after the operation of the counter.
	/// </summary>
	/// <param name="Carry"></param>
	void FIFOLane::sim_CounterCarry(TriState Carry)
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PCLK = ppu->wire.PCLK;

		ZH_FF.set(NOR(NOR3(n_PCLK, zh_latch3.nget(), Carry), NOR(ZH_FF.get(), AND(PCLK, Carry))));
	}

	void FIFOLane::sim(TriState HSel, TriState n_TX[8], TriState ZOut[(size_t)FIFOLaneOutput::Max])
	{
		sim_Enable();
		sim_LaneControl(HSel);
		sim_CounterControl();
		sim_PairedSR(n_TX);
		auto CarryOut = sim_Counter();
		sim_CounterCarry(CarryOut);

		ZOut[(size_t)FIFOLaneOutput::nZ_COL0] = nZ_COL0;
		ZOut[(size_t)FIFOLaneOutput::nZ_COL1] = nZ_COL1;
		ZOut[(size_t)FIFOLaneOutput::Z_COL2] = Z_COL2;
		ZOut[(size_t)FIFOLaneOutput::Z_COL3] = Z_COL3;
		ZOut[(size_t)FIFOLaneOutput::nZ_PRIO] = nZ_PRIO;
		ZOut[(size_t)FIFOLaneOutput::n_xEN] = n_EN;
	}

	size_t FIFOLane::get_Counter()
	{
		size_t val = 0;

		for (size_t n = 0; n < _countof(down_cnt); n++)
		{
			val |= ((down_cnt[n].get() == TriState::One) ? 1ULL : 0) << n;
		}

		return val;
	}

	TriState FIFO_CounterBit::sim(
		TriState Clock, TriState Load, TriState Step,
		TriState val_in,
		TriState carry_in,
		TriState& val_out)
	{
		keep_ff.set(MUX(Step, 
			MUX(Load, MUX(Clock, TriState::Z, keep_ff.get()), val_in),
			step_latch.nget()));
		step_latch.set(MUX(carry_in, keep_ff.nget(), keep_ff.get()), Clock);
		TriState carry_out = NOR(keep_ff.get(), NOT(carry_in));
		val_out = keep_ff.get();
		return carry_out;
	}

	TriState FIFO_CounterBit::get()
	{
		return keep_ff.get();
	}

	TriState FIFO_SRBit::sim(TriState n_PCLK, TriState T_SR, TriState SR_EN,
		TriState nTx, TriState shift_in)
	{
		in_latch.set(MUX(SR_EN, MUX(T_SR, TriState::Z, nTx), shift_in), TriState::One);
		out_latch.set(in_latch.nget(), n_PCLK);
		TriState shift_out = out_latch.nget();
		return shift_out;
	}

#pragma endregion "FIFO Lane"

}
