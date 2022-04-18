// Scrolling Registers

#pragma once

namespace PPUSim
{
	class ScrollRegs
	{
		PPU* ppu = nullptr;

	public:
		ScrollRegs(PPU* parent);
		~ScrollRegs();

		void sim();
	};
}
