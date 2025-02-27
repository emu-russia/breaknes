// OAM

#pragma once

namespace PPUSim
{
	enum class OAMCellTopology
	{
		TopLeft = 0,
		TopRight,
		BottomLeft,
		BottomRight,
		Middle,
	};

	/// <summary>
	/// OAM Cell. The decay simulation is done simply by the PCLK counter. 
	/// If a cell has not been updated for a long time, its value "fades away" (becomes equal to `z`).
	/// </summary>
	class OAMCell
	{
		PPU* ppu = nullptr;
		BaseLogic::FF decay_ff;

		// The value of the global PCLK counter at the time of writing to the cell.
		// Initially, all cells are in limbo, since there is no drive on them.
		size_t savedPclk = (size_t)-1;

		// Timeout after which the cell value "fades away".
		size_t pclksToDecay = 0;

		void DetermineDecay();

		// Determines the geometric location of the cell in the bank. May be useful for determining the decay time.
		OAMCellTopology topo = OAMCellTopology::Middle;

		// Bank number (0: upper OAM bank, 1: lower OAM bank).
		size_t bank = 0;

	public:
		OAMCell(PPU* parent) { ppu = parent; }
		~OAMCell() {}

		BaseLogic::TriState get();
		void set(BaseLogic::TriState val);

		void SetTopo(OAMCellTopology place, size_t bank_num);
	};

	class OAMLane
	{
		PPU* ppu = nullptr;

		static const size_t cells_per_lane = 32 * 8;	// 32 Rows x 8 Bits

		OAMCell* cells[cells_per_lane];

		bool skip_attr_bits = false;

	public:
		OAMLane(PPU *parent, bool SkipAttrBits);
		~OAMLane();

		void sim(size_t Row, size_t bit_num, BaseLogic::TriState& inOut);
	};

	class OAMBufferBit
	{
	protected:
		PPU* ppu = nullptr;

		BaseLogic::FF Input_FF;
		BaseLogic::FF OB_FF;
		BaseLogic::DLatch R4_out_latch;
		BaseLogic::DLatch out_latch;

	public:
		OAMBufferBit(PPU* parent) { ppu = parent; }
		~OAMBufferBit() {}

		virtual void sim(OAMLane *lane, size_t row, size_t bit_num, BaseLogic::TriState OB_OAM, BaseLogic::TriState n_WE);

		BaseLogic::TriState get();
		void set(BaseLogic::TriState val);
	};

	/// <summary>
	/// Special version for RGB PPU.
	/// </summary>
	class OAMBufferBit_RGB : public OAMBufferBit
	{
	public:
		OAMBufferBit_RGB(PPU* parent) : OAMBufferBit(parent) {}
		~OAMBufferBit_RGB() {}

		virtual void sim(OAMLane* lane, size_t row, size_t bit_num, BaseLogic::TriState OB_OAM, BaseLogic::TriState n_WE);
	};

	class OAM
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		static const size_t num_lanes = 8 + 1;	// 0-7: OAM, 8: TempOAM
		OAMLane* lane[num_lanes];

		OAMBufferBit *ob[8];

		BaseLogic::FF OFETCH_FF;
		BaseLogic::FF W4_FF;
		BaseLogic::DLatch latch[4];

		BaseLogic::TriState OB_OAM = BaseLogic::TriState::X;

		// The physical location of the row on the chip.
		size_t ROW = 0;

		OAMLane* sim_AddressDecoder();
		void sim_OBControl();
		void sim_OB(OAMLane *lane);

		size_t RowMap(size_t n);

		OAMDecayBehavior decay_behav = OAMDecayBehavior::Keep;

		void sim_OFETCH_Default();
		void sim_OFETCH_RGB_PPU();

	public:
		OAM(PPU* parent);
		~OAM();

		void sim();

		void sim_OFETCH();

		BaseLogic::TriState get_OB(size_t bit_num);
		void set_OB(size_t bit_num, BaseLogic::TriState val);

		uint8_t Dbg_OAMReadByte(size_t addr);
		uint8_t Dbg_TempOAMReadByte(size_t addr);
		void Dbg_OAMWriteByte(size_t addr, uint8_t val);
		void Dbg_TempOAMWriteByte(size_t addr, uint8_t val);

		void SetOamDecayBehavior(OAMDecayBehavior behavior);
		OAMDecayBehavior GetOamDecayBehavior();

		uint32_t Dbg_Get_OAMBuffer();
		void Dbg_Set_OAMBuffer(uint32_t value);
	};
}
