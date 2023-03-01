// A dummy device that uses the 6502 as a processor.
// Contains 64 Kbytes of memory and nothing else.

#include "pch.h"

using namespace BaseLogic;

namespace Breaknes
{
	BogusBoard::BogusBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev) : Board (apu_rev, ppu_rev)
	{
		core = new M6502Core::M6502(false, false);
        wram = new BaseBoard::SRAM(wram_bits);

        for (int i = 0; i < wram->Dbg_GetSize(); i++)
        {
            wram->Dbg_WriteByte(i, 0);
        }

        AddBoardMemDescriptors();
        AddDebugInfoProviders();
	}

	BogusBoard::~BogusBoard()
	{
		delete core;
        delete wram;
	}

	void BogusBoard::Step()
	{
        uint32_t wram_addr;
        bool dz = false;

        // Memory (Read)

        if (RnW == TriState::One)
        {
            wram_addr = addr_bus;
            wram->sim(TriState::Zero, RnW, NOT(RnW), &wram_addr, &data_bus, dz);
        }

        // Core

        TriState inputs[(size_t)M6502Core::InputPad::Max]{};
        TriState outputs[(size_t)M6502Core::OutputPad::Max];

        inputs[(size_t)M6502Core::InputPad::n_NMI] = n_NMI;
        inputs[(size_t)M6502Core::InputPad::n_IRQ] = n_IRQ;
        inputs[(size_t)M6502Core::InputPad::n_RES] = n_RES;
        inputs[(size_t)M6502Core::InputPad::PHI0] = CLK;
        inputs[(size_t)M6502Core::InputPad::RDY] = RDY;
        inputs[(size_t)M6502Core::InputPad::SO] = SO;

        core->sim(inputs, outputs, &addr_bus, &data_bus);

        PHI1 = outputs[(size_t)M6502Core::OutputPad::PHI1];
        PHI2 = outputs[(size_t)M6502Core::OutputPad::PHI2];
        RnW = outputs[(size_t)M6502Core::OutputPad::RnW];
        SYNC = outputs[(size_t)M6502Core::OutputPad::SYNC];

        // Memory (Write)

        if (RnW == TriState::Zero)
        {
            wram_addr = addr_bus;
            wram->sim(TriState::Zero, RnW, NOT(RnW), &wram_addr, &data_bus, dz);
        }

        CLK = NOT(CLK);

        if (IsNegedge(PrevCLK, CLK))
        {
            phi_counter++;
        }
        PrevCLK = CLK;
	}

    size_t BogusBoard::GetPHICounter()
    {
        return phi_counter;
    }
}
