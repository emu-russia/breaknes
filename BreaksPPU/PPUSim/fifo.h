// OAM FIFO (Motion picture buffer memory)

#pragma once

namespace PPUSim
{
	class FIFO
	{
		PPU* ppu = nullptr;

		BaseLogic::DLatch sh2_latch;
		BaseLogic::DLatch sh3_latch;
		BaseLogic::DLatch sh5_latch;
		BaseLogic::DLatch sh7_latch;

		void sim_HInv();

	public:
		FIFO(PPU* parent);
		~FIFO();

		void sim();

		/// <summary>
		/// You can call right after the FSM.
		/// </summary>
		void sim_SpriteH();
	};
}
