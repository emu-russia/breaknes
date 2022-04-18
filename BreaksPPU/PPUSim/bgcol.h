// Background Color (BG COL)

#pragma once

namespace PPUSim
{
	class BGCol
	{
		PPU* ppu = nullptr;

	public:
		BGCol(PPU* parent);
		~BGCol();

		void sim();
	};
}
