// OAM FIFO (Motion picture buffer memory)

#pragma once

namespace PPUSim
{
	class FIFO_CounterBit
	{
		BaseLogic::FF keep_ff;
		BaseLogic::DLatch step_latch;

	public:
		BaseLogic::TriState sim(BaseLogic::TriState Clock, BaseLogic::TriState Load, BaseLogic::TriState Step,
			BaseLogic::TriState val_in,
			BaseLogic::TriState carry_in,
			BaseLogic::TriState& val_out);
		BaseLogic::TriState get();
	};

	class FIFO_SRBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		BaseLogic::TriState sim(BaseLogic::TriState n_PCLK, BaseLogic::TriState T_SR, BaseLogic::TriState SR_EN,
			BaseLogic::TriState nTx, BaseLogic::TriState shift_in);
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

		bool fast_fifo = true;

		FIFO_SRBit paired_sr[2][8]{};
		FIFO_CounterBit down_cnt[8]{};

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
		BaseLogic::TriState sim_Counter();
		void sim_CounterCarry(BaseLogic::TriState Carry);
		void sim_PairedSREnable();
		void sim_PairedSR(BaseLogic::TriState n_TX[8], uint8_t packed_nTX);

		size_t get_Counter();

		// Faster version of the paired SR and counter

		uint8_t fast_down_cnt = 0;		// Replacing the counter of the logic gates with an ordinary variable
		BaseLogic::TriState sim_CounterFast();

		uint8_t paired_sr_in[2]{};
		uint8_t paired_sr_out[2]{};
		void sim_PairedSRFast(uint8_t packed_nTX);

	public:
		FIFOLane(PPU* parent);
		~FIFOLane();

		void sim(BaseLogic::TriState HSel, BaseLogic::TriState n_TX[8], uint8_t packed_nTX, FIFOLaneOutput& ZOut);
	};

	class FIFO
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		bool fast_fifo = true;

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
