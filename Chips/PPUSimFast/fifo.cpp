// Object FIFO (Motion picture buffer memory)

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	FIFO::FIFO(PPU* parent) : ppu(parent) {}
	FIFO::~FIFO() {}

	static uint8_t ReverseByte(uint8_t value)
	{
		value = uint8_t(((value & 0x55) << 1) | ((value >> 1) & 0x55));
		value = uint8_t(((value & 0x33) << 2) | ((value >> 2) & 0x33));
		return uint8_t((value << 4) | (value >> 4));
	}
	static uint64_t LaneMask(uint8_t bits)
	{
		// Expand each selector bit to its byte, without carries between lanes.
		uint64_t spread = bits;
		spread = (spread | (spread << 28)) & UINT64_C(0x0000000f0000000f);
		spread = (spread | (spread << 14)) & UINT64_C(0x0003000300030003);
		spread = (spread | (spread << 7)) & UINT64_C(0x0101010101010101);
		return spread * 255;
	}
	static uint8_t LowBits(uint64_t value)
	{
		return uint8_t(((value & UINT64_C(0x0101010101010101)) * UINT64_C(0x0102040810204080)) >> 56);
	}
	static uint64_t DecrementBytes(uint64_t value)
	{
		// A sentinel high bit prevents subtraction borrowing across byte lanes.
		return ((value | UINT64_C(0x8080808080808080)) - UINT64_C(0x0101010101010101)) ^
			(~value & UINT64_C(0x8080808080808080));
	}
	static uint8_t ZeroBytes(uint64_t value)
	{
		const uint64_t nonzero = ((value & UINT64_C(0x7f7f7f7f7f7f7f7f)) + UINT64_C(0x7f7f7f7f7f7f7f7f)) | value;
		return LowBits((~nonzero & UINT64_C(0x8080808080808080)) >> 7);
	}
	template<int Phase> void FIFO::sim(EdgeContextT<Phase>& c)
	{
		TriState n_PCLK = c.n_PCLK;
		TriState PCLK = c.PCLK;
		TriState Z_HPOS = c.ZHPOS;

		zh_latch1.set(Z_HPOS, n_PCLK);
		zh_latch2.set(zh_latch1.nget(), PCLK);
		zh_latch3.set(zh_latch2.nget(), n_PCLK);
		c.n_ZH = ppu->wire.n_ZH = zh_latch3.nget();

		sim_HInv(c);
		sim_Lanes(c);
		sim_Prio(c);
	}

	template<int Phase> void FIFO::sim_HInv(const EdgeContextT<Phase>& c)
	{
		const bool high = c.PCLK == TriState::One;
		if (high && c.n_OBJ_RD_ATTR == TriState::Zero)
			HINV_FF.set(static_cast<TriState>((ppu->oam->get_OBByte() >> 6) & 1));
		if (!high) captured_pd = HINV_FF.get() == TriState::One ? ReverseByte(ppu->PD) : ppu->PD;
		packed_nTX = c.PD_FIFO == TriState::One ? ReverseByte(uint8_t(~captured_pd)) : 0xff;
	}

	template<int Phase> void FIFO::sim_Lanes(const EdgeContextT<Phase>& c) { sim_PackedLanes(c); }

	template<int Phase> void FIFO::sim_PackedLanes(const EdgeContextT<Phase>& c)
	{
		const bool high = c.PCLK == TriState::One;
		const uint8_t ob = ppu->oam->get_OBByte();
		const uint64_t ones = UINT64_C(0x0101010101010101);
		if (high)
		{
			const uint8_t attributes = c.n_OBJ_RD_ATTR == TriState::Zero ? selected : 0;
			for (unsigned n = 0; n < 3; n++)
			{
				const unsigned bit = n == 2 ? 5 : n;
				attr_input[n] = uint8_t((attr_input[n] & ~attributes) | (((ob >> bit) & 1) ? attributes : 0));
			}
			const uint8_t load = c.n_OBJ_RD_X == TriState::Zero ? selected : 0;
			const uint64_t load_mask = LaneMask(load);
			counter_keep = (counter_keep & ~load_mask) | ((ones * ob) & load_mask);
			counter_prepared = (counter_prepared & load_mask) | (~DecrementBytes(counter_keep) & ~load_mask);
			const uint8_t carry = ZeroBytes(counter_keep);
			zero_position = uint8_t(carry | (c.n_ZH == TriState::One ? zero_position : 0));
			const uint64_t shift_mask = LaneMask(enabled);
			for (unsigned plane = 0; plane < 2; plane++)
			{
				const auto control = plane == 0 ? c.n_OBJ_RD_A : c.n_OBJ_RD_B;
				const uint64_t plane_load = LaneMask(control == TriState::Zero ? uint8_t(selected & ~enabled) : 0);
				const uint64_t shifted = ((~shift_output[plane] >> 1) & UINT64_C(0x7f7f7f7f7f7f7f7f)) | UINT64_C(0x8080808080808080);
				shift_input[plane] = (shift_input[plane] & ~(shift_mask | plane_load)) |
					(shifted & shift_mask) | ((ones * packed_nTX) & plane_load);
			}
		}
		else
		{
			const unsigned index = unsigned(c.H3_Dash2) | (unsigned(c.H4_Dash2) << 1) |
				(unsigned(c.H5_Dash2) << 2);
			selected = uint8_t(1U << index);
			for (unsigned n = 0; n < 3; n++) attr_output[n] = uint8_t(~attr_input[n]);
			const uint64_t stepping = LaneMask(uint8_t(~zero_position));
			counter_keep = (counter_keep & ~stepping) | (~counter_prepared & stepping);
			counter_prepared = (counter_prepared & stepping) | (~DecrementBytes(counter_keep) & ~stepping);
			enabled = c.nVIS == TriState::Zero ? zero_position : 0;
			shift_output[0] = ~shift_input[0];
			shift_output[1] = ~shift_input[1];
		}
	}

	template<int Phase> void FIFO::sim_Prio(EdgeContextT<Phase>& c)
	{
		const uint8_t bits0 = LowBits(shift_output[0]), bits1 = LowBits(shift_output[1]);
		const uint8_t eligible = c.CLPO == TriState::Zero ? uint8_t((bits0 | bits1) & enabled) : 0;
		s0_latch.set(static_cast<TriState>(eligible & 1), c.PCLK);
		c.n_SPR0HIT = ppu->wire.n_SPR0HIT = s0_latch.nget();
		if (eligible)
		{
			unsigned first = 0;
			while ((eligible & (1U << first)) == 0) first++;
			c.n_ZCOL0 = ppu->wire.n_ZCOL0 = static_cast<TriState>(((bits0 >> first) & 1) ^ 1);
			c.n_ZCOL1 = ppu->wire.n_ZCOL1 = static_cast<TriState>(((bits1 >> first) & 1) ^ 1);
			c.ZCOL2 = ppu->wire.ZCOL2 = static_cast<TriState>(((attr_output[0] >> first) & 1) ^ 1);
			c.ZCOL3 = ppu->wire.ZCOL3 = static_cast<TriState>(((attr_output[1] >> first) & 1) ^ 1);
			c.n_ZPRIO = ppu->wire.n_ZPRIO = static_cast<TriState>(((attr_output[2] >> first) & 1) ^ 1);
			col2_latch.set(c.ZCOL2, TriState::One);
			col3_latch.set(c.ZCOL3, TriState::One);
			prio_latch.set(c.n_ZPRIO, TriState::One);
		}
		else
		{
			c.n_ZCOL0 = c.n_ZCOL1 = ppu->wire.n_ZCOL0 = ppu->wire.n_ZCOL1 = TriState::One;
			c.ZCOL2 = ppu->wire.ZCOL2 = col2_latch.get();
			c.ZCOL3 = ppu->wire.ZCOL3 = col3_latch.get();
			c.n_ZPRIO = ppu->wire.n_ZPRIO = prio_latch.get();
		}
	}
	template<int Phase> void FIFO::sim_SpriteH(EdgeContextT<Phase>& c)
	{
		if (c.n_PCLK == TriState::One)
		{
			const unsigned index = unsigned(c.H0_Dash2) | (unsigned(c.H1_Dash2) << 1) | (unsigned(c.H2_Dash2) << 2);
			static constexpr uint8_t read_mask[8]{0,0,1,2,0,4,0,8};
			sprite_reads = c.OBJ_READ == TriState::One ? read_mask[index] : 0;
		}
		c.n_OBJ_RD_ATTR = ppu->wire.n_OBJ_RD_ATTR = static_cast<TriState>((sprite_reads & 1) ^ 1);
		c.n_OBJ_RD_X = ppu->wire.n_OBJ_RD_X = static_cast<TriState>(((sprite_reads >> 1) & 1) ^ 1);
		c.n_OBJ_RD_A = ppu->wire.n_OBJ_RD_A = static_cast<TriState>(((sprite_reads >> 2) & 1) ^ 1);
		c.n_OBJ_RD_B = ppu->wire.n_OBJ_RD_B = static_cast<TriState>(((sprite_reads >> 3) & 1) ^ 1);
	}

#pragma region "FIFO Lane"

	FIFOLane::FIFOLane(PPU* parent)
	{
		ppu = parent;
	}

	FIFOLane::~FIFOLane()
	{
	}

	void FIFOLane::sim_LaneControl(TriState HSel)
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_OBJ_RD_ATTR = ppu->wire.n_OBJ_RD_ATTR;
		TriState n_OBJ_RD_X = ppu->wire.n_OBJ_RD_X;
		TriState n_OBJ_RD_A = ppu->wire.n_OBJ_RD_A;
		TriState n_OBJ_RD_B = ppu->wire.n_OBJ_RD_B;

		hsel_latch.set(HSel, n_PCLK);

		LDAT = NOR3(n_PCLK, hsel_latch.nget(), n_OBJ_RD_ATTR);
		LOAD = NOR3(n_PCLK, hsel_latch.nget(), n_OBJ_RD_X);
		T_SR0 = NOR3(n_PCLK, hsel_latch.nget(), n_OBJ_RD_A);
		T_SR1 = NOR3(n_PCLK, hsel_latch.nget(), n_OBJ_RD_B);

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
		TriState PCLK = ppu->wire.PCLK;

		STEP = NOR(PCLK, ZH_FF.get());
		UPD = NOR(LOAD, STEP);
		// LOAD is obtained in the Lane Control circuit.
	}

	/// <summary>
	/// The packed borrow calculation preserves the original low-five-bit carry shortcut.
	/// </summary>
	/// <returns></returns>
	TriState FIFOLane::sim_Counter(uint8_t packed_OB)
	{
		return down_cnt.sim(UPD, LOAD, STEP, packed_OB);
	}

	/// <summary>
	/// Switch the state of the circuit after the operation of the counter.
	/// </summary>
	/// <param name="Carry"></param>
	void FIFOLane::sim_CounterCarry(TriState Carry)
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState PCLK = ppu->wire.PCLK;
		TriState n_ZH = ppu->wire.n_ZH;

		ZH_FF.set(NOR(NOR3(n_PCLK, n_ZH, Carry), NOR(ZH_FF.get(), AND(PCLK, Carry))));
	}

	void FIFOLane::sim_PairedSREnable()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_VIS = ppu->fsm.nVIS;

		en_latch.set(NOR(ZH_FF.nget(), n_VIS), n_PCLK);
		n_EN = en_latch.nget();
		SR_EN = NOR(n_PCLK, n_EN);
	}

	void FIFOLane::sim_PairedSR(uint8_t packed_nTX)
	{
		nZ_COL0 = paired_sr[0].sim(ppu->wire.n_PCLK, T_SR0, SR_EN, packed_nTX);
		nZ_COL1 = paired_sr[1].sim(ppu->wire.n_PCLK, T_SR1, SR_EN, packed_nTX);
	}

	void FIFOLane::sim(TriState HSel, uint8_t packed_OB, uint8_t packed_nTX, FIFOLaneOutput& ZOut)
	{
		sim_LaneControl(HSel);
		sim_CounterControl();

		auto CarryOut = sim_Counter(packed_OB);
		sim_CounterCarry(CarryOut);

		sim_PairedSREnable();
		sim_PairedSR(packed_nTX);

		ZOut.nZ_COL0 = nZ_COL0;
		ZOut.nZ_COL1 = nZ_COL1;
		ZOut.Z_COL2 = Z_COL2;
		ZOut.Z_COL3 = Z_COL3;
		ZOut.nZ_PRIO = nZ_PRIO;
		ZOut.n_xEN = n_EN;
	}

	size_t FIFOLane::get_Counter()
	{
		return down_cnt.get();
	}

	TriState FIFO_Counter::sim(TriState Clock, TriState Load, TriState Step, uint8_t value)
	{
		// MUX selects on the low bit, whereas DLatch::set enables only on One.
		if ((Step & 1) != 0)
			keep = static_cast<uint8_t>(~step_latch);
		else if ((Load & 1) != 0)
			keep = value;

		if (Clock == TriState::One)
		{
			// Borrow into bit i is 1 iff all *updated* lower bits are zero.
			// The original NOR5 shortcut computes that same borrow into bit 5.
			step_latch = static_cast<uint8_t>(~static_cast<uint8_t>(keep - 1));
		}
		return keep == 0 ? TriState::One : TriState::Zero;
	}

	TriState FIFO_SR::sim(TriState n_PCLK, TriState T_SR, TriState SR_EN, uint8_t value)
	{
		if ((SR_EN & 1) != 0)
		{
			// The scalar loop runs from bit 7 down to 0. With a transparent
			// output bank, each stage sees the freshly produced preceding 1:
			// the whole bank fills in this call, rather than shifting once.
			input = n_PCLK == TriState::One ? 0xff :
				static_cast<uint8_t>((static_cast<uint8_t>(~output) >> 1) | 0x80);
		}
		else if ((T_SR & 1) != 0)
			input = value;

		if (n_PCLK == TriState::One)
			output = static_cast<uint8_t>(~input);
		return (output & 1) == 0 ? TriState::One : TriState::Zero;
	}

#pragma endregion "FIFO Lane"

	void FIFO::sim_PackedLanes() { sim_PackedLanes(CaptureEdge(*ppu)); }
	void FIFO::sim_Prio() { EdgeContext c = CaptureEdge(*ppu); sim_Prio(c); }

	template void FIFO::sim<0>(EdgeContextT<0>&);
	template void FIFO::sim<1>(EdgeContextT<1>&);
	template void FIFO::sim_SpriteH<0>(EdgeContextT<0>&);
	template void FIFO::sim_SpriteH<1>(EdgeContextT<1>&);
}
