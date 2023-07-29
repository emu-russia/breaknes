// A special version of the board, which contains a bare APU and Fake6502, which can only read/write APU registers from the dump.

#include "pch.h"

using namespace BaseLogic;

namespace Breaknes
{
	APUPlayerBoard::APUPlayerBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, Mappers::ConnectorType p1) : Board(apu_rev, ppu_rev, p1)
	{
		core = new M6502Core::FakeM6502("APU", MappedAPUBase, MappedAPUMask);
		apu = new APUSim::APU(core, apu_rev);
		wram = new BaseBoard::SRAM("WRAM", wram_bits);

		for (int i = 0; i < wram->Dbg_GetSize(); i++)
		{
			wram->Dbg_WriteByte(i, 0);
		}

		apu->SetNormalizedOutput(true);

		AddBoardMemDescriptors();
		AddDebugInfoProviders();
	}

	APUPlayerBoard::~APUPlayerBoard()
	{
		delete apu;
		delete core;
		delete wram;
	}

	void APUPlayerBoard::Step()
	{
		// Simulate APU

		TriState inputs[(size_t)APUSim::APU_Input::Max]{};
		TriState outputs[(size_t)APUSim::APU_Output::Max]{};

		inputs[(size_t)APUSim::APU_Input::CLK] = CLK;

		inputs[(size_t)APUSim::APU_Input::n_NMI] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_IRQ] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_RES] = in_reset ? TriState::Zero : TriState::One;
		inputs[(size_t)APUSim::APU_Input::DBG] = TriState::Zero;

		apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);

		TriState RnW = outputs[(size_t)APUSim::APU_Output::RnW];
		TriState M2 = outputs[(size_t)APUSim::APU_Output::M2];		// There doesn't seem to be any use for it...

		if (RnW == TriState::Z)
		{
			RnW = TriState::One;
		}

		TriState wram_cs = (addr_bus < 0x800 || addr_bus >= 0x8000) == 0 ? TriState::One : TriState::Zero;

		TriState n_WE = RnW;
		TriState n_OE = NOT(RnW);
		uint32_t wram_addr = addr_bus;
		bool dz = false;
		wram->sim(NOT(wram_cs), n_WE, n_OE, &wram_addr, &data_bus, dz);

		// Tick

		CLK = NOT(CLK);
	}

	void APUPlayerBoard::Reset()
	{
		// Execute 1 cycle (not a core cycle, just a cycle). This is enough to reset APU circuits.

		in_reset = true;
		Step();
		Step();
		in_reset = false;
	}

	bool APUPlayerBoard::InResetState()
	{
		return false;
	}

	void APUPlayerBoard::LoadRegDump(uint8_t* data, size_t data_size)
	{
		if (core != nullptr)
		{
			core->SetRegDump(data, data_size);
		}
	}
}
