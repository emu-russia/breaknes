// Palette byte transactions and the retained color/read pipeline.
#pragma once
namespace PPUSimFast
{
	class CRAM
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;
		BaseLogic::DLatch dbpar_latch;
		uint8_t palette[28]{};
		uint8_t color = 0, read_input = 0, read_output = 0;
		uint8_t luma[3]{}, chroma = 0;
		void sim_CRAMControl();
		static size_t MapRow(size_t row) { static constexpr uint8_t map[8]{0,4,2,6,0,3,1,5}; return map[row]; }
		static size_t Address(size_t addr) { return MapRow((addr & 3) | ((addr & 0x10) != 0 ? 4 : 0)) * 4 + ((addr >> 2) & 3); }
	public:
		CRAM(PPU* parent) : ppu(parent) {}
		~CRAM() = default;
		void sim();
		uint8_t Dbg_CRAMReadByte(size_t addr) { return palette[Address(addr)]; }
		void Dbg_CRAMWriteByte(size_t addr, uint8_t value) { palette[Address(addr)] = value & 0x3f; }
	};
}
