// OAM FIFO (Motion picture buffer memory)

#pragma once

namespace PPUSim
{
	class FIFO_CounterBit
	{
	public:
		void sim();
	};

	class FIFO_SRBit
	{
	public:
		void sim();
	};

	enum class FIFOLaneOutput
	{
		nZ_COL0 = 0,
		nZ_COL1,
		Z_COL2,
		Z_COL3,
		nZ_PRIO,
		n_xEN,
		Max,
	};

	class FIFOLane
	{
		PPU* ppu = nullptr;

		FIFO_SRBit paired_sr[2][8]{};
		FIFO_CounterBit down_cnt;

		void sim_LaneControl();
		void sim_CounterControl();
		void sim_PairedSR();
		void sim_Counter();

	public:
		FIFOLane(PPU* parent) { ppu = parent; }
		~FIFOLane() {}

		void sim(BaseLogic::TriState ZOut[(size_t)FIFOLaneOutput::Max]);
	};

	class FIFO
	{
		PPU* ppu = nullptr;

		FIFOLane* lane[8];

		BaseLogic::FF HINV_FF;
		BaseLogic::DLatch tout_latch[8];
		BaseLogic::TriState n_TX[8]{};

		BaseLogic::DLatch sh2_latch;
		BaseLogic::DLatch sh3_latch;
		BaseLogic::DLatch sh5_latch;
		BaseLogic::DLatch sh7_latch;

		BaseLogic::TriState LaneOut[8][(size_t)FIFOLaneOutput::Max]{};

		void sim_HInv();
		void sim_Lanes();
		void sim_Prio();

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
