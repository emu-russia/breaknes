// Sprite Comparison

#pragma once

namespace PPUSimFast
{
	class OAMCounterBit
	{
		BaseLogic::DLatch cnt_latch;
		BaseLogic::FF keep_ff;

	public:
		BaseLogic::TriState sim(
			BaseLogic::TriState Clock,
			BaseLogic::TriState Load,
			BaseLogic::TriState Step,
			BaseLogic::TriState BlockCount,
			BaseLogic::TriState Reset,
			BaseLogic::TriState val_in,
			BaseLogic::TriState carry_in,
			BaseLogic::TriState & val_out,
			BaseLogic::TriState & n_val_out );

		BaseLogic::TriState get();
		void set(BaseLogic::TriState value);
	};

	class OAMCmprBit
	{
	public:
		BaseLogic::TriState sim(
			BaseLogic::TriState OB_Even,
			BaseLogic::TriState V_Even,
			BaseLogic::TriState OB_Odd,
			BaseLogic::TriState V_Odd,
			BaseLogic::TriState carry_in,
			BaseLogic::TriState & OV_Even,
			BaseLogic::TriState & OV_Odd );
	};

	class OAMPosedgeDFFE
	{
		BaseLogic::FF ff;

	public:
		void sim(
			BaseLogic::TriState CLK,
			BaseLogic::TriState n_EN,
			BaseLogic::TriState val_in,
			BaseLogic::TriState & Q,
			BaseLogic::TriState & n_Q );
	};

	class ObjEval
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		struct CounterBank { uint8_t keep=0, latch=0; } main_counter, temp_counter;
		uint8_t latched_y = 0, row = 0, address = 0xff;

		BaseLogic::TriState OAM_x[8]{};
		BaseLogic::TriState OAM_Temp[5]{};
		BaseLogic::TriState OMSTEP = BaseLogic::TriState::X;
		BaseLogic::TriState OMOUT = BaseLogic::TriState::X;
		BaseLogic::TriState ORES = BaseLogic::TriState::X;
		BaseLogic::TriState OSTEP = BaseLogic::TriState::X;
		BaseLogic::TriState OVZ = BaseLogic::TriState::X;
		BaseLogic::TriState OMFG = BaseLogic::TriState::X;
		BaseLogic::TriState OMV = BaseLogic::TriState::X;
		BaseLogic::TriState TMV = BaseLogic::TriState::X;
		BaseLogic::TriState COPY_STEP = BaseLogic::TriState::X;
		BaseLogic::TriState DO_COPY = BaseLogic::TriState::X;
		BaseLogic::TriState COPY_OVF = BaseLogic::TriState::X;
		BaseLogic::TriState W3_Enable = BaseLogic::TriState::X;

		// For PAL PPU
		BaseLogic::DLatch blnk_latch;
		BaseLogic::FF W3_FF1;
		BaseLogic::FF W3_FF2;
		BaseLogic::DLatch w3_latch1;
		BaseLogic::DLatch w3_latch2;
		BaseLogic::DLatch w3_latch3;
		BaseLogic::DLatch w3_latch4;

		BaseLogic::DLatch init_latch;
		BaseLogic::DLatch ofetch_latch;
		BaseLogic::DLatch omv_latch;
		BaseLogic::DLatch eval_latch;
		BaseLogic::DLatch tmv_latch;
		BaseLogic::DLatch nomfg_latch;
		BaseLogic::DLatch ioam2_latch;
		BaseLogic::DLatch temp_latch1;
		BaseLogic::DLatch omfg_latch;
		BaseLogic::DLatch setov_latch;
		BaseLogic::FF OAMCTR2_FF;
		BaseLogic::FF SPR_OV_REG_FF;
		BaseLogic::FF SPR_OV_FF;

		OAMPosedgeDFFE eval_FF1;
		OAMPosedgeDFFE eval_FF2;
		OAMPosedgeDFFE eval_FF3;
		BaseLogic::DLatch fnt_latch;
		BaseLogic::DLatch novz_latch;
		BaseLogic::DLatch i2_latch[6];


		// The blocks are simulated in the signal propagation order as the developers intended or we think they intended.

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_StepJohnson(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_Comparator(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_ComparisonFSM(EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_MainCounterControl(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_MainCounter(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_TempCounterControlBeforeCounter(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_TempCounter(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_TempCounterControlAfterCounter(EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_SpriteOVF(EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_OAMAddress(EdgeContextT<Phase>& c);
		// Field-driven entry points for the kernel tests.
		void sim_Comparator();
		void sim_MainCounter();
		void sim_TempCounter();

		// These auxiliary methods are needed to retrieve old values of signals that have not yet been simulated (`uroboros` signals).

		BaseLogic::TriState get_SPR_OV();

	public:
		ObjEval(PPU* parent);
		~ObjEval();

		// Consumes the FSM commands, delayed H, OFETCH and the previous OAM buffer; produces OAM8,
		// OAMCTR2, SPR_OV, /SPR0_EV and PD/FIFO into the context (and the wires).
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim(EdgeContextT<Phase>& c);
		uint8_t get_Row() const { return row; }
		uint8_t get_Address() const { return address; }

		uint32_t Debug_GetMainCounter();
		uint32_t Debug_GetTempCounter();

		void Debug_SetMainCounter(uint32_t value);
		void Debug_SetTempCounter(uint32_t value);

		void GetDebugInfo(OAMEvalWires& wires);
	};
}
