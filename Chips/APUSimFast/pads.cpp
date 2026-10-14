// Simulation of APU chip terminals and everything related to them.

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
{
	Pads::Pads(APU* parent)
	{
		apu = parent;
	}

	Pads::~Pads()
	{
	}

	void Pads::sim_InputPads(TriState inputs[])
	{
		apu->wire.n_CLK = NOT(inputs[(size_t)APU_Input::CLK]);
		apu->wire.DBG = inputs[(size_t)APU_Input::DBG];
		apu->wire.RES = NOT(inputs[(size_t)APU_Input::n_RES]);
		
		// In the original #NMI and #IRQ terminals overuse the BIDIR terminal circuit.
		// In order to avoid all sorts of gimmicks, let's just do a pass-through.

		apu->wire.n_NMI = inputs[(size_t)APU_Input::n_NMI];
		//n_nmi.sim(inputs[(size_t)APU_Input::n_NMI], TriState::One,
		//	apu->wire.n_NMI, unused, TriState::One, TriState::Zero);

		apu->wire.n_IRQ = inputs[(size_t)APU_Input::n_IRQ];
		//n_irq.sim(inputs[(size_t)APU_Input::n_IRQ], TriState::One,
		//	apu->wire.n_IRQ, unused, TriState::One, TriState::Zero);

		if (apu->wire.RES != prev_RES)
		{
			LOG_APU(Cat_Events, "RESET %s", apu->wire.RES == TriState::One ? "asserted" : "deasserted");
			prev_RES = apu->wire.RES;
		}
	}

	void Pads::sim_OutputPads(TriState outputs[], uint16_t* addr)
	{
		TriState NotDBG_RES = NOR(apu->wire.DBG, NOT(apu->wire.RES));
		outputs[(size_t)APU_Output::M2] = NOT(NotDBG_RES) == TriState::One ? NOR(apu->wire.n_M2, NotDBG_RES) : TriState::Z;

		if (apu->wire.RES == TriState::Zero)
		{
			*addr = apu->Ax;
		}

		outputs[(size_t)APU_Output::RnW] = NOT(apu->wire.RES) == TriState::One ? NOT(NOR(apu->wire.RW, apu->wire.RES)) : TriState::Z;

		// I/O

		sim_OutReg();

		n_in[0].sim(unused, apu->wire.n_R4016, unused, outputs[(size_t)APU_Output::n_IN0], apu->wire.RES, TriState::One);
		n_in[1].sim(unused, apu->wire.n_R4017, unused, outputs[(size_t)APU_Output::n_IN1], apu->wire.RES, TriState::One);

		out[0].sim(unused, OUT_Signal[0], unused, outputs[(size_t)APU_Output::OUT_0], apu->wire.RES, TriState::One);
		out[1].sim(unused, OUT_Signal[1], unused, outputs[(size_t)APU_Output::OUT_1], apu->wire.RES, TriState::One);
		out[2].sim(unused, OUT_Signal[2], unused, outputs[(size_t)APU_Output::OUT_2], apu->wire.RES, TriState::One);
	}

	/// <summary>
	/// External data bus -> Internal data bus
	/// </summary>
	/// <param name="data">External data bus</param>
	void Pads::sim_DataBusInput(uint8_t* data)
	{
		apu->DB_Dirty = apu->wire.RD == TriState::One;
		if (apu->DB_Dirty) apu->DB = input_byte = *data;
	}

	/// <summary>
	/// Internal data bus -> External data bus
	/// </summary>
	/// <param name="data">External data bus</param>
	void Pads::sim_DataBusOutput(uint8_t* data)
	{
		if (apu->wire.WR == TriState::One) *data = output_byte = apu->DB;
	}

	void Pads::sim_OutReg()
	{
		TriState nACLK2 = apu->wire.nACLK2;
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4016 = apu->wire.W4016;
		TriState ACLK5 = NOT(nACLK2);

		for (size_t n = 0; n < 3; n++)
		{
			out_reg[n].sim(ACLK1, W4016, apu->GetDBBit(n));
			out_latch[n].set(out_reg[n].get(), ACLK5);
			OUT_Signal[n] = NOT(out_latch[n].nget());
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
		else
		{
			from_pad = TriState::Z;
		}

		if (NOT(rd) == TriState::One)
		{
			pad_out = NOT(NOR(out_latch.get(), rd));
		}
		else
		{
			pad_out = TriState::Z;
		}
	}

	void BIDIR::sim_Input(TriState pad_in, TriState& from_pad, TriState rd)
	{
		in_latch.set(pad_in, TriState::One);

		if (rd == TriState::One)
		{
			from_pad = NOT(in_latch.nget());
		}
	}

	void BIDIR::sim_Output(TriState to_pad, TriState& pad_out, TriState wr)
	{
		out_latch.set(to_pad, TriState::One);

		if (wr == TriState::One)
		{
			pad_out = out_latch.get();
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

	void BIDIR::set_in(TriState val)
	{
		in_latch.set(val, TriState::One);
	}

	void BIDIR::set_out(TriState val)
	{
		out_latch.set(val, TriState::One);
	}

#pragma region "Debug"

	uint32_t Pads::Get_DBOutputLatch()
	{
		return output_byte;
	}

	uint32_t Pads::Get_DBInputLatch()
	{
		return input_byte;
	}

	uint32_t Pads::Get_OutReg()
	{
		TriState val[4]{};
		for (size_t n = 0; n < 3; n++)
		{
			val[n] = out_latch[n].get();
		}
		val[3] = TriState::Zero;
		return PackNibble(val);
	}

	void Pads::Set_DBOutputLatch(uint32_t value)
	{
		output_byte = uint8_t(value);
	}

	void Pads::Set_DBInputLatch(uint32_t value)
	{
		input_byte = uint8_t(value);
	}

	void Pads::Set_OutReg(uint32_t value)
	{
		TriState val[4]{};
		UnpackNibble(value, val);
		for (size_t n = 0; n < 3; n++)
		{
			out_latch[n].set(val[n], TriState::One);
		}
	}

#pragma endregion "Debug"
}
