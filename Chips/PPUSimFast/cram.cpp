// Palette lookup is a byte transaction; explicit latches retain pixel timing.
#include "pch.h"
using namespace BaseLogic;
namespace PPUSimFast
{
	void CRAM::sim()
	{
		sim_CRAMControl();
		const bool high = ppu->wire.PCLK == TriState::One;
		if (high) color = 0;
		else
		{
			const size_t address = Address(Pack5(ppu->wire.CGA));
			if (ppu->wire.n_DB_CB == TriState::Zero) palette[address] = ppu->DB & 0x3f;
			color = palette[address];
		}
		const uint8_t cb_out = static_cast<uint8_t>((color & 0x30) | (ppu->wire.n_BW == TriState::One ? color & 0x0f : 0));
		if (ppu->rev != Revision::RP2C04_0003)
		{
			if (high) read_output = static_cast<uint8_t>(~read_input) & 0x3f;
			else read_input = cb_out;
			if (ppu->wire.n_CB_DB == TriState::Zero)
				ppu->DB = static_cast<uint8_t>((ppu->DB & 0xc0) | (static_cast<uint8_t>(~read_output) & 0x3f));
		}
		if (high) luma[1] = static_cast<uint8_t>(~luma[0]) & 3;
		else
		{
			luma[0] = color >> 4;
			luma[2] = static_cast<uint8_t>(~luma[1]) & 3;
			chroma = cb_out & 0x0f;
		}
		const uint8_t n_luma = static_cast<uint8_t>(~luma[2]);
		ppu->wire.n_LL[0] = static_cast<TriState>(n_luma & 1);
		ppu->wire.n_LL[1] = static_cast<TriState>((n_luma >> 1) & 1);
		const uint8_t n_chroma = static_cast<uint8_t>(~chroma);
		for (size_t n = 0; n < 4; n++) ppu->wire.n_CC[n] = static_cast<TriState>((n_chroma >> n) & 1);
	}

	void CRAM::sim_CRAMControl()
	{
		TriState n_R7 = ppu->wire.n_R7;
		TriState n_DBE = ppu->wire.n_DBE;
		TriState TH_MUX = ppu->wire.TH_MUX;
		TriState n_PICTURE = ppu->fsm.n_PICTURE;

		switch (ppu->rev)
		{
			// TBD: Add the remaining RGB PPUs

			case Revision::RP2C04_0003:
			{
				dbpar_latch.set(ppu->wire.DB_PAR, ppu->wire.PCLK);
				ppu->wire.n_BW = NOR(n_PICTURE, ppu->wire.BnW);
				ppu->wire.n_DB_CB = NAND(dbpar_latch.get(), TH_MUX);
			}
			break;

			default:
			{
				dbpar_latch.set(ppu->wire.DB_PAR, ppu->wire.PCLK);
				ppu->wire.n_CB_DB = NOT(NOR3(n_R7, n_DBE, NOT(TH_MUX)));
				ppu->wire.n_BW = NOR(AND(ppu->wire.n_CB_DB, n_PICTURE), ppu->wire.BnW);
				ppu->wire.n_DB_CB = NAND(dbpar_latch.get(), TH_MUX);
			}
			break;
		}
	}

}
