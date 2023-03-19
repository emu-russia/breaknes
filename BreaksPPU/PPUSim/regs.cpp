// Control Registers

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	ControlRegs::ControlRegs(PPU* parent)
	{
		ppu = parent;
	}

	ControlRegs::~ControlRegs()
	{
	}

	void ControlRegs::sim()
	{
		sim_RegularRegOps();
		sim_W56RegOps();
		sim_FirstSecond_SCCX_Write();

		sim_RegFFs();

		switch (ppu->rev)
		{
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
				sim_PalBLACK();
				break;
		}
	}

	void ControlRegs::sim_RWDecoder()
	{
		TriState RnW = ppu->wire.RnW;
		TriState n_DBE = ppu->wire.n_DBE;

		ppu->wire.n_RD = NOT(NOR(NOT(RnW), n_DBE));
		ppu->wire.n_WR = NOT(NOR(RnW, n_DBE));
	}

	void ControlRegs::sim_RegularRegOps()
	{
		TriState RS0 = ppu->wire.RS[0];
		TriState RS1 = ppu->wire.RS[1];
		TriState RS2 = ppu->wire.RS[2];
		TriState RnW = ppu->wire.RnW;
		TriState in2[4]{};

		// Others

		in2[0] = NOT(RS0);
		in2[1] = NOT(RS1);
		in2[2] = NOT(RS2);
		in2[3] = NOT(RnW);
		ppu->wire.n_R7 = NOT(NOR4(in2));

		in2[0] = NOT(RS0);
		in2[1] = NOT(RS1);
		in2[2] = NOT(RS2);
		in2[3] = RnW;
		ppu->wire.n_W7 = NOT(NOR4(in2));

		in2[0] = RS0;
		in2[1] = RS1;
		in2[2] = NOT(RS2);
		in2[3] = RnW;
		ppu->wire.n_W4 = NOT(NOR4(in2));

		in2[0] = NOT(RS0);
		in2[1] = NOT(RS1);
		in2[2] = RS2;
		in2[3] = RnW;
		ppu->wire.n_W3 = NOT(NOR4(in2));

		in2[0] = RS0;
		in2[1] = NOT(RS1);
		in2[2] = RS2;
		in2[3] = NOT(RnW);
		ppu->wire.n_R2 = NOT(NOR4(in2));

		in2[0] = NOT(RS0);
		in2[1] = RS1;
		in2[2] = RS2;
		in2[3] = RnW;
		ppu->wire.n_W1 = NOT(NOR4(in2));

		in2[0] = RS0;
		in2[1] = RS1;
		in2[2] = RS2;
		in2[3] = RnW;
		ppu->wire.n_W0 = NOT(NOR4(in2));

		switch (ppu->rev)
		{
			// TBD: Add the remaining RGB PPUs

			case Revision::RP2C04_0003:
			{
				// n_R4 is not used.
			}
			break;

			default:
			{
				in2[0] = RS0;
				in2[1] = RS1;
				in2[2] = NOT(RS2);
				in2[3] = NOT(RnW);
				ppu->wire.n_R4 = NOT(NOR4(in2));
			}
			break;
		}
	}

	void ControlRegs::sim_W56RegOps()
	{
		TriState RS0 = ppu->wire.RS[0];
		TriState RS1 = ppu->wire.RS[1];
		TriState RS2 = ppu->wire.RS[2];
		TriState RnW = ppu->wire.RnW;
		TriState in[5]{};
		TriState in2[4]{};

		// SCCX

		in[0] = RS0;
		in[1] = NOT(RS1);
		in[2] = NOT(RS2);
		in[3] = get_Scnd();
		in[4] = RnW;
		ppu->wire.n_W6_1 = NOT(NOR5(in));

		in[0] = RS0;
		in[1] = NOT(RS1);
		in[2] = NOT(RS2);
		in[3] = get_Frst();
		in[4] = RnW;
		ppu->wire.n_W6_2 = NOT(NOR5(in));

		in[0] = NOT(RS0);
		in[1] = RS1;
		in[2] = NOT(RS2);
		in[3] = get_Scnd();
		in[4] = RnW;
		ppu->wire.n_W5_1 = NOT(NOR5(in));

		in[0] = NOT(RS0);
		in[1] = RS1;
		in[2] = NOT(RS2);
		in[3] = get_Frst();
		in[4] = RnW;
		ppu->wire.n_W5_2 = NOT(NOR5(in));

		in2[0] = NOT(ppu->wire.n_W5_1);
		in2[1] = NOT(ppu->wire.n_W5_2);
		in2[2] = NOT(ppu->wire.n_W6_1);
		in2[3] = NOT(ppu->wire.n_W6_2);
		n_W56 = NOR4(in2);
	}

	void ControlRegs::sim_FirstSecond_SCCX_Write()
	{
		TriState RC = ppu->wire.RC;
		TriState n_DBE = ppu->wire.n_DBE;
		TriState n_R2 = ppu->wire.n_R2;

		TriState R2_Enable = NOR(n_R2, n_DBE);
		TriState W56_Enable = NOR(n_W56, n_DBE);

		SCCX_FF1.set(NOR3(RC, R2_Enable, MUX(W56_Enable, NOT(SCCX_FF2.get()), NOT(SCCX_FF1.get()))));
		SCCX_FF2.set(NOR3(RC, R2_Enable, MUX(W56_Enable, NOT(SCCX_FF2.get()), SCCX_FF1.get())));
	}

	void ControlRegs::sim_RegFFs()
	{
		TriState RC = ppu->wire.RC;
		TriState n_W0 = ppu->wire.n_W0;
		TriState n_W1 = ppu->wire.n_W1;
		TriState n_DBE = ppu->wire.n_DBE;
		TriState W0_Enable = NOR(n_W0, n_DBE);
		TriState W1_Enable = NOR(n_W1, n_DBE);

		// Quick check for changing signals for optimization (check the DB bus change is also important)

		if (RC != Prev_RC || IsPosedge(Prev_W0_Enable, W0_Enable) || IsPosedge(Prev_W1_Enable, W1_Enable) || ppu->DB != Prev_DB)
		{
			for (size_t n = 0; n < 8; n++)
			{
				if (n >= 2)
				{
					// Bits 0 and 1 in the PAR Gen.

					PPU_CTRL0[n].set(NOR(RC, NOT(MUX(W0_Enable, PPU_CTRL0[n].get(), ppu->GetDBBit(n)))));
				}
				PPU_CTRL1[n].set(NOR(RC, NOT(MUX(W1_Enable, PPU_CTRL1[n].get(), ppu->GetDBBit(n)))));
			}

			Prev_RC = RC;
			Prev_W0_Enable = W0_Enable;
			Prev_W1_Enable = W1_Enable;
			Prev_DB = ppu->DB;
		}
		else
		{
			if (ppu->fast)
				return;
		}

		// All code below is derived from CTRL0/CTRL1 FFs and in the simulator is stored on wires with virtually infinite capacity (ppu->wire)

		// CTRL0

		i132_latch.set(PPU_CTRL0[2].get(), NOT(W0_Enable));
		ppu->wire.I1_32 = NOT(i132_latch.nget());

		obsel_latch.set(PPU_CTRL0[3].get(), NOT(W0_Enable));
		ppu->wire.OBSEL = obsel_latch.nget();

		bgsel_latch.set(PPU_CTRL0[4].get(), NOT(W0_Enable));
		ppu->wire.BGSEL = bgsel_latch.nget();

		o816_latch.set(PPU_CTRL0[5].get(), NOT(W0_Enable));
		ppu->wire.O8_16 = NOT(o816_latch.nget());

		ppu->wire.n_SLAVE = PPU_CTRL0[6].get();

		switch (ppu->rev)
		{
			// The PAL PPU (and derivative) uses a hidden latch for the VBL signal, which is stored between the open transistor and the inverter in the VBlank INT circuit.

			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
				vbl_latch.set(PPU_CTRL0[7].get(), NOT(W0_Enable));
				ppu->wire.VBL = vbl_latch.get();
				break;

			default:
				ppu->wire.VBL = PPU_CTRL0[7].get();
				break;
		}

		// CTRL1

		ppu->wire.BnW = PPU_CTRL1[0].get();

		bgclip_latch.set(PPU_CTRL1[1].get(), NOT(W1_Enable));
		ppu->wire.n_BGCLIP = ClippingAlwaysDisabled ? TriState::One : NOT(bgclip_latch.nget());

		obclip_latch.set(PPU_CTRL1[2].get(), NOT(W1_Enable));
		ppu->wire.n_OBCLIP = ClippingAlwaysDisabled ? TriState::One : NOT(obclip_latch.nget());

		bge_latch.set(PPU_CTRL1[3].get(), NOT(W1_Enable));
		obe_latch.set(PPU_CTRL1[4].get(), NOT(W1_Enable));
		ppu->wire.BGE = RenderAlwaysEnabled ? TriState::One : bge_latch.get();
		ppu->wire.OBE = RenderAlwaysEnabled ? TriState::One : obe_latch.get();
		ppu->wire.BLACK = NOR(ppu->wire.BGE, ppu->wire.OBE);

		tr_latch.set(PPU_CTRL1[5].get(), NOT(W1_Enable));
		ppu->wire.n_TR = tr_latch.nget();

		tg_latch.set(PPU_CTRL1[6].get(), NOT(W1_Enable));
		ppu->wire.n_TG = tg_latch.nget();

		ppu->wire.n_TB = NOT(PPU_CTRL1[7].get());
	}

	TriState ControlRegs::get_Frst()
	{
		return SCCX_FF1.nget();
	}

	TriState ControlRegs::get_Scnd()
	{
		return SCCX_FF1.get();
	}

	/// <summary>
	/// The CLPB/CLPO signal acquisition simulation should be done after the FSM.
	/// </summary>
	void ControlRegs::sim_CLP()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_VIS = ppu->fsm.nVIS;
		TriState CLIP_B = ppu->fsm.CLIP_B;
		TriState CLIP_O = ppu->fsm.CLIP_O;
		TriState BGE = ppu->wire.BGE;
		TriState OBE = ppu->wire.OBE;

		nvis_latch.set(n_VIS, n_PCLK);
		clipb_latch.set(CLIP_B, n_PCLK);
		clipo_latch.set(NOR3(nvis_latch.get(), CLIP_O, NOT(OBE)), n_PCLK);

		ppu->wire.n_CLPB = NOR3(nvis_latch.get(), clipb_latch.get(), NOT(BGE));
		ppu->wire.CLPO = clipo_latch.nget();
	}

	void ControlRegs::Debug_RenderAlwaysEnabled(bool enable)
	{
		RenderAlwaysEnabled = enable;
	}

	void ControlRegs::Debug_ClippingAlwaysDisabled(bool enable)
	{
		ClippingAlwaysDisabled = enable;
	}

	uint8_t ControlRegs::Debug_GetCTRL0()
	{
		uint8_t val = 0;

		for (size_t n = 0; n < 8; n++)
		{
			val |= (PPU_CTRL0[n].get() == TriState::One ? 1ULL : 0) << n;
		}

		return val;
	}

	uint8_t ControlRegs::Debug_GetCTRL1()
	{
		uint8_t val = 0;

		for (size_t n = 0; n < 8; n++)
		{
			val |= (PPU_CTRL1[n].get() == TriState::One ? 1ULL : 0) << n;
		}

		return val;
	}

	void ControlRegs::Debug_SetCTRL0(uint8_t val)
	{
		for (size_t n = 0; n < 8; n++)
		{
			TriState bit_val = FromByte((val >> n) & 1);
			PPU_CTRL0[n].set(bit_val);
		}
	}

	void ControlRegs::Debug_SetCTRL1(uint8_t val)
	{
		for (size_t n = 0; n < 8; n++)
		{
			TriState bit_val = FromByte((val >> n) & 1);
			PPU_CTRL1[n].set(bit_val);
		}
	}

	/// <summary>
	/// The `/SLAVE` signal is used for EXT input terminals.
	/// </summary>
	/// <returns></returns>
	TriState ControlRegs::get_nSLAVE()
	{
		return PPU_CTRL0[6].get();
	}

	/// <summary>
	/// Special BLACK signal processing for PAL-Like PPUs.
	/// </summary>
	void ControlRegs::sim_PalBLACK()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;

		BLACK_FF1.set(MUX(PCLK, NOT(NOT(BLACK_FF1.get())), ppu->wire.BLACK));
		BLACK_FF2.set(MUX(n_PCLK, NOT(NOT(BLACK_FF2.get())), NOT(NOT(BLACK_FF1.get()))));
		black_latch1.set(NOT(NOT(BLACK_FF2.get())), PCLK);
		black_latch2.set(black_latch1.nget(), n_PCLK);
		ppu->wire.BLACK = black_latch2.nget();
	}
}
