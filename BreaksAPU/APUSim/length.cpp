// Length Counters

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	LengthCounter::LengthCounter(APU* parent)
	{
		apu = parent;
	}

	LengthCounter::~LengthCounter()
	{
	}

	void LengthCounter::sim(size_t bit_ena, TriState WriteEn, TriState LC_CarryIn, TriState& LC_NoCount)
	{
		sim_Decoder1();
		sim_Decoder2();
		sim_Counter(LC_CarryIn, WriteEn);
		sim_Control(bit_ena, WriteEn, LC_NoCount);
	}

	void LengthCounter::sim_Control(size_t bit_ena, TriState WriteEn, TriState& LC_NoCount)
	{
		TriState ACLK = apu->wire.ACLK;
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;
		TriState n_LFO2 = apu->wire.n_LFO2;
		TriState W4015 = apu->wire.W4015;
		TriState n_R4015 = apu->wire.n_R4015;
		TriState n_ACLK4 = NOT(ACLK);
		TriState LCDIS;

		reg_enable_latch.set( 
			MUX(n_ACLK, 
				MUX(W4015, 
					MUX(RES, TriState::Z, TriState::Zero),
					apu->GetDBBit(bit_ena)),
				NOT(reg_enable_latch.nget())), TriState::One);
		LCDIS = reg_enable_latch.nget();

		ena_latch.set(LCDIS, n_ACLK);
		cout_latch.set(carry_out, n_ACLK);

		STEP = NOR(step_latch.get(), n_LFO2);
		step_latch.set(stat_ff.nget(), n_ACLK);

		stat_ff.set(NOR4(AND(ena_latch.get(), n_ACLK4), AND(cout_latch.get(), STEP), RES, NOR(stat_ff.get(), WriteEn)));

		LC_NoCount = NOT(stat_ff.get());
		apu->SetDBBit(bit_ena, NOT(n_R4015) == TriState::One ? NOT(stat_ff.nget()) : TriState::Z);
	}

	void LengthCounter::sim_Decoder1()
	{
		TriState d[5]{};
		TriState nd[5]{};

		for (size_t n = 0; n < 5; n++)
		{
			dec_latch[n].set(apu->GetDBBit(3 + n), TriState::One);
			d[n] = dec_latch[n].get();
			nd[n] = dec_latch[n].nget();
		}

		Dec1_out[0] = NOR5 (d[0], d[1], d[2], d[3], d[4]);
		Dec1_out[1] = NOR5 (d[0], nd[1], d[2], d[3], d[4]);
		Dec1_out[2] = NOR5 (d[0], d[1], nd[2], d[3], d[4]);
		Dec1_out[3] = NOR5 (d[0], nd[1], nd[2], d[3], d[4]);
		Dec1_out[4] = NOR5 (d[0], d[1], d[2], nd[3], d[4]);
		Dec1_out[5] = NOR5 (d[0], nd[1], d[2], nd[3], d[4]);
		Dec1_out[6] = NOR5 (d[0], d[1], nd[2], nd[3], d[4]);
		Dec1_out[7] = NOR5 (d[0], nd[1], nd[2], nd[3], d[4]);

		Dec1_out[8] = NOR5 (d[0], d[1], d[2], d[3], nd[4]);
		Dec1_out[9] = NOR5 (d[0], nd[1], d[2], d[3], nd[4]);
		Dec1_out[10] = NOR5 (d[0], d[1], nd[2], d[3], nd[4]);
		Dec1_out[11] = NOR5 (d[0], nd[1], nd[2], d[3], nd[4]);
		Dec1_out[12] = NOR5 (d[0], d[1], d[2], nd[3], nd[4]);
		Dec1_out[13] = NOR5 (d[0], nd[1], d[2], nd[3], nd[4]);
		Dec1_out[14] = NOR5 (d[0], d[1], nd[2], nd[3], nd[4]);
		Dec1_out[15] = NOR5 (d[0], nd[1], nd[2], nd[3], nd[4]);

		Dec1_out[16] = NOR5 (nd[0], d[1], d[2], d[3], d[4]);
		Dec1_out[17] = NOR5 (nd[0], nd[1], d[2], d[3], d[4]);
		Dec1_out[18] = NOR5 (nd[0], d[1], nd[2], d[3], d[4]);
		Dec1_out[19] = NOR5 (nd[0], nd[1], nd[2], d[3], d[4]);
		Dec1_out[20] = NOR5 (nd[0], d[1], d[2], nd[3], d[4]);
		Dec1_out[21] = NOR5 (nd[0], nd[1], d[2], nd[3], d[4]);
		Dec1_out[22] = NOR5 (nd[0], d[1], nd[2], nd[3], d[4]);
		Dec1_out[23] = NOR5 (nd[0], nd[1], nd[2], nd[3], d[4]);

		Dec1_out[24] = NOR5 (nd[0], d[1], d[2], d[3], nd[4]);
		Dec1_out[25] = NOR5 (nd[0], nd[1], d[2], d[3], nd[4]);
		Dec1_out[26] = NOR5 (nd[0], d[1], nd[2], d[3], nd[4]);
		Dec1_out[27] = NOR5 (nd[0], nd[1], nd[2], d[3], nd[4]);
		Dec1_out[28] = NOR5 (nd[0], d[1], d[2], nd[3], nd[4]);
		Dec1_out[29] = NOR5 (nd[0], nd[1], d[2], nd[3], nd[4]);
		Dec1_out[30] = NOR5 (nd[0], d[1], nd[2], nd[3], nd[4]);
		Dec1_out[31] = NOR5 (nd[0], nd[1], nd[2], nd[3], nd[4]);
	}

	void LengthCounter::sim_Decoder2()
	{
		TriState* d = Dec1_out;

		LC[7] = NOR29 (
			d[0], d[1], d[2], d[3], d[5], d[6], d[7],
			d[8], d[9], d[10], d[11], d[13], d[14], d[15],
			d[17], d[18], d[19], d[20], d[21], d[22], d[23],
			d[24], d[25], d[26], d[27], d[28], d[29], d[30], d[31] );
		LC[6] = NOR28 (
			d[0], d[1], d[2], d[4], d[5], d[6], d[7], 
			d[8], d[9], d[10], d[12], d[14], d[15], 
			d[17], d[18], d[19], d[20], d[21], d[22], d[23], 
			d[24], d[25], d[26], d[27], d[28], d[29], d[30], d[31] );
		LC[5] = NOR27 (
			d[0], d[1], d[3], d[4], d[6], d[7], 
			d[8], d[9], d[11], d[13], d[14], d[15], 
			d[17], d[18], d[19], d[20], d[21], d[22], d[23], 
			d[24], d[25], d[26], d[27], d[28], d[29], d[30], d[31] );
		LC[4] = NOR16 (
			d[0], d[2], d[3], d[6], 
			d[8], d[10], d[13], d[14], 
			d[17], d[18], d[19], d[20], d[21], d[22], d[23],
			d[24] );
		LC[3] = NOR12 (
			d[1], d[2], 
			d[9], d[13], 
			d[17], d[18], d[19], d[20], 
			d[25], d[26], d[27], d[28] );
		LC[2] = NOR13 (
			d[0], d[1], d[5], d[7], 
			d[8], 
			d[17], d[18], d[21], d[22], 
			d[25], d[26], d[29], d[30] );
		LC[1] = NOR12 (
			d[0], d[6], d[7], 
			//...
			d[16], d[17], d[19], d[21], d[23], 
			d[25], d[27], d[29], d[31] );
		LC[0] = TriState::One;
	}

	void LengthCounter::sim_Counter(TriState LC_CarryIn, TriState WriteEn)
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;
		TriState carry = LC_CarryIn;

		for (size_t n = 0; n < 8; n++)
		{
			carry = cnt[n].sim(carry, RES, WriteEn, STEP, n_ACLK, LC[n]);
		}

		carry_out = carry;
	}

	uint8_t LengthCounter::Debug_GetCnt()
	{
		TriState cnt_out[8]{};

		for (size_t n = 0; n < 8; n++)
		{
			cnt_out[n] = cnt[n].get();
		}

		return Pack(cnt_out);
	}
}
