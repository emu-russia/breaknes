// Module for maintaining a simulated APU environment.

#include "pch.h"

using namespace BaseLogic;

namespace NSFPlayer
{
	NSFPlayerBoard::NSFPlayerBoard(char* boardName, char* apuRev, char* ppuRev, char* p1) : Board(boardName, apuRev, ppuRev, p1)
	{
		APUSim::Revision rev;

		if (!strcmp(apuRev, "RP2A03G"))
		{
			rev = APUSim::Revision::RP2A03G;
		}
		else if (!strcmp(apuRev, "RP2A03H"))
		{
			rev = APUSim::Revision::RP2A03H;
		}
		else if (!strcmp(apuRev, "RP2A07"))
		{
			rev = APUSim::Revision::RP2A07;
		}
		else if (!strcmp(apuRev, "UA6527P"))
		{
			rev = APUSim::Revision::UA6527P;
		}
		else if (!strcmp(apuRev, "TA03NP1"))
		{
			rev = APUSim::Revision::TA03NP1;
		}
		else
		{
			// If someone passed on an obscure APU revision, stay with the default reference RP2A03G, as the most studied.
			rev = APUSim::Revision::RP2A03G;
		}

		core = new M6502Core::M6502(true, true);
		apu = new APUSim::APU(core, rev);
		sram = new NSFMapper();
		wram = new BaseBoard::SRAM(wram_bits);

		for (int i = 0; i < wram->Dbg_GetSize(); i++)
		{
			wram->Dbg_WriteByte(i, 0);
		}

		apu->SetNormalizedOutput(true);

		AddBoardMemDescriptors();
		AddDebugInfoProviders();
	}

	NSFPlayerBoard::~NSFPlayerBoard()
	{
		delete apu;
		delete core;
		delete sram;
		delete wram;
	}

	void NSFPlayerBoard::Step()
	{
		// Simulate APU

		TriState inputs[(size_t)APUSim::APU_Input::Max]{};
		TriState outputs[(size_t)APUSim::APU_Output::Max]{};

		inputs[(size_t)APUSim::APU_Input::CLK] = CLK;

		inputs[(size_t)APUSim::APU_Input::n_NMI] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_IRQ] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_RES] = (pendingReset && reset_apu_also) ? TriState::Zero : TriState::One;
		inputs[(size_t)APUSim::APU_Input::DBG] = TriState::Zero;

		apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);

		SYNC = outputs[(size_t)APUSim::APU_Output::SYNC];

		TriState RnW = outputs[(size_t)APUSim::APU_Output::RnW];
		TriState M2 = outputs[(size_t)APUSim::APU_Output::M2];		// There doesn't seem to be any use for it...

		if (RnW == TriState::Z)
		{
			RnW = TriState::One;
		}

		TriState wram_cs = (addr_bus >> wram_bits) == 0 ? TriState::One : TriState::Zero;

		TriState n_WE = RnW;
		TriState n_OE = NOT(RnW);
		uint32_t wram_addr = addr_bus;
		bool dz = false;
		wram->sim(NOT(wram_cs), n_WE, n_OE, &wram_addr, &data_bus, dz);

		sram->sim(RnW, NOT(wram_cs), addr_bus, &data_bus);

		// Tick

		CLK = NOT(CLK);

		if (pendingReset)
		{
			resetHalfClkCounter--;
			if (resetHalfClkCounter == 0)
			{
				pendingReset = false;
				apu->ResetCore(false);
			}
		}
	}

	void NSFPlayerBoard::ResetAPU(uint16_t addr, bool reset_apu_also)
	{
		pendingReset = true;
		resetHalfClkCounter = 24;
		this->reset_apu_also = reset_apu_also;

		apu->ResetCore(true);
		sram->SetFakeResetVector(addr);
	}

	bool NSFPlayerBoard::APUInResetState()
	{
		return pendingReset;
	}

	void NSFPlayerBoard::LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
	{
		if (sram != nullptr)
		{
			sram->LoadNSFData(data, data_size, load_address);

			dbg_hub->DisposeMemMap();
			AddBoardMemDescriptors();
		}
	}

	void NSFPlayerBoard::EnableNSFBanking(bool enable)
	{
		if (sram != nullptr)
		{
			sram->EnableNSFBanking(enable);
		}
	}

	void NSFPlayerBoard::LoadRegDump(uint8_t* data, size_t data_size)
	{
		// NSFPlayerBoard is not designed for this.
	}
}
