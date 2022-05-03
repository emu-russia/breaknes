// Palette (Color RAM)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	void CBBit::sim(size_t bit_num, TriState& cell, TriState n_OE)
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_DB_CB = ppu->wire.n_DB_CB;
		TriState n_CB_DB = ppu->wire.n_CB_DB;

		if (n_DB_CB == TriState::Zero)
		{
			TriState DBBit = ppu->GetDBBit(bit_num);

			if (cell != TriState::Z && DBBit != TriState::Z)
			{
				cell = DBBit;
			}
		}

		if (PCLK == TriState::One)
		{
			ff.set(TriState::Zero);
		}
		else
		{
			ff.set(cell);
		}

		TriState CBOut = NOT(NAND(ff.get(), n_OE));

		latch1.set(CBOut, n_PCLK);
		latch2.set(latch1.nget(), PCLK);

		TriState DBOut = MUX(NOT(n_CB_DB), TriState::Z, latch2.nget());
		ppu->SetDBBit(bit_num, DBOut);
	}

	TriState CBBit::get_CBOut(TriState n_OE)
	{
		return NOT(NAND(ff.get(), n_OE));
	}

	CRAM::CRAM(PPU* parent)
	{
		ppu = parent;

		for (size_t n = 0; n < 6; n++)
		{
			cb[n] = new CBBit(ppu);
		}
	}

	CRAM::~CRAM()
	{
		for (size_t n = 0; n < 6; n++)
		{
			delete cb[n];
		}
	}

	void CRAM::sim()
	{
		sim_CRAMControl();
		sim_CRAMDecoder();
		sim_ColorBuffer();
	}

	void CRAM::sim_CRAMControl()
	{
		TriState n_R7 = ppu->wire.n_R7;
		TriState n_DBE = ppu->wire.n_DBE;
		TriState TH_MUX = ppu->wire.TH_MUX;
		TriState n_PICTURE = ppu->fsm.n_PICTURE;

		dbpar_latch.set(ppu->wire.DB_PAR, ppu->wire.PCLK);
		ppu->wire.n_CB_DB = NOT(NOR3(n_R7, n_DBE, NOT(TH_MUX)));
		ppu->wire.n_BW = NOR(AND(ppu->wire.n_CB_DB, n_PICTURE), ppu->wire.BnW);
		ppu->wire.n_DB_CB = NAND(dbpar_latch.get(), TH_MUX);
	}

	void CRAM::sim_CRAMDecoder()
	{
		TriState col_in[2]{};
		col_in[0] = ppu->wire.PAL[2];
		col_in[1] = ppu->wire.PAL[3];
		DMX2(col_in, COL);

		TriState row_in[3]{};
		row_in[0] = ppu->wire.PAL[0];
		row_in[1] = ppu->wire.PAL[1];
		row_in[2] = ppu->wire.PAL[4];
		DMX3(row_in, ROW);

		for (size_t n = 0; n < 8; n++)
		{
			ROW[n] = AND(ROW[n], NOT(ppu->wire.PCLK));
		}
	}

	TriState & CRAM::AddressCell(size_t lane)
	{
		size_t row;
		size_t col;
		bool anyRow = false;
		bool anyCol = false;

		// Select row

		if (ROW[0] == TriState::One || ROW[4] == TriState::One)
		{
			row = 0;
			anyRow = true;
		}
		else if (ROW[1] == TriState::One)
		{
			row = 1;
			anyRow = true;
		}
		else if (ROW[2] == TriState::One)
		{
			row = 2;
			anyRow = true;
		}
		else if (ROW[3] == TriState::One)
		{
			row = 3;
			anyRow = true;
		}
		else if (ROW[5] == TriState::One)
		{
			row = 5;
			anyRow = true;
		}
		else if (ROW[6] == TriState::One)
		{
			row = 6;
			anyRow = true;
		}
		else if (ROW[7] == TriState::One)
		{
			row = 7;
			anyRow = true;
		}

		// Select column

		if (anyRow)
		{
			if (COL[0] == TriState::One)
			{
				col = 0;
				anyCol = true;
			}
			else if (COL[1] == TriState::One)
			{
				col = 1;
				anyCol = true;
			}
			else if (COL[2] == TriState::One)
			{
				col = 2;
				anyCol = true;
			}
			else if (COL[3] == TriState::One)
			{
				col = 3;
				anyCol = true;
			}
		}

		// Address cell

		if (anyRow && anyCol)
		{
			return cram[row * col * lane];
		}

		return z_cell;
	}

	void CRAM::sim_ColorBuffer()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;

		for (size_t bit_num = 0; bit_num < cb_num; bit_num++)
		{
			cb[bit_num]->sim(
				bit_num, AddressCell(bit_num), 
				bit_num < 4 ? ppu->wire.n_BW : TriState::One );
		}

		// A chain of output latches.

		LL0_latch[0].set(cb[4]->get_CBOut(TriState::One), n_PCLK);
		LL0_latch[1].set(LL0_latch[0].nget(), PCLK);
		LL0_latch[2].set(LL0_latch[1].nget(), n_PCLK);
		ppu->wire.n_LL[0] = LL0_latch[2].nget();

		LL1_latch[0].set(cb[5]->get_CBOut(TriState::One), n_PCLK);
		LL1_latch[1].set(LL1_latch[0].nget(), PCLK);
		LL1_latch[2].set(LL1_latch[1].nget(), n_PCLK);
		ppu->wire.n_LL[1] = LL1_latch[2].nget();

		for (size_t n = 0; n < 4; n++)
		{
			CC_latch[n].set(cb[n]->get_CBOut(ppu->wire.n_BW), n_PCLK);
			ppu->wire.n_CC[n] = CC_latch[n].nget();
		}
	}

	uint8_t CRAM::Dbg_CRAMReadByte(size_t addr)
	{
		uint8_t val = 0;

		size_t row = (addr & 3) | ((addr & 0x10) ? 4 : 0);
		size_t col = (addr >> 2) & 3;

		if (row == 4)
		{
			row = 0;
		}

		for (size_t n = 0; n < 6; n++)
		{
			val |= (cram[row * col * n] == TriState::One ? 1 : 0) << n;
		}

		return val;
	}
}
