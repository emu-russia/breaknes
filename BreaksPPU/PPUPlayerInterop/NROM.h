// Simple Mapper Simulator (0) - NROM.

#pragma once

namespace PPUPlayer
{
	class NROM
	{
		bool valid = false;

		uint8_t* PRG = nullptr;
		size_t PRGSize = 0;

		uint8_t* CHR = nullptr;
		size_t CHRSize = 0;

	public:
		NROM(uint8_t* nesImage, size_t nesImageSize);
		~NROM();

		void sim(BaseLogic::TriState PA[14], BaseLogic::TriState n_PA13, BaseLogic::TriState n_RD, BaseLogic::TriState n_WR, 
			uint8_t *PD, bool &PDDirty,
			BaseLogic::TriState& n_VRAM_CS, BaseLogic::TriState& VRAM_A10);
	};
}
