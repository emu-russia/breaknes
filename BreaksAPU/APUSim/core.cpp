// 6502 Core Binding

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	CoreBinding::CoreBinding(APU* parent)
	{
		apu = parent;
	}
	
	CoreBinding::~CoreBinding()
	{
	}

	void CoreBinding::sim()
	{
		sim_DividerBeforeCore();

		if (apu->wire.PHI0 != apu->PrevPHI_Core)
		{
			TriState inputs[(size_t)M6502Core::InputPad::Max]{};
			TriState outputs[(size_t)M6502Core::OutputPad::Max];

			TriState nIRQINT = NOR(NOT(apu->wire.n_IRQ), apu->wire.INT);

			inputs[(size_t)M6502Core::InputPad::n_NMI] = apu->wire.n_NMI;
			inputs[(size_t)M6502Core::InputPad::n_IRQ] = nIRQINT;
			inputs[(size_t)M6502Core::InputPad::n_RES] = NOT(OR(apu->wire.RES, apu->wire.RESCore));
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

		// Phase splitter

		CLK_FF.set( NOR(NOR(NOT(n_CLK), CLK_FF.get()), NOT(NOT(n_CLK))) );
		TriState q = CLK_FF.get();
		TriState nq = CLK_FF.nget();

		// Jhonson counter (back propagate)

		div[5].sim(q, nq, TriState::Zero, div[4].get_sout(TriState::Zero));
		div[4].sim(q, nq, TriState::Zero, div[3].get_sout(TriState::Zero));

		TriState new_phi = NOT(div[5].get_sout(TriState::Zero));
		apu->wire.PHI0 = new_phi;
		TriState rst = NOR(apu->wire.PHI0, div[4].get_sout(TriState::Zero));

		div[3].sim(q, nq, rst, div[2].get_sout(rst));
		div[2].sim(q, nq, rst, div[1].get_sout(rst));
		div[1].sim(q, nq, rst, div[0].get_sout(rst));
		div[0].sim(q, nq, rst, apu->wire.PHI0);

		// TBD: Other APU revisions

		// The software PHI counter is triggered by the falling edge.
		// This is purely a software design for convenience, and has nothing to do with APU hardware circuitry.

		if (IsNegedge(prev_phi, new_phi))
		{
			apu->phi_counter++;
		}
	}

	void CoreBinding::sim_DividerAfterCore()
	{
		apu->wire.n_M2 = NOR(div[4].get_nval(), apu->wire.PHI2);

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
