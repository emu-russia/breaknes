// A dummy device that uses the 6502 as a processor.
// Contains 64 Kbytes of memory and nothing else.

#pragma once

namespace Breaknes
{
	struct BogusBoardDebugInfo
	{
		uint32_t n_NMI;
		uint32_t n_IRQ;
		uint32_t n_RES;
		uint32_t PHI0;
		uint32_t RDY;
		uint32_t SO;
		uint32_t PHI1;
		uint32_t PHI2;
		uint32_t RnW;
		uint32_t SYNC;
		uint32_t A;
		uint32_t D;
	};

	class BogusBoard : public Board
	{
		BaseBoard::SRAM* wram = nullptr;
		const size_t wram_bits = 16;
		const size_t wram_size = 1ULL << wram_bits;

		static uint8_t DumpWRAM(void* opaque, size_t addr);
		static void WriteWRAM(void* opaque, size_t addr, uint8_t data);

		static uint32_t GetCoreDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry);
		static uint32_t GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry);

		static void SetCoreDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);
		static void SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value);

		void AddBoardMemDescriptors();
		void AddDebugInfoProviders();

		void GetDebugInfo(BogusBoardDebugInfo& info);

		BaseLogic::TriState n_NMI;
		BaseLogic::TriState n_IRQ;
		BaseLogic::TriState n_RES;
		BaseLogic::TriState RDY;
		BaseLogic::TriState SO;
		BaseLogic::TriState PHI1;
		BaseLogic::TriState PHI2;
		BaseLogic::TriState RnW;
		BaseLogic::TriState SYNC;

		BaseLogic::TriState PrevCLK = BaseLogic::TriState::X;
		size_t phi_counter = 0;

	public:
		BogusBoard(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, Mappers::ConnectorType p1);
		virtual ~BogusBoard();

		void Step() override;
		
		size_t GetPHICounter() override;
	};
}
