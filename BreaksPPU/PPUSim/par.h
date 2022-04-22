// Picture Address Register

#pragma once

namespace PPUSim
{
	class PAR
	{
		PPU* ppu = nullptr;

	public:
		PAR(PPU* parent);
		~PAR();

		void sim();
	};
}
