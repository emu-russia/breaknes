// Simple Mapper Simulator (0) - NROM.
// I'm not very knowledgeable about the zoo of their implementations, so this module will gradually be improved as new knowledge becomes available.
// Also: We limit the simulation to the CHR part only, because the PPUPlayer does not require a PRG.

#include "pch.h"

using namespace BaseLogic;

namespace PPUPlayer
{
#pragma pack(push, 1)
	struct NESHeader
	{
		uint8_t Sign[4];
		uint8_t PRGSize;
		uint8_t CHRSize;
		uint8_t Flags_6;
		uint8_t Flags_7;
		uint8_t Flags_8;
		uint8_t Flags_9;
		uint8_t Flags_10;
		uint8_t padding[5];
	};
#pragma pack(pop)

	NROM::NROM(uint8_t* nesImage, size_t nesImageSize)
	{
		printf("NROM::NROM()\n");

		if (nesImage != nullptr && nesImageSize >= sizeof(NESHeader))
		{
			// Basic .nes header checks

			printf("Basic .nes header checks ...");

			NESHeader* head = (NESHeader*)nesImage;

			if (head->PRGSize >= 0x10 || head->CHRSize >= 0x10 || head->CHRSize == 0)
			{
				printf(" FAILED (1)!\n");
				return;
			}

			bool trainer = (head->Flags_6 & 0b100) != 0;
			size_t mapperNum = (head->Flags_7 & 0xf0) | (head->Flags_6 >> 4);

			if (mapperNum != 0)
			{
				printf(" FAILED (2)!\n");
				return;
			}

			size_t totalSize = (trainer ? 512 : 0) + 0x4000 * head->PRGSize + 0x2000 * head->CHRSize + sizeof(NESHeader);
			if (nesImageSize < totalSize)
			{
				printf(" FAILED (3)!\n");
				return;
			}

			printf(" OK!\n");

			// Load CHR ROM

			CHRSize = head->CHRSize * 0x2000;
			CHR = new uint8_t[CHRSize];

			uint8_t* chrPtr = nesImage + sizeof(NESHeader) + (trainer ? 512 : 0) + head->PRGSize * 0x4000;
			memcpy(CHR, chrPtr, CHRSize);

			valid = true;
		}
	}

	NROM::~NROM()
	{
		printf("NROM::~NROM()\n");

		if (PRG != nullptr)
			delete PRG;

		if (CHR != nullptr)
			delete CHR;
	}

	void NROM::sim(TriState PA[14], TriState n_PA13, TriState n_RD, TriState n_WR,
		uint8_t* PD, bool& PDDirty,
		TriState& n_VRAM_CS, TriState& VRAM_A10)
	{
		if (!valid)
			return;

		// TBD: H/V Mirroring

		// NROM contains a jumper between `/PA13` and `/VRAM_CS`

		n_VRAM_CS = n_PA13;

		if (n_RD == TriState::Zero)
		{
			size_t addr = 0;

			for (size_t n = 0; n < 14; n++)
			{
				addr |= (PA[n] == TriState::One ? 1 : 0) << n;
			}

			if (addr >= CHRSize)
			{
				return;
			}

			uint8_t val = CHR[addr];

			if (!PDDirty)
			{
				*PD = val;
				PDDirty = true;
			}
			else
			{
				*PD = *PD & val;
			}
		}
	}
}
