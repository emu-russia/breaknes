// Simple Mapper Simulator (0) - NROM.

#pragma once

namespace PPUPlayer
{
	struct NROM_DebugInfo
	{
		uint32_t last_PA;
		uint32_t last_nRD;
		uint32_t last_nWR;
	};

	class NROM
	{
		bool valid = false;

		uint8_t* PRG = nullptr;
		size_t PRGSize = 0;

		uint8_t* CHR = nullptr;
		size_t CHRSize = 0;

		NROM_DebugInfo nrom_debug{};

	public:
		NROM(uint8_t* nesImage, size_t nesImageSize);
		~NROM();

		void sim(BaseLogic::TriState PA[14], BaseLogic::TriState n_PA13, BaseLogic::TriState n_RD, BaseLogic::TriState n_WR, 
			uint8_t *PD, bool &PDDirty,
			BaseLogic::TriState& n_VRAM_CS, BaseLogic::TriState& VRAM_A10);

		size_t Dbg_GetCHRSize();

		uint8_t Dbg_ReadCHRByte(size_t addr);

		void GetDebugInfo(NROM_DebugInfo & info);
	};
}
