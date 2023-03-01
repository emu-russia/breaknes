// Simple Mapper Simulator (0) - NROM.
// I'm not very knowledgeable about the zoo of their implementations, so this module will gradually be improved as new knowledge becomes available.
// Also: We limit the simulation to the CHR part only, because the PPUPlayer does not require a PRG.

#include "pch.h"

using namespace BaseLogic;

namespace Mappers
{
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
				printf(" FAILED! Odd size of PRG/CHR banks or unsupported CHR-RAM!\n");
				return;
			}

			bool trainer = (head->Flags_6 & 0b100) != 0;
			size_t mapperNum = (head->Flags_7 & 0xf0) | (head->Flags_6 >> 4);

			if (mapperNum != 0)
			{
				printf(" FAILED! The number of the mapper must be 0 (NROM).\n");
				return;
			}

			size_t totalSize = (trainer ? 512 : 0) + 0x4000 * head->PRGSize + 0x2000 * head->CHRSize + sizeof(NESHeader);
			if (nesImageSize < totalSize)
			{
				printf(" FAILED! Damaged .nes (file size is smaller than required).\n");
				return;
			}

			V_Mirroring = (head->Flags_6 & 1) != 0;

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

	bool NROM::Valid()
	{
		return valid;
	}

	void NROM::sim(TriState PA[14], TriState n_PA13, TriState n_RD, TriState n_WR,
		uint8_t* PD, bool& PDDirty,
		TriState& n_VRAM_CS, TriState& VRAM_A10)
	{
		if (!valid)
			return;

		nrom_debug.last_nRD = n_RD == TriState::One ? 1 : 0;
		nrom_debug.last_nWR = n_WR == TriState::One ? 1 : 0;

		// H/V Mirroring

		VRAM_A10 = V_Mirroring ? PA[10] : PA[11];

		// NROM contains a jumper between `/PA13` and `/VRAM_CS`

		n_VRAM_CS = n_PA13;

		// CHR_A13 is actually `/CS` for CHR-ROM

		TriState nCHR_CS = PA[13];

		if (NOR(n_RD, nCHR_CS) == TriState::One)
		{
			size_t addr = 0;

			for (size_t n = 0; n < 13; n++)
			{
				addr |= (PA[n] == TriState::One ? 1ULL : 0) << n;
			}

			nrom_debug.last_PA = (uint32_t)addr;

			assert(addr < CHRSize);

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

	size_t NROM::Dbg_GetCHRSize()
	{
		if (!valid)
			return 0;

		return CHRSize;
	}

	uint8_t NROM::Dbg_ReadCHRByte(size_t addr)
	{
		if (!valid)
			return 0;

		if (addr < CHRSize)
		{
			return CHR[addr];
		}
		else
		{
			return 0;
		}
	}

	/// <summary>
	/// Generally speaking, the NROM contains CHR-ROM, but who cares.
	/// </summary>
	void NROM::Dbg_WriteCHRByte(size_t addr, uint8_t data)
	{
		if (!valid)
			return;

		if (addr < CHRSize)
		{
			CHR[addr] = data;
		}
	}

	void NROM::GetDebugInfo(NROM_DebugInfo& info)
	{
		info = nrom_debug;
	}
}
