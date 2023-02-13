// A special version of the board, which contains a bare APU and Fake6502, which can only read/write APU registers from the dump.

#include "pch.h"

using namespace BaseLogic;

namespace NSFPlayer
{
	APUPlayerBoard::APUPlayerBoard(char* boardName, char* apuRev, char* ppuRev, char* p1) : Board(boardName, apuRev, ppuRev, p1)
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

		core = new M6502Core::FakeM6502();
		apu = new APUSim::APU(core, rev);
		wram = new BaseBoard::SRAM(wram_bits);

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
		inputs[(size_t)APUSim::APU_Input::n_RES] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::DBG] = TriState::Zero;

		apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);

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

		// Tick

		CLK = NOT(CLK);
	}

	int APUPlayerBoard::InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
	{
		// The NSF board does not have a cartridge connector.

		return 0;
	}

	void APUPlayerBoard::EjectCartridge()
	{
		// The NSF board does not have a cartridge connector.
	}

	void APUPlayerBoard::ResetAPU(uint16_t addr, bool reset_apu_also)
	{
	}

	bool APUPlayerBoard::APUInResetState()
	{
		return false;
	}

	size_t APUPlayerBoard::GetACLKCounter()
	{
		return apu->GetACLKCounter();
	}

	size_t APUPlayerBoard::GetPHICounter()
	{
		return apu->GetPHICounter();
	}

	void APUPlayerBoard::SampleAudioSignal(float* sample)
	{
		if (sample != nullptr)
		{
			*sample = (aux.normalized.a + aux.normalized.b) / 2.0f;
		}
	}

	void APUPlayerBoard::LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
	{
	}

	void APUPlayerBoard::EnableNSFBanking(bool enable)
	{
	}

	void APUPlayerBoard::LoadRegDump(uint8_t* data, size_t data_size)
	{
		// TBD
	}

	void APUPlayerBoard::GetSignalFeatures(APUSim::AudioSignalFeatures* features)
	{
		APUSim::AudioSignalFeatures feat{};
		apu->GetSignalFeatures(feat);
		*features = feat;
	}
}