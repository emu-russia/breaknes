// Pattern Address Generator

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	PATGen::PATGen(PPU* parent)
	{
		ppu = parent;
	}

	PATGen::~PATGen()
	{
	}

	void PATGen::sim()
	{
		sim_Control();
		sim_VInv();
		sim_PatBitsInv();
		sim_PatBit4();
		sim_PatBits();

		TriState n_H1_D = ppu->wire.nH1_Dash;
		ppu->wire.PAD[3] = NOT(n_H1_D);
	}

	void PATGen::sim_Control()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState nF_NT = ppu->fsm.nFNT;
		TriState BGSEL = ppu->wire.BGSEL;
		TriState OBSEL = ppu->wire.OBSEL;
		TriState O8_16 = ppu->wire.O8_16;
		TriState PAR_O = ppu->fsm.PARO;

		fnt_latch.set(NOT(NOR(NOT(H0_DD), nF_NT)), n_PCLK);
		O = NOR(fnt_latch.get(), n_PCLK);
		ob0_latch.set(ppu->wire.OB[0], O);
		pad12_latch.set(MUX(PAR_O, BGSEL, MUX(O8_16, OBSEL, ob0_latch.nget())), n_PCLK);

		ppu->wire.PAD[12] = pad12_latch.nget();
	}

	void PATGen::sim_VInv()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_SH2 = ppu->wire.n_SH2;

		VINV_FF.set(MUX(n_PCLK, MUX(NOR(n_PCLK, n_SH2), TriState::Z, ppu->wire.OB[7]), NOT(NOT(VINV_FF.get()))));
		VINV = NOT(NOT(VINV_FF.get()));
	}

	void PATGen::sim_PatBitsInv()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PAR_O = ppu->fsm.PARO;

		for (size_t n = 0; n < 4; n++)
		{
			inv_bits[n].sim(n_PCLK, O, VINV, ppu->wire.OV[n], inv_bits_out[n]);
		}

		pad0_latch.set(MUX(PAR_O, ppu->wire.n_FVO[0], inv_bits_out[0]), n_PCLK);
		pad1_latch.set(MUX(PAR_O, ppu->wire.n_FVO[1], inv_bits_out[1]), n_PCLK);
		pad2_latch.set(MUX(PAR_O, ppu->wire.n_FVO[2], inv_bits_out[2]), n_PCLK);

		ppu->wire.PAD[0] = pad0_latch.nget();
		ppu->wire.PAD[1] = pad1_latch.nget();
		ppu->wire.PAD[2] = pad2_latch.nget();
	}

	void PATGen::sim_PatBit4()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState val_OB = ppu->wire.OB[0];
		TriState val_PD = ppu->GetPDBit(0);
		TriState PAR_O = ppu->fsm.PARO;
		TriState O8_16 = ppu->wire.O8_16;
		TriState val_OBPrev = inv_bits_out[3];

		pdin_latch.set(NOT(val_PD), n_PCLK);
		pdout_latch.set(pdin_latch.nget(), O);
		ob_latch.set(val_OB, O);
		pad4_latch.set(MUX(PAR_O, pdout_latch.nget(), MUX(O8_16, ob_latch.nget(), val_OBPrev)), n_PCLK);
		ppu->wire.PAD[4] = pad4_latch.nget();
	}

	void PATGen::sim_PatBits()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PAR_O = ppu->fsm.PARO;

		for (size_t n = 0; n < 7; n++)
		{
			bits[n].sim(n_PCLK, O, ppu->wire.OB[n + 1], ppu->GetPDBit(n + 1), PAR_O, ppu->wire.PAD[5 + n]);
		}
	}

	void PatBitInv::sim(TriState n_PCLK, TriState O, TriState INV, TriState val_in,
		TriState& val_out)
	{
		in_latch.set(val_in, n_PCLK);
		out_latch.set(in_latch.nget(), O);
		val_out = NOT(MUX(INV, out_latch.nget(), NOT(out_latch.nget())));
	}

	void PatBit::sim(TriState n_PCLK, TriState O, TriState val_OB, TriState val_PD, TriState PAR_O,
		TriState& PADx)
	{
		pdin_latch.set(NOT(val_PD), n_PCLK);
		pdout_latch.set(pdin_latch.nget(), O);
		ob_latch.set(val_OB, O);
		padx_latch.set(MUX(PAR_O, pdout_latch.nget(), ob_latch.nget()), n_PCLK);
		PADx = padx_latch.nget();
	}
}
