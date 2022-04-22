// Pattern Address Generator

#pragma once

namespace PPUSim
{
	class PATGen
	{
		PPU* ppu = nullptr;

	public:
		PATGen(PPU* parent);
		~PATGen();

		void sim();
	};
}
