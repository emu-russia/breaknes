// Background Color (BG COL)

#pragma once

namespace PPUSimFast
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

		uint8_t input = 0, output = 0;

	public:
		uint8_t sim_Packed(uint8_t value, BaseLogic::TriState sin, BaseLogic::TriState Load, BaseLogic::TriState Step, BaseLogic::TriState Next);

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
		uint8_t captured_pattern = 0, captured_attribute = 0;
		uint8_t pixel_input = 0, pixel_output = 0, visible_pixel = 0;
		uint64_t bank_input = 0, bank_output = 0;
		uint8_t attribute_input = 0, attribute_output = 0;
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_Packed(const EdgeContextT<Phase>& c);
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

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_Control(const EdgeContextT<Phase>& c);
		void sim_BGC0();
		void sim_BGC1();
		void sim_BGC2();
		void sim_BGC3();
		void sim_Output();

	public:
		BGCol(PPU* parent);
		~BGCol();

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim(const EdgeContextT<Phase>& c);
		// Field-driven entry point for the kernel tests.
		void sim();
		uint8_t get_Pixel() const { return visible_pixel; }
	};
}
