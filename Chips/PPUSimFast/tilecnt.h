// Tile Counters (nesdev `v`)

#pragma once

namespace PPUSimFast
{
	template<unsigned Width> struct PackedTileCounter
	{
		static_assert(Width > 0 && Width <= 8, "tile counter must fit a byte");
		static constexpr unsigned Mask = (1u << Width) - 1;
		uint8_t main = 0, prepared = 0;
		BaseLogic::TriState sim(BaseLogic::TriState clock, BaseLogic::TriState load,
			BaseLogic::TriState step, uint8_t value, BaseLogic::TriState carry,
			BaseLogic::TriState reset, uint8_t& visible, uint8_t& complement)
		{
			if (step & 1) main = uint8_t(~prepared) & Mask;
			else if (load & 1) main = value & Mask;
			else if ((clock & 1) && reset != BaseLogic::Zero) main = 0;
			if (clock == BaseLogic::One)
				prepared = uint8_t(~(unsigned(main) + (carry == BaseLogic::One))) & Mask;
			// The TV reset pulls down the direct output, not its complement or
			// carry. They can intentionally disagree while reset is asserted.
			visible = reset == BaseLogic::Zero ? main : 0;
			complement = uint8_t(~main) & Mask;
			return carry == BaseLogic::One && main == Mask ? BaseLogic::One : BaseLogic::Zero;
		}
	};
	class TileCounterBit
	{
		BaseLogic::FF ff;
		BaseLogic::DLatch step_latch;

	public:
		BaseLogic::TriState sim(BaseLogic::TriState Clock, BaseLogic::TriState Load, BaseLogic::TriState Step,
			BaseLogic::TriState val_in, BaseLogic::TriState carry_in,
			BaseLogic::TriState& val_out, BaseLogic::TriState& n_val_out);

		BaseLogic::TriState sim_res(BaseLogic::TriState Clock, BaseLogic::TriState Load, BaseLogic::TriState Step,
			BaseLogic::TriState val_in, BaseLogic::TriState carry_in, BaseLogic::TriState Reset,
			BaseLogic::TriState& val_out, BaseLogic::TriState& n_val_out);
	};

	class TileCnt
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		BaseLogic::DLatch w62_latch;
		BaseLogic::FF W62_FF1;
		BaseLogic::FF W62_FF2;
		BaseLogic::DLatch sccnt_latch;
		BaseLogic::DLatch eev_latch1;
		BaseLogic::DLatch eev_latch2;
		BaseLogic::DLatch tvz_latch1;
		BaseLogic::DLatch tvz_latch2;
		BaseLogic::DLatch tvstep_latch;

		PackedTileCounter<3> FVCounter;
		PackedTileCounter<1> NTHCounter, NTVCounter;
		PackedTileCounter<5> TVCounter, THCounter;

		BaseLogic::TriState THZB = BaseLogic::TriState::X;
		BaseLogic::TriState THZ = BaseLogic::TriState::X;
		BaseLogic::TriState TVZB = BaseLogic::TriState::X;
		BaseLogic::TriState TVZ = BaseLogic::TriState::X;
		BaseLogic::TriState FVZ = BaseLogic::TriState::X;

		BaseLogic::TriState TVLOAD = BaseLogic::TriState::X;
		BaseLogic::TriState THLOAD = BaseLogic::TriState::X;
		BaseLogic::TriState TVSTEP = BaseLogic::TriState::X;
		BaseLogic::TriState THSTEP = BaseLogic::TriState::X;

		BaseLogic::TriState NTHIN = BaseLogic::TriState::X;
		BaseLogic::TriState NTVIN = BaseLogic::TriState::X;
		BaseLogic::TriState FVIN = BaseLogic::TriState::X;
		BaseLogic::TriState TVIN = BaseLogic::TriState::X;
		BaseLogic::TriState THIN = BaseLogic::TriState::X;
		BaseLogic::TriState Z_TV = BaseLogic::TriState::X;

		uint8_t th = 0, tv = 0, fv = 0, nth = 0, ntv = 0;
		uint8_t n_th = 0, n_tv = 0, n_fv = 0;
		BaseLogic::TriState NTHO = BaseLogic::TriState::X;
		BaseLogic::TriState NTVO = BaseLogic::TriState::X;

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_CountersControl(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_CountersCarry(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_FVCounter(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_NTCounters(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_TVCounter(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_THCounter(const EdgeContextT<Phase>& c);
		// Field-driven entry point for the kernel tests.
		void sim_CountersCarry();

	public:
		TileCnt(PPU* parent);
		~TileCnt();

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim(const EdgeContextT<Phase>& c);
		uint8_t get_n_FV() const { return n_fv; }
		uint8_t get_TH() const { return th; }
		uint8_t get_TV() const { return tv; }
		uint8_t get_FV() const { return fv; }
		uint8_t get_NTH() const { return nth; }
		uint8_t get_NTV() const { return ntv; }
	};
}
