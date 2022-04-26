// Module for maintaining a simulated PPU environment.

#include "pch.h"

using namespace BaseLogic;

namespace PPUPlayer
{
	Board::Board()
	{
		ppu = new PPUSim::PPU(PPUSim::Revision::RP2C02G);
		vram = new Breaknes::SRAM(11);
	}

	Board::~Board()
	{
		delete ppu;
		delete vram;
	}

	void Board::Step()
	{
		// Simulate PPU

		TriState inputs[(size_t)PPUSim::InputPad::Max]{};
		TriState outputs[(size_t)PPUSim::OutputPad::Max]{};

		inputs[(size_t)PPUSim::InputPad::CLK] = CLK;
		inputs[(size_t)PPUSim::InputPad::n_RES] = TriState::One;
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
		TriState n_RD = outputs[(size_t)PPUSim::OutputPad::n_RD];
		TriState n_WR = outputs[(size_t)PPUSim::OutputPad::n_WR];

		// Simulate all other surrounding logic and cartridge

		uint8_t LatchedAddress = 0;
		bool LatchOutZ = false;

		latch.sim(ALE, TriState::Zero, ad_bus, &LatchedAddress, LatchOutZ);

		TriState PA[14]{};
		TriState n_PA13 = NOT(PA[13]);

		n_VRAM_CS = TriState::Zero;
		VRAM_A10 = TriState::Zero;

		if (cart != nullptr)
		{
			cart->sim(PA, n_PA13, n_RD, n_WR, &ad_bus, ADDirty, n_VRAM_CS, VRAM_A10);
		}

		// Tick

		CLK = NOT(CLK);

		if (pendingCpuOperation && ppu->GetPCLKCounter() != savedPclk)
		{
			pendingCpuOperation = false;
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

	void Board::InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
	{
		if (cart == nullptr)
		{
			cart = new NROM(nesImage, nesImageSize);
		}
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
}
