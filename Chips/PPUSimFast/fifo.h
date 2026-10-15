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

		// Eight byte lanes in each word. Control and attribute bits are lane masks.
		uint64_t counter_keep = 0, counter_prepared = 0;
		uint64_t shift_input[2]{}, shift_output[2]{};
		uint8_t attr_input[3]{}, attr_output[3]{};
		uint8_t selected = 0, zero_position = 0, enabled = 0;
		uint8_t captured_pd = 0;
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_PackedLanes(const EdgeContextT<Phase>& c);
		// Field-driven entry points for the kernel tests.
		void sim_PackedLanes();
		void sim_Prio();

		BaseLogic::DLatch zh_latch1;
		BaseLogic::DLatch zh_latch2;
		BaseLogic::DLatch zh_latch3;

		BaseLogic::FF HINV_FF;
		uint8_t packed_nTX = 0;

		uint8_t sprite_reads = 0;

		BaseLogic::DLatch s0_latch;
		BaseLogic::DLatch col2_latch;
		BaseLogic::DLatch col3_latch;
		BaseLogic::DLatch prio_latch;


		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_HInv(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_Lanes(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_Prio(EdgeContextT<Phase>& c);


	public:
		FIFO(PPU* parent);
		~FIFO();

		// Consumes the FSM commands, delayed H, PD/FIFO and the /OBJ_RD strobes; produces /ZH, the
		// Z colour/priority lanes and /SPR0HIT into the context (and the wires).
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim(EdgeContextT<Phase>& c);

		/// <summary>
		/// You can call right after the FSM. Produces the four /OBJ_RD strobes into the context (and the wires).
		/// </summary>
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_SpriteH(EdgeContextT<Phase>& c);
	};
}
