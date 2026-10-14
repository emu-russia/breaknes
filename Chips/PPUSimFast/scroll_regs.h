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

	public:
		ScrollRegs(PPU* parent);
		~ScrollRegs();

		void sim();
		BaseLogic::TriState get_NTH() const { return static_cast<BaseLogic::TriState>((address >> 10) & 1); }
		BaseLogic::TriState get_NTV() const { return static_cast<BaseLogic::TriState>((address >> 11) & 1); }
		void set_NTH(BaseLogic::TriState value) { address = static_cast<uint16_t>((address & ~0x0400) | (value == BaseLogic::One ? 0x0400 : 0)); }
		void set_NTV(BaseLogic::TriState value) { address = static_cast<uint16_t>((address & ~0x0800) | (value == BaseLogic::One ? 0x0800 : 0)); }
	};
}
