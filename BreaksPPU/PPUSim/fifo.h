// OAM FIFO (Motion picture buffer memory)

#pragma once

namespace PPUSim
{
	class FIFO
	{
		PPU* ppu = nullptr;

	public:
		FIFO(PPU* parent);
		~FIFO();

		void sim();
	};
}
