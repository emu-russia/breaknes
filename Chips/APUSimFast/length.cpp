// Length Counters

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
{
	static constexpr uint8_t LengthReload[32] = { 0x9, 0xfd, 0x13, 0x1, 0x27, 0x3, 0x4f, 0x5, 0x9f, 0x7, 0x3b, 0x9, 0xd, 0xb, 0x19, 0xd, 0xb, 0xf, 0x17, 0x11, 0x2f, 0x13, 0x5f, 0x15, 0xbf, 0x17, 0x47, 0x19, 0xf, 0x1b, 0x1f, 0x1d };
	LengthCounter::LengthCounter(APU* parent)
	{
		apu = parent;
	}

	LengthCounter::~LengthCounter()
	{
	}

	void LengthCounter::sim(size_t bit_ena, TriState WriteEn, TriState LC_CarryIn, TriState& LC_NoCount)
	{
		sim_Decoder();
		sim_Counter(LC_CarryIn, WriteEn);
		sim_Control(bit_ena, WriteEn, LC_NoCount);
	}

	void LengthCounter::sim_Control(size_t bit_ena, TriState WriteEn, TriState& LC_NoCount)
	{
		TriState nACLK2 = apu->wire.nACLK2;
		TriState ACLK1 = apu->wire.ACLK1;
		TriState RES = apu->wire.RES;
		TriState n_LFO2 = apu->wire.n_LFO2;
		TriState W4015 = apu->wire.W4015;
		TriState n_R4015 = apu->wire.n_R4015;
		TriState ACLK4 = NOT(nACLK2);
		TriState LCDIS;

		reg_enable_latch.set(
			MUX(ACLK1,
				MUX(W4015,
					MUX(RES, TriState::Z, TriState::Zero),
					apu->GetDBBit(bit_ena)),
				NOT(reg_enable_latch.nget())), TriState::One);
		LCDIS = reg_enable_latch.nget();

		ena_latch.set(LCDIS, ACLK1);
		cout_latch.set(carry_out, ACLK1);

		STEP = NOR(step_latch.get(), n_LFO2);
		step_latch.set(stat_ff.nget(), ACLK1);

		stat_ff.set(NOR4(AND(ena_latch.get(), ACLK4), AND(cout_latch.get(), STEP), RES, NOR(stat_ff.get(), WriteEn)));

		LC_NoCount = NOT(stat_ff.get());
		apu->SetDBBit(bit_ena, NOT(n_R4015) == TriState::One ? NOT(stat_ff.nget()) : TriState::Z);
	}

	void LengthCounter::sim_Decoder()
	{
		// The original ROM encodes the reload count minus one.
		reload = LengthReload[apu->DB >> 3];
	}

	void LengthCounter::sim_Counter(TriState LC_CarryIn, TriState WriteEn)
	{
		cnt.sim(apu->wire.RES, WriteEn, STEP, apu->wire.ACLK1, reload, LC_CarryIn == TriState::One);
		carry_out = cnt.get_carry(LC_CarryIn == TriState::One);
	}

#pragma region "Debug"

	uint8_t LengthCounter::Debug_GetCnt() { return uint8_t(cnt.get()); }
	void LengthCounter::Debug_SetCnt(uint8_t value) { cnt.set(value); }

	bool LengthCounter::Debug_GetEnable()
	{
		return reg_enable_latch.get() == TriState::One;
	}

	void LengthCounter::Debug_SetEnable(bool enable)
	{
		reg_enable_latch.set(enable ? TriState::One : TriState::Zero, TriState::One);
	}

#pragma endregion "Debug"
}
