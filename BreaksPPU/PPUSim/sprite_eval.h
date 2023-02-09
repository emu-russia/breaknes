// Sprite Comparison

#pragma once

namespace PPUSim
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

	class OAMEval
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		OAMCounterBit MainCounter[8]{};
		OAMCounterBit TempCounter[5]{};
		OAMCmprBit cmpr[4]{};

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
		BaseLogic::TriState OB_Bits[8]{};
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

		BaseLogic::DLatch OB_latch[8];
		BaseLogic::DLatch ovz_latch;

		// The blocks are simulated in the signal propagation order as the developers intended or we think they intended.

		void sim_StepJohnson();
		void sim_Comparator();
		void sim_ComparisonFSM();
		void sim_MainCounterControl();
		void sim_MainCounter();
		void sim_TempCounterControlBeforeCounter();
		void sim_TempCounter();
		void sim_TempCounterControlAfterCounter();
		void sim_SpriteOVF();
		void sim_OAMAddress();

		// These auxiliary methods are needed to retrieve old values of signals that have not yet been simulated (`uroboros` signals).

		BaseLogic::TriState get_SPR_OV();

	public:
		OAMEval(PPU* parent);
		~OAMEval();

		void sim();

		uint32_t Debug_GetMainCounter();
		uint32_t Debug_GetTempCounter();

		void Debug_SetMainCounter(uint32_t value);
		void Debug_SetTempCounter(uint32_t value);

		void GetDebugInfo(OAMEvalWires& wires);
	};
}
