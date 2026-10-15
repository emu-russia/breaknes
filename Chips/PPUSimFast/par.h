// Picture Address Register

#pragma once

namespace PPUSimFast
{
	// The two transparent banks are distinct even though the data is packed.
	struct PatternRowPipeline
	{
		uint8_t input = 0, output = 0;
		uint8_t sim(BaseLogic::TriState nPclk, BaseLogic::TriState capture,
			BaseLogic::TriState invert, uint8_t row)
		{
			if (nPclk == BaseLogic::One) input = row & 15;
			if (capture == BaseLogic::One) output = uint8_t(~input) & 15;
			return output ^ ((invert & 1) ? 15 : 0);
		}
	};
	struct PatternBytePipeline
	{
		uint8_t pd_input = 0, pd_output = 0, object = 0, address = 0;
		uint8_t sim(BaseLogic::TriState nPclk, BaseLogic::TriState capture,
			BaseLogic::TriState objectRead, BaseLogic::TriState sprite16,
			uint8_t pd, uint8_t ob, uint8_t row)
		{
			if (nPclk == BaseLogic::One) pd_input = uint8_t(~pd);
			if (capture == BaseLogic::One) { pd_output = uint8_t(~pd_input); object = ob; }
			if (nPclk == BaseLogic::One)
			{
				address = uint8_t(~((objectRead & 1) ? object : pd_output));
				if ((objectRead & 1) && (sprite16 & 1)) address = (address & 0xfe) | ((row >> 3) & 1);
			}
			return uint8_t(~address);
		}
	};
	class ParBitInv
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState n_PCLK, BaseLogic::TriState O, BaseLogic::TriState INV, BaseLogic::TriState val_in,
			BaseLogic::TriState& val_out);
	};

	class ParBit
	{
		BaseLogic::DLatch pdin_latch;
		BaseLogic::DLatch pdout_latch;
		BaseLogic::DLatch ob_latch;
		BaseLogic::DLatch padx_latch;

	public:
		void sim(BaseLogic::TriState n_PCLK, BaseLogic::TriState O, BaseLogic::TriState val_OB, BaseLogic::TriState val_PD, BaseLogic::TriState OBJ_READ,
			BaseLogic::TriState& PADx);
	};

	class PAR
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		PatternRowPipeline rows{};
		PatternBytePipeline tiles{};
		uint8_t fine = 0;
		uint16_t address = 0;

		BaseLogic::DLatch fnt_latch;
		BaseLogic::DLatch ob0_latch;
		BaseLogic::DLatch pad12_latch;

		BaseLogic::FF VINV_FF;

		BaseLogic::TriState O = BaseLogic::TriState::X;
		BaseLogic::TriState VINV = BaseLogic::TriState::X;

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_Control(const EdgeContextT<Phase>& c);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_VInv(const EdgeContextT<Phase>& c);

	public:
		PAR(PPU* parent);
		~PAR();

		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim(const EdgeContextT<Phase>& c);
		uint16_t get_address() const { return address; }
	};
}
