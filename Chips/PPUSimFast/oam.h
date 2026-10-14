// Byte-oriented OAM transactions with the original retained buffer stages.
#pragma once
namespace PPUSimFast
{
	class OAM
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;
		struct ByteCell { uint8_t value = 0; size_t saved = static_cast<size_t>(-1); size_t timeout = 0; };
		ByteCell cells[9][32]{};
		uint8_t input_ff = 0, ob_ff = 0, r4_latch = 0, out_latch = 0;
		BaseLogic::FF OFETCH_FF, W4_FF;
		BaseLogic::DLatch latch[4];
		OAMDecayBehavior decay_behav = OAMDecayBehavior::Keep;
		void sim_OFETCH_Default();
		void sim_OFETCH_RGB_PPU();
		uint8_t Read(size_t col, size_t row, uint8_t& driven);
		void Write(size_t col, size_t row, uint8_t value);
		static uint8_t Mask(size_t col) { return col == 2 || col == 6 ? 0xe3 : 0xff; }
	public:
		OAM(PPU* parent) : ppu(parent) {}
		~OAM() = default;
		void sim();
		void sim_OFETCH();
		BaseLogic::TriState get_OB(size_t n);
		uint8_t get_OBByte() const { return ob_ff; }
		void set_OB(size_t n, BaseLogic::TriState val);
		uint8_t Dbg_OAMReadByte(size_t addr);
		uint8_t Dbg_TempOAMReadByte(size_t addr);
		void Dbg_OAMWriteByte(size_t addr, uint8_t val);
		void Dbg_TempOAMWriteByte(size_t addr, uint8_t val);
		void SetOamDecayBehavior(OAMDecayBehavior behavior) { decay_behav = behavior; }
		OAMDecayBehavior GetOamDecayBehavior() { return decay_behav; }
		uint32_t Dbg_Get_OAMBuffer() { return ob_ff; }
		void Dbg_Set_OAMBuffer(uint32_t value) { ob_ff = static_cast<uint8_t>(value); }
	};
}
