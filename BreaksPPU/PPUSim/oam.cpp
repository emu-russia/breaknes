// OAM

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	OAM::OAM(PPU* parent)
	{
		ppu = parent;

		for (size_t n = 0; n < 8; n++)
		{
			ob[n] = new OAMBufferBit(ppu);
		}

		for (size_t n = 0; n < num_lanes; n++)
		{
			lane[n] = new OAMLane(ppu, n == 2 || n == 6);
		}
	}

	OAM::~OAM()
	{
		for (size_t n = 0; n < 8; n++)
		{
			delete ob[n];
		}

		for (size_t n = 0; n < num_lanes; n++)
		{
			delete lane[n];
		}
	}

	void OAM::sim()
	{
		OAMLane* lane = sim_AddressDecoder();
		sim_OBControl();
		sim_OB(lane);
	}

	/// <summary>
	/// From the input OAM address (n_OAM0-7 + OAM8) determines the row (lane) and column number (COL).
	/// During PCLK the precharge is made and all COLx outputs are 0. This situation is handled by returning nullptr instead of lane.
	/// </summary>
	/// <returns></returns>
	OAMLane* OAM::sim_AddressDecoder()
	{
		size_t row = 0;

		if (ppu->wire.OAM8 == TriState::One)
		{
			row = 8;
		}
		else
		{
			for (size_t n = 0; n < 3; n++)
			{
				if (ppu->wire.n_OAM[n] == TriState::Zero)
				{
					row |= (1ULL << n);
				}
			}
		}

		COL = 0;

		for (size_t n = 0; n < 5; n++)
		{
			if (ppu->wire.n_OAM[3 + n] == TriState::Zero)
			{
				COL |= (1ULL << n);
			}
		}

		COL = ColMap(COL);

		return ppu->wire.PCLK == TriState::One ? nullptr : lane[row];
	}

	void OAM::sim_OBControl()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState BLNK = ppu->fsm.BLNK;
		TriState SPR_OV = ppu->wire.SPR_OV;
		TriState OAMCTR2 = ppu->wire.OAMCTR2;
		TriState H0_DD = ppu->wire.H0_Dash2;
		TriState n_VIS = ppu->fsm.nVIS;

		OB_OAM = NOR(n_PCLK, BLNK);

		TriState temp[6]{};
		temp[0] = BLNK;
		temp[1] = SPR_OV;
		temp[2] = PCLK;
		temp[3] = NOT(H0_DD);
		temp[4] = OAMCTR2;
		temp[5] = n_VIS;
		ppu->wire.n_WE = NOR(ppu->wire.OFETCH, NOR6(temp));
	}

	/// <summary>
	/// The OFETCH signal must be applied before the Sprite Eval simulation.
	/// </summary>
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

	void OAM::sim_OB(OAMLane* lane)
	{
		TriState n_WE = ppu->wire.n_WE;

		for (size_t n = 0; n < 8; n++)
		{
			ob[n]->sim(lane, COL, n, OB_OAM, n_WE);
		}
	}

	TriState OAMCell::get()
	{
		size_t pclkNow = ppu->GetPCLKCounter();
		if (pclkNow >= (savedPclk + pclksToDecay))
		{
			switch (ppu->oam->GetOamDecayBehavior())
			{
				case OAMDecayBehavior::Keep:
					return decay_ff.get();

				case OAMDecayBehavior::ToZero:
					return TriState::Zero;

				case OAMDecayBehavior::ToOne:
					return TriState::One;

				case OAMDecayBehavior::Evaporate:
					return TriState::Z;

				case OAMDecayBehavior::Randomize:
					// Year 2022. C++ still doesn't contain a standard way to get TimeStamp.
					return FromByte (__rdtsc() & 1);

				default:
					break;
			}

			return TriState::Z;
		}
		else
		{
			return decay_ff.get();
		}
	}

	void OAMCell::set(TriState val)
	{
		savedPclk = ppu->GetPCLKCounter();
		DetermineDecay();
		decay_ff.set(val);
	}

	void OAMCell::DetermineDecay()
	{
		// TBD: You can tweak individual statistical behavior (PRNG, in range, depending on ambient "temperature", etc.)

		pclksToDecay = 1000000;	// now just a constant
	}

	void OAMCell::SetTopo(OAMCellTopology place, size_t bank_num)
	{
		topo = place;
		bank = bank_num;
	}

	void OAMBufferBit::sim(OAMLane* lane, size_t column, size_t bit_num, TriState OB_OAM, TriState n_WE)
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_R4 = ppu->wire.n_R4;
		TriState n_DBE = ppu->wire.n_DBE;
		TriState BLNK = ppu->fsm.BLNK;
		TriState I_OAM2 = ppu->fsm.IOAM2;

		auto n_R4_Enable = NOT(NOR(n_R4, n_DBE));

		TriState FromOAM = TriState::Z;
		if (lane != nullptr)
		{
			lane->sim(column, bit_num, FromOAM);
		}

		Input_FF.set(MUX(PCLK, FromOAM, TriState::Zero));
		OB_FF.set(NOT(MUX(PCLK, NOR(I_OAM2, Input_FF.get()), NOT(OB_FF.get()))));
		auto OBOut = OB_FF.get();
		ppu->wire.OB[bit_num] = OBOut;
		R4_out_latch.set(OBOut, PCLK);
		ppu->SetDBBit(bit_num, MUX(NOT(n_R4_Enable), TriState::Z, R4_out_latch.get()));
		out_latch.set(MUX(BLNK, MUX(OB_OAM, TriState::Z, OBOut), ppu->GetDBBit(bit_num)), TriState::One);

		if (lane != nullptr)
		{
			TriState val_out = MUX(NOT(n_WE), TriState::Z, out_latch.get());
			lane->sim(column, bit_num, val_out);
		}
	}

	TriState OAMBufferBit::get()
	{
		return OB_FF.get();
	}

	void OAMBufferBit::set(TriState val)
	{
		return OB_FF.set(val);
	}

	OAMLane::OAMLane(PPU* parent, bool SkipAttrBits)
	{
		ppu = parent;
		skip_attr_bits = SkipAttrBits;

		for (size_t n = 0; n < cells_per_lane; n++)
		{
			cells[n] = new OAMCell(ppu);

			size_t bit = n / 8;
			size_t bank = bit < 4 ? 0 : 1;

			OAMCellTopology topo = OAMCellTopology::Middle;

			// TBD: Topology

			cells[n]->SetTopo(topo, bank);
		}
	}

	OAMLane::~OAMLane()
	{
		for (size_t n = 0; n < cells_per_lane; n++)
		{
			delete cells[n];
		}
	}

	/// <summary>
	/// Repeats the logic of the real cell.
	/// If the input is z - place the cell value on it (Read), otherwise - write a new value to the cell (Write).
	/// </summary>
	void OAMLane::sim(size_t Column, size_t bit_num, TriState& inOut)
	{
		OAMCell* cell = cells[8 * Column + bit_num];

		// Skip unused bits 2-4 for rows 2/6, which correspond to the attribute byte.

		bool skip_bit = skip_attr_bits && (bit_num == 2 || bit_num == 3 || bit_num == 4);

		if (!skip_bit)
		{
			if (inOut == TriState::Z)
			{
				inOut = cell->get();
			}
			else
			{
				cell->set(inOut);
			}
		}
	}

	/// <summary>
	/// Remaps the logical COL value to the physical column position.
	/// Physical topology: 0 - left, 31 - right.
	/// The need for this method arose because the NTSC PPU OAM address is in inverse logic (/OAM0-7) and the PAL PPU is in direct logic (OAM0-7).
	/// Therefore the physical arrangement of the columns on the chip is different.
	/// </summary>
	/// <param name="n"></param>
	/// <returns></returns>
	size_t OAM::ColMap(size_t n)
	{
		// TBD.

		// Might come in handy when we add topology support. But until the cell degradation timings are calculated, it makes no sense.

		return n;
	}

	TriState OAM::get_OB(size_t bit_num)
	{
		return ob[bit_num]->get();
	}

	void OAM::set_OB(size_t bit_num, TriState val)
	{
		ob[bit_num]->set(val);
	}

	uint8_t OAM::Dbg_OAMReadByte(size_t addr)
	{
		size_t row = addr & 7;
		OAMLane* l = lane[row];
		size_t col = addr >> 3;
		TriState val[8]{};

		for (size_t bit_num = 0; bit_num < 8; bit_num++)
		{
			val[bit_num] = TriState::Z;
			l->sim(col, bit_num, val[bit_num]);

			// Set the unused bits of the attribute byte to 0.

			if (val[bit_num] == TriState::Z)
			{
				val[bit_num] = TriState::Zero;
			}
		}

		return Pack(val);
	}

	uint8_t OAM::Dbg_TempOAMReadByte(size_t addr)
	{
		OAMLane* l = lane[8];
		size_t col = addr;
		TriState val[8]{};

		for (size_t bit_num = 0; bit_num < 8; bit_num++)
		{
			val[bit_num] = TriState::Z;
			l->sim(col, bit_num, val[bit_num]);
		}

		return Pack(val);
	}

	void OAM::Dbg_OAMWriteByte(size_t addr, uint8_t value)
	{
		size_t row = addr & 7;
		OAMLane* l = lane[row];
		size_t col = addr >> 3;
		TriState val[8]{};

		Unpack(value, val);

		for (size_t bit_num = 0; bit_num < 8; bit_num++)
		{
			l->sim(col, bit_num, val[bit_num]);
		}
	}

	void OAM::Dbg_TempOAMWriteByte(size_t addr, uint8_t value)
	{
		OAMLane* l = lane[8];
		size_t col = addr;
		TriState val[8]{};

		Unpack(value, val);

		for (size_t bit_num = 0; bit_num < 8; bit_num++)
		{
			l->sim(col, bit_num, val[bit_num]);
		}
	}

	void OAM::SetOamDecayBehavior(OAMDecayBehavior behavior)
	{
		decay_behav = behavior;
	}

	OAMDecayBehavior OAM::GetOamDecayBehavior()
	{
		return decay_behav;
	}
}
