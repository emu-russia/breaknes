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
		uint8_t input_ff = 0, ob_ff = 0, published_ob = 0, r4_latch = 0, out_latch = 0;
		BaseLogic::FF OFETCH_FF, W4_FF;
		BaseLogic::DLatch latch[4];
		OAMDecayBehavior decay_behav = OAMDecayBehavior::Keep;
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_OFETCH_Default(EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_OFETCH_RGB_PPU(EdgeContextT<Phase>& c);
		PPUSIM_FAST_EDGE_INLINE uint8_t Read(size_t col, size_t row, uint8_t& driven);
		PPUSIM_FAST_EDGE_INLINE void Write(size_t col, size_t row, uint8_t value);
		static uint8_t Mask(size_t col) { return col == 2 || col == 6 ? 0xe3 : 0xff; }
	public:
		OAM(PPU* parent) : ppu(parent) {}
		~OAM() = default;
		// Consumes the evaluation products (OAM8, OAMCTR2, SPR_OV) and the FSM commands from the context.
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim(const EdgeContextT<Phase>& c);
		// Produces OFETCH into the context (and the wire).
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_OFETCH(EdgeContextT<Phase>& c);
		BaseLogic::TriState get_OB(size_t n);
		uint8_t get_OBByte() const { return ob_ff; }
		uint8_t get_PublishedOB() const { return published_ob; }
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
