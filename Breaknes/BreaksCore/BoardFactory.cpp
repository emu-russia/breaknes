#include "pch.h"

namespace Breaknes
{
	BoardFactory::BoardFactory(std::string board, std::string apu, std::string ppu, std::string p1)
	{
		board_name = board;

		// Perform a reflection for APU

		if (apu == "RP2A03G")
		{
			apu_rev = APUSim::Revision::RP2A03G;
		}
		else if (apu == "RP2A03H")
		{
			apu_rev = APUSim::Revision::RP2A03H;
		}
		else if (apu == "RP2A07")
		{
			apu_rev = APUSim::Revision::RP2A07;
		}
		else if (apu == "UA6527P")
		{
			apu_rev = APUSim::Revision::UA6527P;
		}
		else if (apu == "TA03NP1")
		{
			apu_rev = APUSim::Revision::TA03NP1;
		}
		else
		{
			board_name = "Bogus";
		}

		// Perform a reflection for PPU

		if (ppu == "RP2C02G")
		{
			ppu_rev = PPUSim::Revision::RP2C02G;
		}
		else if (ppu == "RP2C02H")
		{
			ppu_rev = PPUSim::Revision::RP2C02H;
		}
		else if (ppu == "RP2C03B")
		{
			ppu_rev = PPUSim::Revision::RP2C03B;
		}
		else if (ppu == "RP2C03C")
		{
			ppu_rev = PPUSim::Revision::RP2C03C;
		}
		else if (ppu == "RC2C03B")
		{
			ppu_rev = PPUSim::Revision::RC2C03B;
		}
		else if (ppu == "RC2C03C")
		{
			ppu_rev = PPUSim::Revision::RC2C03C;
		}
		else if (ppu == "RP2C04-0001")
		{
			ppu_rev = PPUSim::Revision::RP2C04_0001;
		}
		else if (ppu == "RP2C04-0002")
		{
			ppu_rev = PPUSim::Revision::RP2C04_0002;
		}
		else if (ppu == "RP2C04-0003")
		{
			ppu_rev = PPUSim::Revision::RP2C04_0003;
		}
		else if (ppu == "RP2C04-0004")
		{
			ppu_rev = PPUSim::Revision::RP2C04_0004;
		}
		else if (ppu == "RC2C05-01")
		{
			ppu_rev = PPUSim::Revision::RC2C05_01;
		}
		else if (ppu == "RC2C05-02")
		{
			ppu_rev = PPUSim::Revision::RC2C05_02;
		}
		else if (ppu == "RC2C05-03")
		{
			ppu_rev = PPUSim::Revision::RC2C05_03;
		}
		else if (ppu == "RC2C05-04")
		{
			ppu_rev = PPUSim::Revision::RC2C05_04;
		}
		else if (ppu == "RC2C05-99")
		{
			ppu_rev = PPUSim::Revision::RC2C05_99;
		}
		else if (ppu == "RP2C07-0")
		{
			ppu_rev = PPUSim::Revision::RP2C07_0;
		}
		else if (ppu == "UMC UA6538")
		{
			ppu_rev = PPUSim::Revision::UMC_UA6538;
		}
		else
		{
			board_name = "Bogus";
		}

		// Perform a reflection for cartridge slot

		if (p1 == "Fami")
		{
			p1_type = ConnectorType::FamicomStyle;
		}
		else if (p1 == "NES")
		{
			p1_type = ConnectorType::NESStyle;
		}
		else
		{
			board_name = "Bogus";
		}
	}

	BoardFactory::~BoardFactory()
	{
	}

	Board* BoardFactory::CreateInstance()
	{
		Board* inst = nullptr;

		if (board_name == "Famicom")
		{
			// TBD
		}
		else if (board_name == "NES")
		{
			// TBD
		}
		else if (board_name == "NSFPlayer")
		{
			inst = new NSFPlayerBoard(apu_rev, ppu_rev);
		}
		else if (board_name == "APUPlayer")
		{
			inst = new APUPlayerBoard(apu_rev, ppu_rev);
		}
		else if (board_name == "PPUPlayer")
		{
			inst = new PPUPlayerBoard(apu_rev, ppu_rev);
		}
		else
		{
			inst = new BogusBoard(apu_rev, ppu_rev);
		}

		return inst;
	}
}
