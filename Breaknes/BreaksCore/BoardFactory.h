#pragma once

#include <vector>

namespace Breaknes
{
	class BoardFactory
	{
		std::string board_name = "Bogus";
		APUSim::Revision apu_rev = APUSim::Revision::Unknown;
		std::vector<PPUSim::Revision> ppu_revs;
		CartPcb::ConnectorType p1_type = CartPcb::ConnectorType::None;

	public:
		BoardFactory(std::string board, std::string apu, std::vector<std::string> ppus, std::string p1);
		~BoardFactory();

		Board* CreateInstance();
	};
}
