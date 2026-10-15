// 6502 Core Binding

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
{
	CoreBinding::CoreBinding(APU* parent)
	{
		apu = parent;
	}

	CoreBinding::~CoreBinding()
	{
	}

	bool CoreBinding::PeekPhi0Edge() const
	{
		const uint8_t next_output = apu->wire.n_CLK == TriState::Zero ? divider_output : divider_input;
		const TriState new_phi = (next_output & 0x20) != 0 ? TriState::Zero : TriState::One;
		return new_phi != apu->PrevPHI_Core;
	}

	void CoreBinding::sim()
	{
		core_called = false;
		sim_DividerBeforeCore();

		if (apu->wire.PHI0 != apu->PrevPHI_Core)
		{
			core_called = true;
			TriState inputs[(size_t)M6502Core::InputPad::Max]{};
			TriState outputs[(size_t)M6502Core::OutputPad::Max];

			TriState nIRQINT = NOR(NOT(apu->wire.n_IRQ), apu->wire.INT);

			inputs[(size_t)M6502Core::InputPad::n_NMI] = apu->wire.n_NMI;
			inputs[(size_t)M6502Core::InputPad::n_IRQ] = nIRQINT;
			inputs[(size_t)M6502Core::InputPad::n_RES] = NOT(apu->wire.RES);
			inputs[(size_t)M6502Core::InputPad::PHI0] = apu->wire.PHI0;
			inputs[(size_t)M6502Core::InputPad::RDY] = AND(apu->wire.RDY, apu->wire.RDY2);
			inputs[(size_t)M6502Core::InputPad::SO] = TriState::One;

			apu->core->sim(inputs, outputs, &apu->CPU_Addr, &apu->DB);

			apu->wire.PHI1 = outputs[(size_t)M6502Core::OutputPad::PHI1];
			apu->wire.PHI2 = outputs[(size_t)M6502Core::OutputPad::PHI2];
			apu->wire.RnW = outputs[(size_t)M6502Core::OutputPad::RnW];
			apu->wire.SYNC = outputs[(size_t)M6502Core::OutputPad::SYNC];

			apu->PrevPHI_Core = apu->wire.PHI0;
		}

		sim_DividerAfterCore();
	}

	void CoreBinding::sim_DividerBeforeCore()
	{
		TriState n_CLK = apu->wire.n_CLK;
		TriState prev_phi = apu->wire.PHI0;

		// The phase splitter is exactly !n_CLK for binary physical clocks.
		// Preserve both latch banks; they update on opposite master phases.
		if (n_CLK == TriState::Zero)
		{
			const bool reset = (divider_output & 0x20) != 0 && (divider_output & 0x10) == 0;
			divider_input = uint8_t(((divider_output << 1) & 0x3f) | ((divider_output & 0x20) == 0));
			if (reset) divider_input &= uint8_t(~0x0e);
		}
		else
		{
			divider_output = divider_input;
		}
		const TriState new_phi = (divider_output & 0x20) != 0 ? TriState::Zero : TriState::One;
		apu->wire.PHI0 = new_phi;

		// TBD: Other APU revisions

		// The software PHI counter is triggered by the falling edge.
		// This is purely a software design for convenience, and has nothing to do with APU hardware circuitry.
		// CPU cycles count by falling edge means that it is a counter of FULL cycles (i.e. PHI1+PHI2 of the processor are executed)

		if (IsNegedge(prev_phi, new_phi))
		{
			apu->phi_counter++;
		}
	}

	void CoreBinding::sim_DividerAfterCore()
	{
		apu->wire.n_M2 = NOR((divider_input & 0x10) != 0 ? TriState::Zero : TriState::One, apu->wire.PHI2);

		// TBD: Other APU revisions
	}

	void DIV_SRBit::sim(TriState q, TriState nq, TriState rst, TriState sin)
	{
		in_latch.set(sin, q);
		out_latch.set(in_latch.nget(), nq);
	}

	TriState DIV_SRBit::get_sout(TriState rst)
	{
		return NOR(out_latch.get(), rst);
	}

	TriState DIV_SRBit::get_nval()
	{
		return in_latch.nget();
	}
}
