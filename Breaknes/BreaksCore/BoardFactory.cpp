#include "pch.h"

namespace Breaknes
{
	BoardFactory::BoardFactory(std::string board, std::string apu, std::string ppu, std::string p1)
	{
		board_name = board;

		// Perform a reflection for APU

		if (apu == "RP2A03")
		{
			apu_rev = APUSim::Revision::RP2A03;
		}
		else if (apu == "RP2A07")
		{
			apu_rev = APUSim::Revision::RP2A07;
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
		else if (ppu == "RP2C03")
		{
			ppu_rev = PPUSim::Revision::RP2C03;
		}
		else if (ppu == "RP2C03B")
		{
			ppu_rev = PPUSim::Revision::RP2C03B;
		}
		else if (ppu == "RP2C04_0001")
		{
			ppu_rev = PPUSim::Revision::RP2C04_0001;
		}
		else if (ppu == "RP2C04_0002")
		{
			ppu_rev = PPUSim::Revision::RP2C04_0002;
		}
		else if (ppu == "RP2C04_0003")
		{
			ppu_rev = PPUSim::Revision::RP2C04_0003;
		}
		else if (ppu == "RP2C04_0004")
		{
			ppu_rev = PPUSim::Revision::RP2C04_0004;
		}
		else if (ppu == "RP2C05")
		{
			ppu_rev = PPUSim::Revision::RP2C05;
		}
		else if (ppu == "RP2C07_0")
		{
			ppu_rev = PPUSim::Revision::RP2C07_0;
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
		else
		{
			inst = new BogusBoard(apu_rev, ppu_rev);
		}

		return inst;
	}
}
