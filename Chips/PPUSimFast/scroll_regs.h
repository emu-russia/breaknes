// Scroll Registers (nesdev `t`)

#pragma once

namespace PPUSimFast
{
	class SCC_FF
	{
		BaseLogic::FF ff;

	public:
		void sim(BaseLogic::TriState val_in, BaseLogic::TriState n_DBE, BaseLogic::TriState RC, BaseLogic::TriState & val_out);
		BaseLogic::TriState get();
		void set(BaseLogic::TriState val);
	};

	class ScrollRegs
	{
		friend PPUSimUnitTest::UnitTest;
		friend ControlRegs;
		PPU* ppu = nullptr;

		uint16_t address = 0;
		uint8_t fine_x = 0;
		uint16_t published_address = 0;
		uint8_t published_fine_x = 0;

	public:
		ScrollRegs(PPU* parent);
		~ScrollRegs();

		PPUSIM_FAST_EDGE_INLINE void sim();
		uint8_t get_FH() const { return published_fine_x; }
		uint8_t get_FV() const { return uint8_t(published_address >> 12) & 7; }
		uint8_t get_TH() const { return uint8_t(published_address) & 31; }
		uint8_t get_TV() const { return uint8_t(published_address >> 5) & 31; }
		uint8_t get_PublishedNTH() const { return (published_address >> 10) & 1; }
		uint8_t get_PublishedNTV() const { return (published_address >> 11) & 1; }
		BaseLogic::TriState get_NTH() const { return static_cast<BaseLogic::TriState>((address >> 10) & 1); }
		BaseLogic::TriState get_NTV() const { return static_cast<BaseLogic::TriState>((address >> 11) & 1); }
		void set_NTH(BaseLogic::TriState value) { address = static_cast<uint16_t>((address & ~0x0400) | (value == BaseLogic::One ? 0x0400 : 0)); }
		void set_NTV(BaseLogic::TriState value) { address = static_cast<uint16_t>((address & ~0x0800) | (value == BaseLogic::One ? 0x0800 : 0)); }
	};
}
