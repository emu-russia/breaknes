// OAM and secondary OAM. Every public write clocks all implemented bits
// of a byte together, so one timestamp represents their identical decay state.
#include "pch.h"
using namespace BaseLogic;
namespace PPUSimFast
{
	uint8_t OAM::Read(size_t col, size_t row, uint8_t& driven)
	{
		auto& cell = cells[col][row];
		driven = Mask(col);
		if (ppu->GetPCLKCounter() >= cell.saved + cell.timeout)
		{
			switch (decay_behav)
			{
			case OAMDecayBehavior::Keep: break;
			case OAMDecayBehavior::ToZero: return 0;
			case OAMDecayBehavior::ToOne: return 0xff;
			case OAMDecayBehavior::Evaporate: driven = 0; return 0;
			case OAMDecayBehavior::Randomize:
			{
				uint8_t value = 0;
#ifdef _WIN32
				for (size_t n = 0; n < 8; n++)
					if ((driven & (1U << n)) != 0) value |= static_cast<uint8_t>((__rdtsc() & 1) << n);
#endif
				return value;
			}
			}
		}
		return cell.value;
	}

	void OAM::Write(size_t col, size_t row, uint8_t value)
	{
		auto& cell = cells[col][row];
		cell.value = value & Mask(col);
		cell.saved = ppu->GetPCLKCounter();
		cell.timeout = 1000000;
	}

	void OAM::sim()
	{
		const bool high = ppu->wire.PCLK == TriState::One;
		const bool blank = ppu->fsm.BLNK == TriState::One;
		const uint8_t address = static_cast<uint8_t>(~Pack(ppu->wire.n_OAM));
		const size_t col = ppu->wire.OAM8 == TriState::One ? 8 : address & 7;
		const size_t row = address >> 3;
		const bool ob_to_oam = high && !blank;
		const bool secondary_write = !blank && ppu->wire.SPR_OV == TriState::Zero && !high &&
			ppu->wire.H0_Dash2 == TriState::One && ppu->wire.OAMCTR2 == TriState::Zero && ppu->fsm.nVIS == TriState::Zero;
		ppu->wire.n_WE = ppu->wire.OFETCH == TriState::Zero && !secondary_write ? TriState::One : TriState::Zero;

		if (high)
		{
			// Precharge closes every OAM row. The output buffer retains its byte.
			input_ff = 0;
			r4_latch = ob_ff;
		}
		else
		{
			uint8_t driven;
			const uint8_t value = Read(col, row, driven);
			input_ff = static_cast<uint8_t>((input_ff & ~driven) | (value & driven));
			ob_ff = ppu->fsm.IOAM2 == TriState::One ? 0xff : input_ff;
		}
		Unpack(ob_ff, ppu->wire.OB);
		if (ppu->rev != Revision::RP2C04_0003 && ppu->wire.n_R4 == TriState::Zero && ppu->wire.n_DBE == TriState::Zero)
			ppu->DB = r4_latch;
		if (blank) out_latch = ppu->DB;
		else if (ob_to_oam) out_latch = ob_ff;
		if (!high && ppu->wire.n_WE == TriState::Zero) Write(col, row, out_latch);
	}

	void OAM::sim_OFETCH()
	{
		switch (ppu->rev)
		{
			// TBD: Find out how the rest of the RGB PPUs are doing

			case Revision::RP2C04_0003:
				sim_OFETCH_RGB_PPU();
				break;

			default:
				sim_OFETCH_Default();
				break;
		}
	}

	void OAM::sim_OFETCH_Default()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_W4 = ppu->wire.n_W4;
		TriState n_DBE = ppu->wire.n_DBE;
		auto W4_Enable = NOR(n_W4, n_DBE);

		latch[0].set(OFETCH_FF.nget(), n_PCLK);
		latch[1].set(latch[0].nget(), PCLK);
		latch[2].set(latch[1].nget(), n_PCLK);
		latch[3].set(latch[2].nget(), PCLK);

		ppu->wire.OFETCH = NOR(latch[1].nget(), latch[3].get());

		W4_FF.set(NOR(W4_Enable, NOR(W4_FF.get(), latch[2].nget())));

		// OFETCH FF can be simulated after all because it is updated during PCLK = 1 and the input latch on its output is during PCLK = 0.

		OFETCH_FF.set(MUX(PCLK, NOT(NOT(OFETCH_FF.get())), NOR(W4_Enable, W4_FF.get())));
	}

	void OAM::sim_OFETCH_RGB_PPU()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_W4 = ppu->wire.n_W4;
		TriState n_DBE = ppu->wire.n_DBE;
		auto W4_Enable = NOR(n_W4, n_DBE);

		// The number of latches is 2 less. W4_FF is missing.
		
		latch[2].set(OFETCH_FF.nget(), n_PCLK);
		latch[3].set(latch[2].nget(), PCLK);

		ppu->wire.OFETCH = NOR(OFETCH_FF.nget(), latch[3].get());

		OFETCH_FF.set(MUX(PCLK, NOT(NOT(OFETCH_FF.get())), W4_Enable));
	}

	TriState OAM::get_OB(size_t n) { return (ob_ff & (1U << n)) != 0 ? TriState::One : TriState::Zero; }
	void OAM::set_OB(size_t n, TriState val)
	{
		if (val != TriState::Z) ob_ff = static_cast<uint8_t>((ob_ff & ~(1U << n)) | ((val == TriState::One ? 1U : 0U) << n));
	}
	uint8_t OAM::Dbg_OAMReadByte(size_t addr)
	{
		uint8_t mask; const uint8_t value = Read(addr & 7, addr >> 3, mask); return value & mask;
	}
	uint8_t OAM::Dbg_TempOAMReadByte(size_t addr)
	{
		uint8_t mask; const uint8_t value = Read(8, addr, mask); return value & mask;
	}
	void OAM::Dbg_OAMWriteByte(size_t addr, uint8_t val) { Write(addr & 7, addr >> 3, val); }
	void OAM::Dbg_TempOAMWriteByte(size_t addr, uint8_t val) { Write(8, addr, val); }
}
