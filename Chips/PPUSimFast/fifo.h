// Object FIFO (Motion picture buffer memory)

#pragma once

namespace PPUSimFast
{
	// Packed copies of the original per-bit storage. Both banks remain distinct:
	// a transparent latch update is not a single edge-triggered register update.
	class FIFO_Counter
	{
		friend PPUSimUnitTest::UnitTest;
		uint8_t keep = 0;
		uint8_t step_latch = 0;

	public:
		BaseLogic::TriState sim(BaseLogic::TriState Clock, BaseLogic::TriState Load,
			BaseLogic::TriState Step, uint8_t value);
		uint8_t get() const { return keep; }
	};

	class FIFO_SR
	{
		friend PPUSimUnitTest::UnitTest;
		uint8_t input = 0;
		uint8_t output = 0;

	public:
		BaseLogic::TriState sim(BaseLogic::TriState n_PCLK, BaseLogic::TriState T_SR,
			BaseLogic::TriState SR_EN, uint8_t value);
	};

	struct FIFOLaneOutput
	{
		BaseLogic::TriState nZ_COL0;
		BaseLogic::TriState nZ_COL1;
		BaseLogic::TriState Z_COL2;
		BaseLogic::TriState Z_COL3;
		BaseLogic::TriState nZ_PRIO;
		BaseLogic::TriState n_xEN;
	};

	class FIFOLane
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		FIFO_SR paired_sr[2]{};
		FIFO_Counter down_cnt;

		BaseLogic::DLatch ob0_latch[2];
		BaseLogic::DLatch ob1_latch[2];
		BaseLogic::DLatch ob5_latch[2];
		BaseLogic::DLatch hsel_latch;

		BaseLogic::TriState nZ_COL0 = BaseLogic::TriState::X;
		BaseLogic::TriState nZ_COL1 = BaseLogic::TriState::X;
		BaseLogic::TriState Z_COL2 = BaseLogic::TriState::X;
		BaseLogic::TriState Z_COL3 = BaseLogic::TriState::X;
		BaseLogic::TriState nZ_PRIO = BaseLogic::TriState::X;
		BaseLogic::TriState SR_EN = BaseLogic::TriState::X;
		BaseLogic::TriState LDAT = BaseLogic::TriState::X;
		BaseLogic::TriState LOAD = BaseLogic::TriState::X;
		BaseLogic::TriState T_SR0 = BaseLogic::TriState::X;
		BaseLogic::TriState T_SR1 = BaseLogic::TriState::X;

		BaseLogic::FF ZH_FF;
		BaseLogic::DLatch en_latch;

		BaseLogic::TriState UPD = BaseLogic::TriState::X;
		BaseLogic::TriState STEP = BaseLogic::TriState::X;
		BaseLogic::TriState n_EN = BaseLogic::TriState::X;

		void sim_LaneControl(BaseLogic::TriState HSel);
		void sim_CounterControl();
		BaseLogic::TriState sim_Counter(uint8_t packed_OB);
		void sim_CounterCarry(BaseLogic::TriState Carry);
		void sim_PairedSREnable();
		void sim_PairedSR(uint8_t packed_nTX);

		size_t get_Counter();

	public:
		FIFOLane(PPU* parent);
		~FIFOLane();

		void sim(BaseLogic::TriState HSel, uint8_t packed_OB, uint8_t packed_nTX, FIFOLaneOutput& ZOut);
	};

	class FIFO
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		FIFOLane* lane[8];

		BaseLogic::DLatch zh_latch1;
		BaseLogic::DLatch zh_latch2;
		BaseLogic::DLatch zh_latch3;

		BaseLogic::FF HINV_FF;
		BaseLogic::DLatch tout_latch[8];
		BaseLogic::TriState n_TX[8]{};
		uint8_t packed_nTX = 0;

		BaseLogic::DLatch sh2_latch;
		BaseLogic::DLatch sh3_latch;
		BaseLogic::DLatch sh5_latch;
		BaseLogic::DLatch sh7_latch;

		BaseLogic::DLatch s0_latch;
		BaseLogic::DLatch col2_latch;
		BaseLogic::DLatch col3_latch;
		BaseLogic::DLatch prio_latch;

		FIFOLaneOutput LaneOut[8]{};

		void sim_HInv();
		void sim_Lanes();
		void sim_Prio();

		BaseLogic::TriState get_nZCOL0(size_t lane);
		BaseLogic::TriState get_nZCOL1(size_t lane);
		BaseLogic::TriState get_nxEN(size_t lane);

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
