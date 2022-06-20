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
		else if (!strcmp(ppuRev, "RP2C04-0003"))
		{
			rev = PPUSim::Revision::RP2C04_0003;
		}
		else if (!strcmp(ppuRev, "RP2C07-0"))
		{
			rev = PPUSim::Revision::RP2C07_0;
		}
		else if (!strcmp(ppuRev, "UMC UA6538"))
		{
			rev = PPUSim::Revision::UMC_UA6538;
		}
		else
		{
			// If someone passed on an obscure PPU revision, stay with the default reference RP2C02G, as the most studied.
			rev = PPUSim::Revision::RP2C02G;
		}

		ppu = new PPUSim::PPU(rev);
		vram = new BaseBoard::SRAM(11);

		pal = new RGB_Triplet[8 * 64];

		AddBoardMemDescriptors();
		AddDebugInfoProviders();
	}

	Board::~Board()
	{
		delete ppu;
		delete vram;
		delete pal;
	}

	/// <summary>
	/// Simulate 1 half cycle of the test board with PPUPlayer. The simulation of the signal edge is not supported, this is overkill.
	/// </summary>
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

	/// <summary>
	/// Queue to read a value from the PPU register.
	/// The PPU pins will be exposed until PCLK is changed.
	/// </summary>
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

	/// <summary>
	/// Get the "pixel" counter. Keep in mind that pixels refers to an abstract entity representing the visible or invisible part of the video signal.
	/// </summary>
	/// <returns></returns>
	size_t Board::GetPCLKCounter()
	{
		return ppu->GetPCLKCounter();
	}

	/// <summary>
	/// "Insert" the cartridge as a .nes ROM. In this implementation we are simply trying to instantiate an NROM, but in a more advanced emulation, Cartridge Factory will take care of "inserting" the cartridge.
	/// </summary>
	/// <param name="nesImage">A pointer to the ROM image in memory.</param>
	/// <param name="nesImageSize">ROM image size in bytes.</param>
	/// <returns></returns>
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

	/// <summary>
	/// Remove the cartridge. Logically this means that all terminals associated with the cartridge take the value of `z`.
	/// </summary>
	void Board::EjectCartridge()
	{
		if (cart != nullptr)
		{
			delete cart;
			cart = nullptr;
		}
	}

	/// <summary>
	/// Get 1 sample of the video signal.
	/// </summary>
	/// <param name="sample"></param>
	void Board::SampleVideoSignal(PPUSim::VideoOutSignal* sample)
	{
		if (sample != nullptr)
		{
			*sample = vidSample;
		}
	}

	/// <summary>
	/// Get the direct value from the PPU H counter.
	/// </summary>
	/// <returns></returns>
	size_t Board::GetHCounter()
	{
		return ppu->GetHCounter();
	}

	/// <summary>
	/// Get the direct value from the PPU V counter.
	/// </summary>
	/// <returns></returns>
	size_t Board::GetVCounter()
	{
		return ppu->GetVCounter();
	}

	/// <summary>
	/// Make the PPU /RES pin = 0 for a few CLK half cycles so that the PPU resets all of its internal circuits.
	/// </summary>
	void Board::ResetPPU()
	{
		pendingReset = true;
		resetHalfClkCounter = 4;
	}

	/// <summary>
	/// The parent application can check that the PPU is in the reset process and ignore the video signal for that time.
	/// </summary>
	/// <returns></returns>
	bool Board::PPUInResetState()
	{
		return pendingReset;
	}

	/// <summary>
	/// Forcibly enable rendering ($2001[3] = $2001[4] always equals 1). 
	/// Used for debugging PPU signals, when the CPU I/F register dump is limited, or when you want to get faster simulation results. 
	/// Keep in mind that with permanently enabled rendering the PPU becomes unstable and this hack should be applied when you know what you're doing.
	/// </summary>
	/// <param name="enable"></param>
	void Board::RenderAlwaysEnabled(bool enable)
	{
		ppu->Dbg_RenderAlwaysEnabled(enable);
	}

	/// <summary>
	/// Get video signal settings that help with its rendering on the consumer side.
	/// </summary>
	/// <param name="features"></param>
	void Board::GetSignalFeatures(PPUSim::VideoSignalFeatures* features)
	{
		PPUSim::VideoSignalFeatures feat{};
		ppu->GetSignalFeatures(feat);
		*features = feat;
	}

	/// <summary>
	/// Convert the raw color to RGB. Can be used for palette generation or PPU video output in RAW mode.
	/// The SYNC level (RAW.Sync) check must be done from the outside.
	/// </summary>
	void Board::ConvertRAWToRGB(uint16_t raw, uint8_t* r, uint8_t* g, uint8_t* b)
	{
		if (!pal_cached)
		{
			PPUSim::VideoOutSignal rawIn{}, rgbOut{};

			// 8 Emphasis bands, each with 64 colors.

			for (size_t n = 0; n < (8 * 64); n++)
			{
				rawIn.RAW.raw = (uint16_t)n;
				ppu->ConvertRAWToRGB(rawIn, rgbOut);
				pal[n].r = rgbOut.RGB.RED;
				pal[n].g = rgbOut.RGB.GREEN;
				pal[n].b = rgbOut.RGB.BLUE;
			}

			pal_cached = true;
		}

		size_t n = raw & 0b111'11'1111;

		*r = pal[n].r;
		*g = pal[n].g;
		*b = pal[n].b;
	}

	/// <summary>
	/// Use RAW color output. 
	/// RAW color refers to the Chroma/Luma combination that comes to the video generator and the Emphasis bit combination.
	/// </summary>
	/// <param name="enable"></param>
	void Board::SetRAWColorMode(bool enable)
	{
		ppu->SetRAWOutput(enable);
	}

	/// <summary>
	/// Set one of the ways to decay OAM cells.
	/// </summary>
	void Board::SetOamDecayBehavior(PPUSim::OAMDecayBehavior behavior)
	{
		ppu->SetOamDecayBehavior(behavior);
	}
}
