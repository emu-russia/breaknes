// Picture Address Generator

#pragma once

namespace PPUSim
{
	class PARGen
	{
		PPU* ppu = nullptr;

	public:
		PARGen(PPU* parent);
		~PARGen();

		void sim();
	};
}
