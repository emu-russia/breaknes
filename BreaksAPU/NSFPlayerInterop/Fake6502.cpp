// 6502, which can only simulate reading and writing APU registers from the dump history

#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	FakeM6502::FakeM6502() : M6502()
	{
		rp = new NSFPlayer::RegDumpProcessor();
	}

	FakeM6502::~FakeM6502()
	{
		delete rp;
	}

	void FakeM6502::sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], uint16_t* addr_bus, uint8_t* data_bus)
	{
		TriState PHI0 = inputs[(size_t)M6502Core::InputPad::PHI0];
		TriState n_RES = inputs[(size_t)M6502Core::InputPad::n_RES];

		TriState PHI1 = NOT(PHI0);
		TriState PHI2 = PHI0;

		// Default core mode (read address 0)

		TriState RnW = TriState::One;
		*addr_bus = 0;

		rp->sim(PHI0, n_RES, RnW, addr_bus, data_bus);

		// Return values of the terminals

		outputs[(size_t)M6502Core::OutputPad::PHI1] = PHI1;
		outputs[(size_t)M6502Core::OutputPad::PHI2] = PHI2;
		outputs[(size_t)M6502Core::OutputPad::RnW] = RnW;
		outputs[(size_t)M6502Core::OutputPad::SYNC] = TriState::Zero;
	}
}
