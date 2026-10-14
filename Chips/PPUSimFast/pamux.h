// PPU Address Mux

#pragma once

namespace PPUSimFast
{
	struct AddressMuxPipeline
	{
		uint16_t input = 0, output = 0;
		void sim(BaseLogic::TriState clock, BaseLogic::TriState pattern,
			BaseLogic::TriState data, BaseLogic::TriState lowName,
			BaseLogic::TriState highName, BaseLogic::TriState attribute,
			uint16_t at, uint16_t nt, uint16_t pat, uint8_t db)
		{
			// No selected source is the original Z input: retain that bank.
			if (data & 1) input = (input & 0x3f00) | db;
			else if (pattern & 1) input = (input & 0x3f00) | (pat & 255);
			else if (lowName & 1) input = (input & 0x3f00) | (nt & 255);
			else if (attribute & 1) input = (input & 0x3f00) | (at & 255);
			if (pattern & 1) input = (input & 255) | (pat & 0x3f00);
			else if (highName & 1) input = (input & 255) | (nt & 0x3f00);
			else if (attribute & 1) input = (input & 255) | (at & 0x3f00);
			if (clock == BaseLogic::One) output = uint16_t(~input) & 0x3fff;
		}
	};
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

		AddressMuxPipeline pipeline{};

		BaseLogic::TriState PARR = BaseLogic::TriState::X;
		BaseLogic::TriState PAH = BaseLogic::TriState::X;
		BaseLogic::TriState PAL = BaseLogic::TriState::X;

		uint16_t AT_ADR = 0, NT_ADR = 0, PAT_ADR = 0;

		void sim_Control();

	public:
		PAMUX(PPU* parent);
		~PAMUX();

		void sim();

		void sim_MuxInputs();
		void sim_MuxOutputs();
		uint16_t get_n_address() const { return pipeline.output; }
	};
}
