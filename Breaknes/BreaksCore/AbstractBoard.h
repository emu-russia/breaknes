#pragma once

namespace Breaknes
{
	class Board
	{
	protected:

		// These basic chips are on all variations of motherboards, so we make them available to all inherited classes.

		M6502Core::M6502* cpu = nullptr;
		APUSim::APU* apu = nullptr;
		PPUSim::PPU* ppu = nullptr;

		// The cartridge slot supports hotplugging during simulation.

		AbstractCartridge* cart = nullptr;

	public:
		Board(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev);
		virtual ~Board();

		void InsertCartridge(AbstractCartridge* cart);
		void DestroyCartridge();

		virtual void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analogOutputs[]) = 0;
	};
}
