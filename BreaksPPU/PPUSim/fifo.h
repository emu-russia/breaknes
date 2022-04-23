// OAM FIFO (Motion picture buffer memory)

#pragma once

namespace PPUSim
{
	class FIFO_CounterBit
	{
		BaseLogic::FF keep_ff;
		BaseLogic::DLatch step_latch;

	public:
		void sim(BaseLogic::TriState Clock, BaseLogic::TriState Load, BaseLogic::TriState Step,
			BaseLogic::TriState val_in,
			BaseLogic::TriState carry_in, BaseLogic::TriState & carry_out);
	};

	class FIFO_SRBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState n_PCLK, BaseLogic::TriState T_SR, BaseLogic::TriState SR_EN,
			BaseLogic::TriState nTx, BaseLogic::TriState shift_in, BaseLogic::TriState & shift_out );
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
		FIFO_CounterBit down_cnt[8]{};

		BaseLogic::DLatch ob0_latch[2];
		BaseLogic::DLatch ob1_latch[2];
		BaseLogic::DLatch ob5_latch[2];
		BaseLogic::DLatch hsel_latch;

		BaseLogic::TriState nZ_COL0;
		BaseLogic::TriState nZ_COL1;
		BaseLogic::TriState Z_COL2;
		BaseLogic::TriState Z_COL3;
		BaseLogic::TriState nZ_PRIO;
		BaseLogic::TriState SR_EN;
		BaseLogic::TriState LOAD;
		BaseLogic::TriState T_SR0;
		BaseLogic::TriState T_SR1;

		BaseLogic::DLatch zh_latch1;
		BaseLogic::DLatch zh_latch2;
		BaseLogic::DLatch zh_latch3;
		BaseLogic::FF ZH_FF;
		BaseLogic::DLatch en_latch;

		BaseLogic::TriState UPD;
		BaseLogic::TriState STEP;
		BaseLogic::TriState n_EN;

		void sim_Enable();
		void sim_LaneControl(BaseLogic::TriState HSel);
		void sim_CounterControl();
		void sim_PairedSR(BaseLogic::TriState n_TX[8]);
		BaseLogic::TriState sim_Counter();
		void sim_CounterCarry(BaseLogic::TriState Carry);

	public:
		FIFOLane(PPU* parent) { ppu = parent; }
		~FIFOLane() {}

		void sim(BaseLogic::TriState HSel, BaseLogic::TriState n_TX[8], BaseLogic::TriState ZOut[(size_t)FIFOLaneOutput::Max]);
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

		BaseLogic::DLatch s0_latch;
		BaseLogic::DLatch col2_latch;
		BaseLogic::DLatch col3_latch;
		BaseLogic::DLatch prio_latch;

		BaseLogic::TriState LaneOut[8][(size_t)FIFOLaneOutput::Max]{};

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
