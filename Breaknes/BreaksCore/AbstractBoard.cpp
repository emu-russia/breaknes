#include "pch.h"

namespace Breaknes
{
	Board::Board(Core* _core, APUSim::Revision apu_rev, PPUSim::Revision ppu_rev)
	{
		core = _core;
		cpu = new M6502Core::M6502(false, true);
		apu = new APUSim::APU(cpu, apu_rev);
		ppu = new PPUSim::PPU(ppu_rev);
	}

	Board::~Board()
	{
		delete cpu;
		delete apu;
		delete ppu;
		DestroyCartridge();
	}

	void Board::InsertCartridge(AbstractCartridge* _cart)
	{
		assert(cart == nullptr);
		cart = _cart;
	}

	void Board::DestroyCartridge()
	{
		if (cart)
		{
			delete cart;
			cart = nullptr;
		}
	}
}
