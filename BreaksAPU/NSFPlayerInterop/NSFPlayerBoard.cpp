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

		apu->SetNormalizedOutput(true);

		AddBoardMemDescriptors();
		AddDebugInfoProviders();
	}

	Board::~Board()
	{
		delete apu;
		delete core;
		delete sram;
	}

	/// <summary>
	/// Simulate 1 half cycle of the test board with NSFPlayer. The simulation of the signal edge is not supported, this is overkill.
	/// </summary>
	void Board::Step()
	{
		// Simulate APU

		// TBD: By analogy.

		//TriState inputs[(size_t)PPUSim::InputPad::Max]{};
		//TriState outputs[(size_t)PPUSim::OutputPad::Max]{};

		//inputs[(size_t)PPUSim::InputPad::CLK] = CLK;
		//inputs[(size_t)PPUSim::InputPad::n_RES] = pendingReset ? TriState::Zero : TriState::One;
		//inputs[(size_t)PPUSim::InputPad::RnW] = pendingWrite ? TriState::Zero : TriState::One;
		//inputs[(size_t)PPUSim::InputPad::RS0] = pendingCpuOperation ? ((ppuRegId & 1) ? TriState::One : TriState::Zero) : TriState::Zero;
		//inputs[(size_t)PPUSim::InputPad::RS1] = pendingCpuOperation ? ((ppuRegId & 2) ? TriState::One : TriState::Zero) : TriState::Zero;
		//inputs[(size_t)PPUSim::InputPad::RS2] = pendingCpuOperation ? ((ppuRegId & 4) ? TriState::One : TriState::Zero) : TriState::Zero;
		//inputs[(size_t)PPUSim::InputPad::n_DBE] = pendingCpuOperation ? TriState::Zero : TriState::One;

		//if (pendingCpuOperation && pendingWrite)
		//{
		//	data_bus = writeValue;
		//}

		//ppu->sim(inputs, outputs, &ext_bus, &data_bus, &ad_bus, &pa8_13, vidSample);

		//ALE = outputs[(size_t)PPUSim::OutputPad::ALE];
		//n_RD = outputs[(size_t)PPUSim::OutputPad::n_RD];
		//n_WR = outputs[(size_t)PPUSim::OutputPad::n_WR];
		//n_INT = outputs[(size_t)PPUSim::OutputPad::n_INT];

		if (n_INT == TriState::Z)
		{
			n_INT = TriState::One;		// pullup
		}

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
}
