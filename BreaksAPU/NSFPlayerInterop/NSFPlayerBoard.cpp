// Module for maintaining a simulated APU environment.

#include "pch.h"

using namespace BaseLogic;

namespace NSFPlayer
{
	Board::Board(char* boardName, char* apuRev, char* ppuRev, char* p1)
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
		sram = new BankedSRAM();
		wram = new BaseBoard::SRAM(wram_bits);

		apu->SetNormalizedOutput(true);

		AddBoardMemDescriptors();
		AddDebugInfoProviders();
	}

	Board::~Board()
	{
		delete apu;
		delete core;
		delete sram;
		delete wram;
	}

	/// <summary>
	/// Simulate 1 half cycle of the test board with NSFPlayer. The simulation of the signal edge is not supported, this is overkill.
	/// </summary>
	void Board::Step()
	{
		// Simulate APU

		TriState inputs[(size_t)APUSim::APU_Input::Max]{};
		TriState outputs[(size_t)APUSim::APU_Output::Max]{};

		inputs[(size_t)APUSim::APU_Input::CLK] = CLK;

		inputs[(size_t)APUSim::APU_Input::n_NMI] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_IRQ] = TriState::One;
		inputs[(size_t)APUSim::APU_Input::n_RES] = pendingReset ? TriState::Zero : TriState::One;
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

		sram->sim(RnW, NOT(wram_cs), addr_bus, &data_bus);

		// Tick

		CLK = NOT(CLK);

		if (pendingReset)
		{
			resetHalfClkCounter--;
			if (resetHalfClkCounter == 0)
			{
				pendingReset = false;
			}
		}
	}

	/// <summary>
	/// "Insert" the cartridge as a .nes ROM. In this implementation we are simply trying to instantiate an NROM, but in a more advanced emulation, Cartridge Factory will take care of "inserting" the cartridge.
	/// </summary>
	/// <param name="nesImage">A pointer to the ROM image in memory.</param>
	/// <param name="nesImageSize">ROM image size in bytes.</param>
	/// <returns></returns>
	int Board::InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
	{
		// The NSF board does not have a cartridge connector.

		return 0;
	}

	/// <summary>
	/// Remove the cartridge. Logically this means that all terminals associated with the cartridge take the value of `z`.
	/// </summary>
	void Board::EjectCartridge()
	{
		// The NSF board does not have a cartridge connector.
	}

	/// <summary>
	/// Make the APU /RES pin = 0 for a few CLK half cycles so that the APU resets all of its internal circuits.
	/// </summary>
	void Board::ResetAPU()
	{
		apu->ResetACLKCounter();

		for (int i = 0; i < wram->Dbg_GetSize(); i++)
		{
			wram->Dbg_WriteByte(i, 0);
		}

		pendingReset = true;
		resetHalfClkCounter = 4;
	}

	/// <summary>
	/// The parent application can check that the APU is in the reset process and ignore the video signal for that time.
	/// </summary>
	/// <returns></returns>
	bool Board::APUInResetState()
	{
		return pendingReset;
	}

	size_t Board::GetACLKCounter()
	{
		return apu->GetACLKCounter();
	}

	void Board::SampleAudioSignal(float* sample)
	{
		if (sample != nullptr)
		{
			*sample = (aux.normalized.a + aux.normalized.b) / 2.0f;
		}
	}

	/// <summary>
	/// Load the whole NSF data image to the BankedSRAM device.
	/// </summary>
	/// <param name="data">nsf data offset +0x80</param>
	/// <param name="data_size">nsf data size</param>
	/// <param name="load_address">nsf load address (from header)</param>
	void Board::LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
	{
		if (sram != nullptr)
		{
			sram_load_addr = load_address;
			sram->LoadNSFData(data, data_size, load_address);

			dbg_hub->DisposeMemMap();
			AddBoardMemDescriptors();
		}
	}

	/// <summary>
	/// Enable the bank switching circuit for the BankedSRAM device.
	/// </summary>
	/// <param name="enable"></param>
	void Board::EnableNSFBanking(bool enable)
	{
		if (sram != nullptr)
		{
			sram->EnableNSFBanking(enable);
		}
	}
}
