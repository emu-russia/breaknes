// Background Color (BG COL)

#pragma once

namespace PPUSim
{
	class BGC_SRBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState shift_in, BaseLogic::TriState val_in,
			BaseLogic::TriState Load, BaseLogic::TriState Step, BaseLogic::TriState Next,
			BaseLogic::TriState & shift_out);
	};

	class BGC_SR8
	{
		friend PPUSimUnitTest::UnitTest;

		BGC_SRBit sr[8]{};

	public:
		void sim(BaseLogic::TriState val[8], BaseLogic::TriState sin, 
			BaseLogic::TriState Load, BaseLogic::TriState Step, BaseLogic::TriState Next,
			BaseLogic::TriState sout[8]);
	};

	class BGCol
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		BaseLogic::DLatch fat_latch;
		BaseLogic::DLatch tho1_latch;

		BaseLogic::DLatch clpb_latch;
		BaseLogic::DLatch bgc0_latch[2];
		BaseLogic::DLatch bgc1_latch[2];
		BaseLogic::DLatch bgc2_latch[2];
		BaseLogic::DLatch bgc3_latch[2];

		BaseLogic::TriState PD_SR = BaseLogic::TriState::X;
		BaseLogic::TriState SRLOAD = BaseLogic::TriState::X;
		BaseLogic::TriState STEP = BaseLogic::TriState::X;
		BaseLogic::TriState STEP2 = BaseLogic::TriState::X;
		BaseLogic::TriState PD_SEL = BaseLogic::TriState::X;
		BaseLogic::TriState NEXT = BaseLogic::TriState::X;
		BaseLogic::TriState H01 = BaseLogic::TriState::X;

		BaseLogic::DLatch BGC0_Latch[8]{};
		BGC_SR8 BGC0_SR1;
		BGC_SR8 BGC0_SR2;

		BGC_SR8 BGC1_SR1;
		BGC_SR8 BGC1_SR2;

		BaseLogic::DLatch pd_latch[8]{};

		BGC_SRBit BGC2_SRBit1;
		BGC_SR8 BGC2_SR1;

		BGC_SRBit BGC3_SRBit1;
		BGC_SR8 BGC3_SR1;

		BaseLogic::TriState n_BGC0_Out = BaseLogic::TriState::X;
		BaseLogic::TriState BGC1_Out = BaseLogic::TriState::X;
		BaseLogic::TriState n_BGC2_Out = BaseLogic::TriState::X;
		BaseLogic::TriState n_BGC3_Out = BaseLogic::TriState::X;

		BaseLogic::TriState unused[8]{};

		void sim_Control();
		void sim_BGC0();
		void sim_BGC1();
		void sim_BGC2();
		void sim_BGC3();
		void sim_Output();

	public:
		BGCol(PPU* parent);
		~BGCol();

		void sim();
	};
}
