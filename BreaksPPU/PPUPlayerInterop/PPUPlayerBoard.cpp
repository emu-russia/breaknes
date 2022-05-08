// Module for maintaining a simulated PPU environment.

#include "pch.h"

using namespace BaseLogic;

namespace PPUPlayer
{
	Board::Board(char* boardName, char* apu, char* ppuRev, char* p1)
	{
		PPUSim::Revision rev;

		if (!strcmp(ppuRev, "RP2C02G"))
		{
			rev = PPUSim::Revision::RP2C02G;
		}
		else if (!strcmp(ppuRev, "RP2C04-0001"))
		{
			rev = PPUSim::Revision::RP2C04_0001;
		}
		else if (!strcmp(ppuRev, "RP2C07-0"))
		{
			rev = PPUSim::Revision::RP2C07_0;
		}
		else
		{
			// If someone passed on an obscure PPU revision, stay with the default reference RP2C02G, as the most studied.
			rev = PPUSim::Revision::RP2C02G;
		}

		ppu = new PPUSim::PPU(rev);
		vram = new Breaknes::SRAM(11);

		AddBoardMemDescriptors();
		AddDebugInfoProviders();

		//ppu->Dbg_RandomizePicture(true);
		//ppu->Dbg_FixedPicture(true);
		//ppu->Dbg_RenderAlwaysEnabled(true);
	}

	Board::~Board()
	{
		delete ppu;
		delete vram;
	}

	void Board::Step()
	{
		ADDirty = false;
		ext_bus = 0;

		// Simulate PPU

		TriState inputs[(size_t)PPUSim::InputPad::Max]{};
		TriState outputs[(size_t)PPUSim::OutputPad::Max]{};

		inputs[(size_t)PPUSim::InputPad::CLK] = CLK;
		inputs[(size_t)PPUSim::InputPad::n_RES] = pendingReset ? TriState::Zero : TriState::One;
		inputs[(size_t)PPUSim::InputPad::RnW] = pendingWrite ? TriState::Zero : TriState::One;
		inputs[(size_t)PPUSim::InputPad::RS0] = pendingCpuOperation ? ((ppuRegId & 1) ? TriState::One : TriState::Zero) : TriState::Zero;
		inputs[(size_t)PPUSim::InputPad::RS1] = pendingCpuOperation ? ((ppuRegId & 2) ? TriState::One : TriState::Zero) : TriState::Zero;
		inputs[(size_t)PPUSim::InputPad::RS2] = pendingCpuOperation ? ((ppuRegId & 4) ? TriState::One : TriState::Zero) : TriState::Zero;
		inputs[(size_t)PPUSim::InputPad::n_DBE] = pendingCpuOperation ? TriState::Zero : TriState::One;

		if (pendingCpuOperation && pendingWrite)
		{
			data_bus = writeValue;
		}

		ppu->sim(inputs, outputs, &ext_bus, &data_bus, &ad_bus, &pa8_13, vidSample);

		TriState ALE = outputs[(size_t)PPUSim::OutputPad::ALE];
		n_RD = outputs[(size_t)PPUSim::OutputPad::n_RD];
		n_WR = outputs[(size_t)PPUSim::OutputPad::n_WR];
		n_INT = outputs[(size_t)PPUSim::OutputPad::n_INT];

		if (n_INT == TriState::Z)
		{
			n_INT = TriState::One;		// pullup
		}

		// Simulate all other surrounding logic and cartridge

		bool LatchOutZ = false;
		latch.sim(ALE, TriState::Zero, ad_bus, &LatchedAddress, LatchOutZ);

		for (size_t n = 0; n < 8; n++)
		{
			PA[n] = ((LatchedAddress >> n) & 1) ? TriState::One : TriState::Zero;
		}
		for (size_t n = 0; n < 6; n++)
		{
			PA[8 + n] = ((pa8_13 >> n) & 1) ? TriState::One : TriState::Zero;
		}
		n_PA13 = NOT(PA[13]);

		if (cart != nullptr)
		{
			cart->sim(PA, n_PA13, n_RD, n_WR, &ad_bus, ADDirty, n_VRAM_CS, VRAM_A10);
		}
		else
		{
			// No cartridge in the slot means 'z' on these signals.
			// Simulate this situation in the most painless way possible.

			n_VRAM_CS = TriState::One;		// VRAM closed
			VRAM_A10 = TriState::Zero;
		}

		VRAM_Addr = LatchedAddress;
		VRAM_Addr |= ((PA[8] == TriState::One) ? 1 : 0) << 8;
		VRAM_Addr |= ((PA[9] == TriState::One) ? 1 : 0) << 9;
		VRAM_Addr |= ((VRAM_A10 == TriState::One) ? 1 : 0) << 10;

		bool dz;
		vram->sim(n_VRAM_CS, n_WR, n_RD, &VRAM_Addr, &ad_bus, dz);

		// Tick

		CLK = NOT(CLK);

		if (pendingCpuOperation && ppu->GetPCLKCounter() != savedPclk)
		{
			pendingCpuOperation = false;
		}

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
	/// Queue to write a value to the PPU register.
	/// The PPU pins will be exposed until PCLK is changed.
	/// </summary>
	void Board::CPUWrite(size_t ppuReg, uint8_t val)
	{
		if (!pendingCpuOperation)
		{
			ppuRegId = ppuReg;
			writeValue = val;
			pendingCpuOperation = true;
			pendingWrite = true;
			savedPclk = ppu->GetPCLKCounter();
		}
	}

	void Board::CPURead(size_t ppuReg)
	{
		if (!pendingCpuOperation)
		{
			ppuRegId = ppuReg;
			pendingCpuOperation = true;
			pendingWrite = false;
			savedPclk = ppu->GetPCLKCounter();
		}
	}

	size_t Board::GetPCLKCounter()
	{
		return ppu->GetPCLKCounter();
	}

	int Board::InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
	{
		if (cart == nullptr)
		{
			cart = new NROM(nesImage, nesImageSize);

			if (!cart->Valid())
			{
				delete cart;
				cart = nullptr;

				return -1;
			}

			AddCartMemDescriptors();
			AddCartDebugInfoProviders();
		}
		return 0;
	}

	void Board::EjectCartridge()
	{
		if (cart != nullptr)
		{
			delete cart;
			cart = nullptr;
		}
	}

	void Board::SampleVideoSignal(float* sample)
	{
		if (sample != nullptr)
		{
			*sample = vidSample.composite;
		}
	}

	size_t Board::GetHCounter()
	{
		return ppu->GetHCounter();
	}

	size_t Board::GetVCounter()
	{
		return ppu->GetVCounter();
	}

	void Board::ResetPPU()
	{
		pendingReset = true;
		resetHalfClkCounter = 4;
	}

	bool Board::PPUInResetState()
	{
		return pendingReset;
	}
}
