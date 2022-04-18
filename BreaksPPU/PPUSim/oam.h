// OAM

#pragma once

namespace PPUSim
{
	class OAM
	{
		PPU* ppu = nullptr;

	public:
		OAM(PPU* parent);
		~OAM();

		void sim();
	};
}
