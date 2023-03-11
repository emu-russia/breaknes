// Simulation of APU chip terminals and everything related to them.

#include "pch.h"

using namespace BaseLogic;

//#define PAD_LOG(...) printf(__VA_ARGS__)
#define PAD_LOG(...)

namespace APUSim
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

		// Extra

		outputs[(size_t)APU_Output::SYNC] = apu->wire.SYNC;
	}

	/// <summary>
	/// External data bus -> Internal data bus
	/// </summary>
	/// <param name="data">External data bus</param>
	void Pads::sim_DataBusInput(uint8_t* data)
	{
		TriState val[8]{};

		if (apu->wire.RD != TriState::One)
		{
			PAD_LOG("DMABuffer in WriteMode. Skipping sim_DataBusInput\n");
			apu->DB_Dirty = false;
			return;
		}

		for (size_t n = 0; n < 8; n++)
		{
			TriState db_bit = ((*data) >> n) & 1 ? TriState::One : TriState::Zero;
			data_bus[n].sim_Input(db_bit, val[n], apu->wire.RD);
		}

		apu->DB = Pack(val);
		apu->DB_Dirty = true;
		PAD_LOG("External databus: %02X => Internal DB: %02X\n", *data, apu->DB);
	}

	/// <summary>
	/// Internal data bus -> External data bus
	/// </summary>
	/// <param name="data">External data bus</param>
	void Pads::sim_DataBusOutput(uint8_t* data)
	{
		TriState val[8]{};

		if (apu->wire.WR != TriState::One)
		{
			PAD_LOG("DMABuffer in ReadMode. Skipping sim_DataBusOutput\n");
			return;
		}

		for (size_t n = 0; n < 8; n++)
		{
			data_bus[n].sim_Output(apu->GetDBBit(n), val[n], apu->wire.WR);
;		}

		*data = Pack(val);
		PAD_LOG("Internal DB: %02X => External databus: %02X\n", apu->DB, *data);
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

		if (NOT(rd) == TriState::One)
		{
			pad_out = NOT(NOR(out_latch.get(), rd));
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
		TriState val[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val[n] = data_bus[n].get_out();
		}
		return Pack(val);
	}

	uint32_t Pads::Get_DBInputLatch()
	{
		TriState val[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val[n] = data_bus[n].get_in();
		}
		return Pack(val);
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
		TriState val[8]{};
		Unpack(value, val);
		for (size_t n = 0; n < 8; n++)
		{
			data_bus[n].set_out(val[n]);
		}
	}

	void Pads::Set_DBInputLatch(uint32_t value)
	{
		TriState val[8]{};
		Unpack(value, val);
		for (size_t n = 0; n < 8; n++)
		{
			data_bus[n].set_in(val[n]);
		}
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
