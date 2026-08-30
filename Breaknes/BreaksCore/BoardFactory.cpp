#include "pch.h"

namespace Breaknes
{
	// The revision lookup tables (issue #521). The BoardDescription.json lists the
	// chip markings of the real hardware. Not every marking is implemented in the
	// chip simulations yet: the entries marked `exact = false` are approximated by
	// the closest supported revision of the same chip family (the difference is
	// logged when the board is created).

	struct APURevisionDesc
	{
		const char* name;
		APUSim::Revision rev;
		bool exact;
	};

	static const APURevisionDesc APURevisions[] =
	{
		{ "RP2A03",		APUSim::Revision::RP2A03,	false },	// "letterless", launch Famicoms; the sim currently uses the RP2A03G model
		{ "RP2A03E",	APUSim::Revision::RP2A03G,	false },	// not implemented; approximated by RP2A03G
		{ "RP2A03G",	APUSim::Revision::RP2A03G,	true },
		{ "RP2A03H",	APUSim::Revision::RP2A03H,	true },
		{ "RP2A07",		APUSim::Revision::RP2A07,	true },
		{ "RP2A07A",	APUSim::Revision::RP2A07,	false },	// not implemented; approximated by RP2A07
		{ "UA6527P",	APUSim::Revision::UA6527P,	true },
		{ "TA03NP1",	APUSim::Revision::TA03NP1,	true },
	};

	struct PPURevisionDesc
	{
		const char* name;
		PPUSim::Revision rev;
		bool exact;
	};

	static const PPURevisionDesc PPURevisions[] =
	{
		{ "RP2C02",		PPUSim::Revision::RP2C02G,		false },	// "letterless", launch Famicoms; approximated by RP2C02G
		{ "RP2C02A",	PPUSim::Revision::RP2C02G,		false },	// not implemented; approximated by RP2C02G
		{ "RP2C02B",	PPUSim::Revision::RP2C02G,		false },	// not implemented; approximated by RP2C02G
		{ "RP2C02C",	PPUSim::Revision::RP2C02G,		false },	// not implemented; approximated by RP2C02G
		{ "RC2C02C",	PPUSim::Revision::RP2C02G,		false },	// ceramic package of the -C; approximated by RP2C02G
		{ "RP2C02D",	PPUSim::Revision::RP2C02G,		false },	// not implemented; approximated by RP2C02G
		{ "RP2C02D-0",	PPUSim::Revision::RP2C02G,		false },	// not implemented; approximated by RP2C02G
		{ "RP2C02E",	PPUSim::Revision::RP2C02G,		false },	// not implemented; approximated by RP2C02G
		{ "RP2C02E-0",	PPUSim::Revision::RP2C02G,		false },	// not implemented; approximated by RP2C02G
		{ "RP2C02G",	PPUSim::Revision::RP2C02G,		true },
		{ "RP2C02G-0",	PPUSim::Revision::RP2C02G,		true },	// the "-0" suffix marking; same simulation model
		{ "RP2C02H",	PPUSim::Revision::RP2C02H,		true },
		{ "RP2C02H-0",	PPUSim::Revision::RP2C02H,		true },	// the "-0" suffix marking; same simulation model
		{ "RP2C03B",	PPUSim::Revision::RP2C03B,		true },
		{ "RP2C03C",	PPUSim::Revision::RP2C03C,		true },
		{ "RC2C03B",	PPUSim::Revision::RC2C03B,		true },
		{ "RC2C03C",	PPUSim::Revision::RC2C03C,		true },
		{ "RP2C04-0001",PPUSim::Revision::RP2C04_0001,	true },
		{ "RP2C04-0002",PPUSim::Revision::RP2C04_0002,	true },
		{ "RP2C04-0003",PPUSim::Revision::RP2C04_0003,	true },
		{ "RP2C04-0004",PPUSim::Revision::RP2C04_0004,	true },
		{ "RC2C05-01",	PPUSim::Revision::RC2C05_01,	true },
		{ "RC2C05-02",	PPUSim::Revision::RC2C05_02,	true },
		{ "RC2C05-03",	PPUSim::Revision::RC2C05_03,	true },
		{ "RC2C05-04",	PPUSim::Revision::RC2C05_04,	true },
		{ "RC2C05-99",	PPUSim::Revision::RC2C05_99,	true },
		{ "RP2C07",		PPUSim::Revision::RP2C07_0,		false },	// not implemented; approximated by RP2C07-0
		{ "RP2C07-0",	PPUSim::Revision::RP2C07_0,		true },
		{ "RP2C07A-0",	PPUSim::Revision::RP2C07_0,		false },	// not implemented; approximated by RP2C07-0
		{ "UMC UA6538",	PPUSim::Revision::UMC_UA6538,	true },
	};

	BoardFactory::BoardFactory(std::string board, std::string apu, std::vector<std::string> ppus, std::string p1)
	{
		board_name = board;

		// Perform a reflection for APU

		bool apu_found = false;
		for (auto& entry : APURevisions)
		{
			if (apu == entry.name)
			{
				apu_rev = entry.rev;
				apu_found = true;
				if (!entry.exact)
				{
					LOG_BOARD(Cat_Events, "APU revision %s is not implemented, using the closest supported model", apu.c_str());
				}
				break;
			}
		}

		if (!apu_found)
		{
			board_name = "Bogus";
		}

		// Perform a reflection for each PPU (a board may contain several PPUs, up to 2)

		for (auto& ppu : ppus)
		{
			PPUSim::Revision rev = PPUSim::Revision::Unknown;
			bool ppu_found = false;
			for (auto& entry : PPURevisions)
			{
				if (ppu == entry.name)
				{
					rev = entry.rev;
					ppu_found = true;
					if (!entry.exact)
					{
						LOG_BOARD(Cat_Events, "PPU revision %s is not implemented, using the closest supported model", ppu.c_str());
					}
					break;
				}
			}

			if (!ppu_found)
			{
				board_name = "Bogus";
				break;
			}
			ppu_revs.push_back(rev);
		}

		// The current boards (NES/Famicom/PPUPlayer) all require a PPU. A board
		// without PPUs would dereference a null PPU in Step(), so degrade to Bogus.

		if (ppu_revs.empty())
		{
			board_name = "Bogus";
		}

		// Perform a reflection for cartridge slot

		if (p1 == "Fami")
		{
			p1_type = CartPcb::ConnectorType::FamicomStyle;
		}
		else if (p1 == "NES")
		{
			p1_type = CartPcb::ConnectorType::NESStyle;
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

		// At this time, we don't pay much attention to the differences between the NES/Famicom models and consider them to be `Generic'.
		// As more information about significant differences appears, we will add it.

		if (std::string(board_name).find("HVC") != std::string::npos)
		{
			inst = new FamicomBoard(apu_rev, ppu_revs, p1_type);
		}
		else if ( std::string(board_name).find("NES") != std::string::npos )
		{
			inst = new NESBoard(apu_rev, ppu_revs, p1_type);
		}
		else if (board_name == "APUPlayer")
		{
			inst = new APUPlayerBoard(apu_rev, ppu_revs, p1_type);
		}
		else if (board_name == "PPUPlayer")
		{
			inst = new PPUPlayerBoard(apu_rev, ppu_revs, p1_type);
		}
		else
		{
			inst = new BogusBoard(apu_rev, ppu_revs, p1_type);
		}

		return inst;
	}
}
