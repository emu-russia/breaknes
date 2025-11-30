// PPU Address Mux

#pragma once

namespace PPUSim
{
	class PAMUX_LowBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState PCLK, BaseLogic::TriState PARR, BaseLogic::TriState DB_PAR, BaseLogic::TriState PAL, BaseLogic::TriState F_AT,
			BaseLogic::TriState AT_ADR, BaseLogic::TriState NT_ADR, BaseLogic::TriState PAT_ADR, BaseLogic::TriState DB_in,
			BaseLogic::TriState& n_PAx);
	};

	class PAMUX_HighBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState PCLK, BaseLogic::TriState PARR, BaseLogic::TriState PAH, BaseLogic::TriState F_AT,
			BaseLogic::TriState AT_ADR, BaseLogic::TriState NT_ADR, BaseLogic::TriState PAT_ADR,
			BaseLogic::TriState& n_PAx);
	};

	class PAMUX
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		PAMUX_LowBit par_lo[8]{};
		PAMUX_HighBit par_hi[6]{};

		BaseLogic::TriState PARR = BaseLogic::TriState::X;
		BaseLogic::TriState PAH = BaseLogic::TriState::X;
		BaseLogic::TriState PAL = BaseLogic::TriState::X;

		BaseLogic::TriState AT_ADR[14]{};
		BaseLogic::TriState NT_ADR[14]{};
		BaseLogic::TriState PAT_ADR[14]{};

		void sim_Control();

	public:
		PAMUX(PPU* parent);
		~PAMUX();

		void sim();

		void sim_MuxInputs();
		void sim_MuxOutputs();
	};
}