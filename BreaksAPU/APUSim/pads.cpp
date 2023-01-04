// Simulation of APU chip terminals and everything related to them.

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	Pads::Pads(APU* parent)
	{
		apu = parent;
	}

	Pads::~Pads()
	{
	}

	void Pads::sim_InputPads(TriState inputs[], uint8_t* data)
	{
		apu->wire.n_CLK = NOT(inputs[(size_t)APU_Input::CLK]);
		apu->wire.DBG = inputs[(size_t)APU_Input::DBG];
		apu->wire.RES = NOT(inputs[(size_t)APU_Input::n_RES]);
		
		n_nmi.sim(inputs[(size_t)APU_Input::n_NMI], TriState::One,
			apu->wire.n_NMI, unused, TriState::One, TriState::Zero);

		n_irq.sim(inputs[(size_t)APU_Input::n_IRQ], TriState::One,
			apu->wire.n_IRQ, unused, TriState::One, TriState::Zero);
	}

	void Pads::sim_OutputPads(TriState outputs[], uint8_t* data, uint16_t* addr)
	{
		TriState NotDBG_RES = NOR(apu->wire.DBG, NOT(apu->wire.RES));
		outputs[(size_t)APU_Output::M2] = NOT(NotDBG_RES) == TriState::One ? NOR(apu->wire.n_M2, NotDBG_RES) : TriState::Z;

		if (apu->wire.RES == TriState::Zero)
		{
			*addr = apu->Ax;
		}

		outputs[(size_t)APU_Output::RnW] = NOT(apu->wire.RES) == TriState::One ? NOR(apu->wire.RW, apu->wire.RES) : TriState::Z;

		// I/O

		sim_OutReg();

		n_in[0].sim(unused, apu->wire.n_R4016, unused, outputs[(size_t)APU_Output::n_IN0], apu->wire.RES, TriState::One);
		n_in[1].sim(unused, apu->wire.n_R4017, unused, outputs[(size_t)APU_Output::n_IN1], apu->wire.RES, TriState::One);

		out[0].sim(unused, OUT[0], unused, outputs[(size_t)APU_Output::OUT_0], apu->wire.RES, TriState::One);
		out[1].sim(unused, OUT[1], unused, outputs[(size_t)APU_Output::OUT_1], apu->wire.RES, TriState::One);
		out[2].sim(unused, OUT[2], unused, outputs[(size_t)APU_Output::OUT_2], apu->wire.RES, TriState::One);
	}

	void Pads::sim_OutReg()
	{
		TriState ACLK = apu->wire.ACLK;
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W4016 = apu->wire.W4016;
		TriState n_ACLK5 = NOT(ACLK);

		for (size_t n = 0; n < 3; n++)
		{
			out_reg[n].sim(n_ACLK, W4016, apu->GetDBBit(n));
			out_latch[n].set(out_reg[n].get(), n_ACLK5);
			OUT[n] = NOT(out_latch[n].nget());
		}
	}

	void BIDIR::sim(TriState pad_in, TriState to_pad,
		TriState& from_pad, TriState& pad_out,
		TriState rd, TriState wr)
	{
		in_latch.set(pad_in, TriState::One);
		out_latch.set(to_pad, TriState::One);

		if (NOT(wr) == TriState::One)
		{
			from_pad = NOT(in_latch.nget());
		}

		if (NOT(rd) == TriState::One)
		{
			pad_out = NOT(NOR(out_latch.get(), rd));
		}
	}

	TriState BIDIR::get_in()
	{
		return in_latch.get();
	}

	TriState BIDIR::get_out()
	{
		return out_latch.get();
	}

	void Pads::Debug_Get(APU_Registers* info)
	{
		TriState val[8]{};

		for (size_t n = 0; n < 8; n++)
		{
			val[n] = data_bus[n].get_in();
		}
		info->DBInputLatch = Pack(val);

		for (size_t n = 0; n < 8; n++)
		{
			val[n] = data_bus[n].get_out();
		}
		info->DBOutputLatch = Pack(val);

		for (size_t n = 0; n < 3; n++)
		{
			val[n] = out_latch[n].get();
		}
		val[3] = TriState::Zero;
		info->OutReg = PackNibble(val);
	}
}
