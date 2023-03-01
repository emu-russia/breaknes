// Module for maintaining a simulated PPU environment.

#include "pch.h"

using namespace BaseLogic;

namespace Breaknes
{
	PPUPlayerBoard::PPUPlayerBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev) : Board(apu_rev, ppu_rev)
	{
		ppu = new PPUSim::PPU(ppu_rev);
		vram = new BaseBoard::SRAM(11);

		AddBoardMemDescriptors();
		AddDebugInfoProviders();
	}

	PPUPlayerBoard::~PPUPlayerBoard()
	{
		delete ppu;
		delete vram;
	}

	/// <summary>
	/// Simulate 1 half cycle of the test board with PPUPlayer. The simulation of the signal edge is not supported, this is overkill.
	/// </summary>
	void PPUPlayerBoard::Step()
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

		ALE = outputs[(size_t)PPUSim::OutputPad::ALE];
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

		bool dz = (n_RD == TriState::One && n_WR == TriState::One);
		vram->sim(n_VRAM_CS, n_WR, n_RD, &VRAM_Addr, &ad_bus, dz);

		// Tick

		CLK = NOT(CLK);

		if (pendingCpuOperation && ppu->GetPCLKCounter() != savedPclk)
		{
			pendingCpuOperation = false;
		}

		// Another scenario for canceling a CPU operation.
		// If the operation is started at the PCLK boundary, the PCLK Counter can change quite quickly.
		// To be sure, we will artificially delay the execution of the CPU operation to make it look natural.

		if (pendingCpuOperation && faithDelayCounter != 0)
		{
			faithDelayCounter--;
			if (faithDelayCounter == 0)
			{
				pendingCpuOperation = false;
			}
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
	void PPUPlayerBoard::CPUWrite(size_t ppuReg, uint8_t val)
	{
		if (!pendingCpuOperation)
		{
			ppuRegId = ppuReg;
			writeValue = val;
			pendingCpuOperation = true;
			pendingWrite = true;
			savedPclk = ppu->GetPCLKCounter();
			faithDelayCounter = faithDelay;
		}
	}

	/// <summary>
	/// Queue to read a value from the PPU register.
	/// The PPU pins will be exposed until PCLK is changed.
	/// </summary>
	void PPUPlayerBoard::CPURead(size_t ppuReg)
	{
		if (!pendingCpuOperation)
		{
			ppuRegId = ppuReg;
			pendingCpuOperation = true;
			pendingWrite = false;
			savedPclk = ppu->GetPCLKCounter();
			faithDelayCounter = faithDelay;
		}
	}

	/// <summary>
	/// "Insert" the cartridge as a .nes ROM. In this implementation we are simply trying to instantiate an NROM, but in a more advanced emulation, Cartridge Factory will take care of "inserting" the cartridge.
	/// </summary>
	/// <param name="nesImage">A pointer to the ROM image in memory.</param>
	/// <param name="nesImageSize">ROM image size in bytes.</param>
	/// <returns></returns>
	int PPUPlayerBoard::InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
	{
		if (cart == nullptr)
		{
			cart = new Mappers::NROM(nesImage, nesImageSize);

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

	/// <summary>
	/// Remove the cartridge. Logically this means that all terminals associated with the cartridge take the value of `z`.
	/// </summary>
	void PPUPlayerBoard::EjectCartridge()
	{
		if (cart != nullptr)
		{
			delete cart;
			cart = nullptr;
		}
	}

	/// <summary>
	/// Make the PPU /RES pin = 0 for a few CLK half cycles so that the PPU resets all of its internal circuits.
	/// </summary>
	void PPUPlayerBoard::Reset()
	{
		pendingReset = true;
		resetHalfClkCounter = 4;
	}

	/// <summary>
	/// The parent application can check that the PPU is in the reset process and ignore the video signal for that time.
	/// </summary>
	/// <returns></returns>
	bool PPUPlayerBoard::InResetState()
	{
		return pendingReset;
	}

	void PPUPlayerBoard::LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
	{
	}

	void PPUPlayerBoard::EnableNSFBanking(bool enable)
	{
	}

	void PPUPlayerBoard::LoadRegDump(uint8_t* data, size_t data_size)
	{
	}
}
