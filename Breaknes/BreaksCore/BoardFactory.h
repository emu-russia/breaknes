#pragma once

namespace Breaknes
{
	class BoardFactory
	{
		std::string board_name = "Bogus";
		APUSim::Revision apu_rev = APUSim::Revision::Unknown;
		PPUSim::Revision ppu_rev = PPUSim::Revision::Unknown;
		Mappers::ConnectorType p1_type = Mappers::ConnectorType::None;

	public:
		BoardFactory(std::string board, std::string apu, std::string ppu, std::string p1);
		~BoardFactory();

		Board* CreateInstance();
	};
}
