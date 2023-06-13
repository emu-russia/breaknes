// SxROM
#include "pch.h"

// It's all sad here. The NES format does not determine the connection of MMC1 with the rest of the cartridge components, so you have to "guess" and make dirty hacks.

using namespace BaseLogic;

namespace Mappers
{
	MMC1_Based::MMC1_Based(ConnectorType p1, uint8_t* nesImage, size_t nesImageSize) : AbstractCartridge(p1, nesImage, nesImageSize)
	{
		printf("MMC1_Based::MMC1_Based()\n");

		mmc = new MMC1();

		NESHeader* head = (NESHeader*)nesImage;

		bool trainer = (head->Flags_6 & 0b100) != 0;
		bool wram = (head->Flags_6 & 0b10) != 0;

		// Load CHR ROM

		if (head->CHRSize != 0)
		{
			CHRSize = head->CHRSize * 0x2000;
			CHR = new uint8_t[CHRSize];

			uint8_t* chrPtr = nesImage + sizeof(NESHeader) + (trainer ? NES_TRAINER_SIZE : 0) + head->PRGSize * 0x4000;
			memcpy(CHR, chrPtr, CHRSize);
		}
		else
		{
			CHRSize = 0x2000;
			CHR = new uint8_t[CHRSize];
			memset(CHR, 0, CHRSize);
		}

		// Load PRG ROM

		PRGSize = head->PRGSize * 0x4000;
		PRG = new uint8_t[PRGSize];

		uint8_t* prgPtr = nesImage + sizeof(NESHeader) + (trainer ? NES_TRAINER_SIZE : 0);
		memcpy(PRG, prgPtr, PRGSize);

		// Create WRAM

		if (wram) {
			RAMSize = 0x2000;
			RAM = new uint8_t[0x2000];
			memset(RAM, 0, RAMSize);
		}

		valid = true;
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
		if (!valid)
			return;

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

		// PPU Part

		TriState nRD = cart_in[(size_t)CartInput::nRD];
		TriState nWR = cart_in[(size_t)CartInput::nWR];

		cart_out[(size_t)CartOutput::VRAM_A10] = mmc1_out[(size_t)MMC1_Output::CIRAM_A10];

		// Contains a jumper between `/PA13` and `/VRAM_CS` (? probably, just don't care for now)
		cart_out[(size_t)CartOutput::VRAM_nCS] = cart_in[(size_t)CartInput::nPA13];

		// CHR_A13 is actually `/CS` for CHR
		TriState nCHR_CS = FromByte((ppu_addr >> 13) & 1);

		ppu_addr = (ppu_addr & 0x0fff) |			// Clear original A12+
			((size_t)ToByte(mmc1_out[(size_t)MMC1_Output::CHR_A12]) << 12) |
			((size_t)ToByte(mmc1_out[(size_t)MMC1_Output::CHR_A13]) << 13) |
			((size_t)ToByte(mmc1_out[(size_t)MMC1_Output::CHR_A14]) << 14) |
			((size_t)ToByte(mmc1_out[(size_t)MMC1_Output::CHR_A15]) << 15) |
			((size_t)ToByte(mmc1_out[(size_t)MMC1_Output::CHR_A16]) << 16);

		if (nCHR_CS == TriState::Zero)
		{
			assert(ppu_addr < CHRSize);

			if (nRD == TriState::Zero)
			{
				uint8_t val = CHR[ppu_addr];

				if (!ppu_data_dirty)
				{
					*ppu_data = val;
					ppu_data_dirty = true;
				}
				else
				{
					*ppu_data = *ppu_data & val;
				}
			}

			if (nWR == TriState::Zero)
			{
				CHR[ppu_addr] = *ppu_data;
			}
		}

		// CPU Part

		TriState SRAM_CE = mmc1_out[(size_t)MMC1_Output::SRAM_CE];
		TriState PRG_nCE = mmc1_out[(size_t)MMC1_Output::PRG_nCE];

		size_t prg_address = (cpu_addr & 0x3fff) |			// Clear original A14 (A15 is not routed actually)
			((size_t)ToByte(mmc1_out[(size_t)MMC1_Output::PRG_A14]) << 14) |		// A14 
			((size_t)ToByte(mmc1_out[(size_t)MMC1_Output::PRG_A15]) << 15) |		// A15
			((size_t)ToByte(mmc1_out[(size_t)MMC1_Output::PRG_A16]) << 16) |		// A16
			((size_t)ToByte(mmc1_out[(size_t)MMC1_Output::PRG_A17]) << 17);			// A17

		if (PRG_nCE == TriState::Zero)
		{
			uint8_t val = PRG[prg_address];

			if (!cpu_data_dirty)
			{
				*cpu_data = val;
				cpu_data_dirty = true;
			}
			else
			{
				*cpu_data = *cpu_data & val;
			}
		}

		if (SRAM_CE == TriState::One)
		{
			uint8_t val = RAM[prg_address & 0x1fff];

			if (!cpu_data_dirty)
			{
				*cpu_data = val;
				cpu_data_dirty = true;
			}
			else
			{
				*cpu_data = *cpu_data & val;
			}
		}

		TriState nIRQ = cart_out[(size_t)CartOutput::nIRQ];
		if (!(nIRQ == TriState::Zero || nIRQ == TriState::One))
		{
			cart_out[(size_t)CartOutput::nIRQ] = TriState::Z;
		}

		if (p1_type == ConnectorType::FamicomStyle && snd_out)
		{
			snd_out->normalized = 0.0f;
		}
	}
}
