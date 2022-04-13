// Palette (Color RAM)

#pragma once

namespace PPUSim
{
	class CBBit
	{
		PPU* ppu = nullptr;

		BaseLogic::FF ff;
		BaseLogic::DLatch latch1;
		BaseLogic::DLatch latch2;

	public:
		CBBit(PPU* parent) { ppu = parent; }
		~CBBit() {}

		void sim(size_t bit_num, BaseLogic::TriState & cell, BaseLogic::TriState n_OE);

		BaseLogic::TriState get_CBOut(BaseLogic::TriState n_OE);
	};

	class CRAM
	{
		PPU* ppu = nullptr;

		BaseLogic::DLatch dbpar_latch;

		BaseLogic::DLatch LL0_latch[3];
		BaseLogic::DLatch LL1_latch[3];
		BaseLogic::DLatch CC_latch[4];

		static const size_t cb_num = 6;
		CBBit* cb[cb_num];

		void sim_CRAMControl();
		void sim_CRAMDecoder();
		void sim_ColorBuffer();

		/// <summary>
		/// The organization of CRAM is very intricate. Rows = 7 (Rows 0+4 are combined). Columns = 4.
		/// One lane is not a byte, but a `6-bit` (corresponds to the number of bits of the Color Buffer).
		/// </summary>
		static const size_t cram_lane_rows = 8;
		static const size_t cram_lane_cols = 4;
		BaseLogic::TriState cram[cram_lane_rows * cram_lane_cols * cb_num]{};

		BaseLogic::TriState COL[4]{};
		BaseLogic::TriState ROW[8]{};	// ROW[0] = ROW[4]

		/// <summary>
		/// This cell is used to address closed memory.
		/// </summary>
		BaseLogic::TriState z_cell = BaseLogic::TriState::Z;

		BaseLogic::TriState& AddressCell(size_t lane);

	public:
		CRAM(PPU* parent);
		~CRAM();

		void sim();
	};
}
