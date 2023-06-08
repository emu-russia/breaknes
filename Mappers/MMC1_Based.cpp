// SxROM
#include "pch.h"

using namespace BaseLogic;

namespace Mappers
{
	MMC1_Based::MMC1_Based(ConnectorType p1, uint8_t* nesImage, size_t nesImageSize) : AbstractCartridge(p1, nesImage, nesImageSize)
	{
		printf("MMC1_Based::MMC1_Based()\n");

		mmc = new MMC1();
	}

	MMC1_Based::~MMC1_Based()
	{
		printf("MMC1_Based::~MMC1_Based()\n");

		delete mmc;

		if (PRG != nullptr)
			delete PRG;

		if (CHR != nullptr)
			delete CHR;

		if (RAM != nullptr)
			delete RAM;
	}

	bool MMC1_Based::Valid()
	{
		return valid;
	}

	void MMC1_Based::sim(
		BaseLogic::TriState cart_in[(size_t)CartInput::Max],
		BaseLogic::TriState cart_out[(size_t)CartOutput::Max],
		uint16_t cpu_addr,
		uint8_t* cpu_data, bool& cpu_data_dirty,
		uint16_t ppu_addr,
		uint8_t* ppu_data, bool& ppu_data_dirty,
		// Famicom only
		CartAudioOutSignal* snd_out,
		// NES only
		uint16_t* exp, bool& exp_dirty)
	{
		// MMC1

		TriState mmc1_in[(size_t)MMC1_Input::Max]{};
		TriState mmc1_out[(size_t)MMC1_Output::Max]{};

		mmc1_in[(size_t)MMC1_Input::M2] = cart_in[(size_t)CartInput::M2];
		
		mmc1_in[(size_t)MMC1_Input::CPU_RnW] = cart_in[(size_t)CartInput::RnW];
		mmc1_in[(size_t)MMC1_Input::CPU_A13] = FromByte((cpu_addr >> 13) & 1);
		mmc1_in[(size_t)MMC1_Input::CPU_A14] = FromByte((cpu_addr >> 14) & 1);
		mmc1_in[(size_t)MMC1_Input::CPU_D0] = FromByte((*cpu_data >> 0) & 1);
		mmc1_in[(size_t)MMC1_Input::CPU_D7] = FromByte((*cpu_data >> 7) & 1);
		mmc1_in[(size_t)MMC1_Input::PPU_A10] = FromByte((ppu_addr >> 10) & 1);
		mmc1_in[(size_t)MMC1_Input::PPU_A11] = FromByte((ppu_addr >> 11) & 1);
		mmc1_in[(size_t)MMC1_Input::PPU_A12] = FromByte((ppu_addr >> 12) & 1);
		mmc1_in[(size_t)MMC1_Input::nROMSEL] = cart_in[(size_t)CartInput::nROMSEL];

		mmc->sim(mmc1_in, mmc1_out);

		//CHR_A12 = 0,
		//CHR_A13,
		//CHR_A14,
		//CHR_A15,
		//CHR_A16,
		//PRG_A14,
		//PRG_A15,
		//PRG_A16,
		//PRG_A17,
		//SRAM_CE,
		//PRG_nCE,

		cart_out[(size_t)CartOutput::VRAM_A10] = mmc1_out[(size_t)MMC1_Output::CIRAM_A10];

		// Contains a jumper between `/PA13` and `/VRAM_CS` (? probably, just don't care for now)
		cart_out[(size_t)CartOutput::VRAM_nCS] = cart_in[(size_t)CartInput::nPA13];

		// Memory operations

		if (p1_type == ConnectorType::FamicomStyle && snd_out)
		{
			snd_out->normalized = 0.0f;
		}
	}
}
