// 6502 Core Binding

#include "pch.h"

using namespace BaseLogic;

namespace FastAPU
{
	CoreBinding::CoreBinding(FastAPU* parent)
	{
		apu = parent;
	}

	CoreBinding::~CoreBinding()
	{
	}

	void CoreBinding::sim()
	{
		sim_DividerBeforeCore();

		// The 6502 core uses TriState from BaseLogicLib, so you have to convert the values

		if (apu->wire.PHI0 != apu->PrevPHI_Core)
		{
			TriState inputs[(size_t)M6502Core::InputPad::Max]{};
			TriState outputs[(size_t)M6502Core::OutputPad::Max];

			auto IRQINT = (!apu->wire.n_IRQ || apu->wire.INT);

			inputs[(size_t)M6502Core::InputPad::n_NMI] = apu->wire.n_NMI ? TriState::One : TriState::Zero;
			inputs[(size_t)M6502Core::InputPad::n_IRQ] = IRQINT ? TriState::Zero : TriState::One;
			inputs[(size_t)M6502Core::InputPad::n_RES] = apu->wire.RES ? TriState::Zero : TriState::One;
			inputs[(size_t)M6502Core::InputPad::PHI0] = apu->wire.PHI0 ? TriState::One : TriState::Zero;
			inputs[(size_t)M6502Core::InputPad::RDY] = (apu->wire.RDY && apu->wire.RDY2) ? TriState::One : TriState::Zero;
			inputs[(size_t)M6502Core::InputPad::SO] = TriState::One;

			apu->core->sim(inputs, outputs, &apu->CPU_Addr, &apu->DB);

			apu->wire.PHI1 = outputs[(size_t)M6502Core::OutputPad::PHI1] == TriState::One ? 1 : 0;
			apu->wire.PHI2 = outputs[(size_t)M6502Core::OutputPad::PHI2] == TriState::One ? 1 : 0;
			apu->wire.RnW = outputs[(size_t)M6502Core::OutputPad::RnW] == TriState::One ? 1 : 0;
			apu->wire.SYNC = outputs[(size_t)M6502Core::OutputPad::SYNC] == TriState::One ? 1 : 0;

			apu->PrevPHI_Core = apu->wire.PHI0;
		}

		sim_DividerAfterCore();
	}

	void CoreBinding::sim_DividerBeforeCore()
	{

	}

	void CoreBinding::sim_DividerAfterCore()
	{

	}
}
