// Simple Mapper Simulator (0) - NROM.
// I'm not very knowledgeable about the zoo of their implementations, so this module will gradually be improved as new knowledge becomes available.

#include "pch.h"

using namespace BaseLogic;

#define NROM_CATEGORY "NROM"
#define CHR_ROM_NAME "CHR-ROM"

namespace Mappers
{
	NROM::NROM(Breaknes::ConnectorType p1, uint8_t* nesImage, size_t nesImageSize) : AbstractCartridge (p1, nesImage, nesImageSize)
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

			// Load PRG ROM

			PRGSize = head->PRGSize * 0x4000;
			PRG = new uint8_t[PRGSize];

			uint8_t* prgPtr = nesImage + sizeof(NESHeader) + (trainer ? 512 : 0);
			memcpy(PRG, prgPtr, PRGSize);

			valid = true;

			AddCartMemDescriptors();
			AddCartDebugInfoProviders();
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

	void NROM::sim(
		TriState cart_in[(size_t)Breaknes::CartInput::Max],
		TriState cart_out[(size_t)Breaknes::CartOutput::Max],
		uint16_t cpu_addr,
		uint8_t* cpu_data, bool& cpu_data_dirty,
		uint16_t ppu_addr,
		uint8_t* ppu_data, bool& ppu_data_dirty,
		// Famicom only
		APUSim::AudioOutSignal* snd_in,
		Breaknes::CartAudioOutSignal* snd_out,
		// NES only
		uint16_t* exp, bool& exp_dirty)
	{
		UNREFERENCED_PARAMETER(snd_in);
		UNREFERENCED_PARAMETER(snd_out);
		UNREFERENCED_PARAMETER(exp);
		UNREFERENCED_PARAMETER(exp_dirty);

		if (!valid)
			return;

		// PPU Part

		TriState nRD = cart_in[(size_t)Breaknes::CartInput::nRD];
		TriState nWR = cart_in[(size_t)Breaknes::CartInput::nWR];

		nrom_debug.last_nRD = nRD == TriState::One ? 1 : 0;
		nrom_debug.last_nWR = nWR == TriState::One ? 1 : 0;

		// H/V Mirroring
		cart_out[(size_t)Breaknes::CartOutput::VRAM_A10] = V_Mirroring ? FromByte((ppu_addr >> 10) & 1) : FromByte((ppu_addr >> 11) & 1);

		// NROM contains a jumper between `/PA13` and `/VRAM_CS`
		cart_out[(size_t)Breaknes::CartOutput::VRAM_nCS] = cart_in[(size_t)Breaknes::CartInput::nPA13];

		// CHR_A13 is actually `/CS` for CHR-ROM
		TriState nCHR_CS = FromByte((ppu_addr >> 13) & 1);

		if (NOR(nRD, nCHR_CS) == TriState::One)
		{
			nrom_debug.last_PA = (uint32_t)ppu_addr;

			assert(ppu_addr < CHRSize);

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

		// CPU Part

		TriState nROMSEL = cart_in[(size_t)Breaknes::CartInput::nROMSEL];

		if (nROMSEL == TriState::Zero)
		{
			uint8_t val = PRG[cpu_addr & 0x3fff];		// A13 not used

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

		cart_out[(size_t)Breaknes::CartOutput::nIRQ] = TriState::Z;
	}

	struct SignalOffsetPair
	{
		const char* name;
		const size_t offset;
		const uint8_t bits;
	};

	SignalOffsetPair nrom_signals[] = {
		"Last PA", offsetof(NROM_DebugInfo, last_PA), 16,
		"Last /RD", offsetof(NROM_DebugInfo, last_nRD), 1,
		"Last /WR", offsetof(NROM_DebugInfo, last_nWR), 1,
	};

	void NROM::AddCartMemDescriptors()
	{
		MemDesciptor* chrRegion = new MemDesciptor;
		memset(chrRegion, 0, sizeof(MemDesciptor));
		strcpy_s(chrRegion->name, sizeof(chrRegion->name), CHR_ROM_NAME);
		chrRegion->size = (int32_t)Dbg_GetCHRSize();
		dbg_hub->AddMemRegion(chrRegion, Dbg_ReadCHRByte, Dbg_WriteCHRByte, this, true);
	}

	void NROM::AddCartDebugInfoProviders()
	{
		for (size_t n = 0; n < _countof(nrom_signals); n++)
		{
			SignalOffsetPair* sp = &nrom_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), NROM_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Cart, entry, GetCartDebugInfo, SetCartDebugInfo, this);
		}
	}

	uint32_t NROM::GetCartDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NROM* nrom = (NROM*)opaque;

		for (size_t n = 0; n < _countof(nrom_signals); n++)
		{
			SignalOffsetPair* sp = &nrom_signals[n];

			if (!strcmp(sp->name, entry->name))
			{
				Mappers::NROM_DebugInfo nrom_info{};
				nrom->GetDebugInfo(nrom_info);

				uint8_t* ptr = (uint8_t*)&nrom_info + sp->offset;
				return *(uint32_t*)ptr;
			}
		}

		return 0;
	}

	void NROM::SetCartDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not need
	}

	size_t NROM::Dbg_GetCHRSize()
	{
		if (!valid)
			return 0;

		return CHRSize;
	}

	uint8_t NROM::Dbg_ReadCHRByte(void* opaque, size_t addr)
	{
		NROM* nrom = (NROM*)opaque;

		if (!nrom->valid)
			return 0;

		if (addr < nrom->CHRSize)
		{
			return nrom->CHR[addr];
		}
		else
		{
			return 0;
		}
	}

	/// <summary>
	/// Generally speaking, the NROM contains CHR-ROM, but who cares.
	/// </summary>
	void NROM::Dbg_WriteCHRByte(void* opaque, size_t addr, uint8_t data)
	{
		NROM* nrom = (NROM*)opaque;

		if (!nrom->valid)
			return;

		if (addr < nrom->CHRSize)
		{
			nrom->CHR[addr] = data;
		}
	}

	void NROM::GetDebugInfo(NROM_DebugInfo& info)
	{
		info = nrom_debug;
	}
}
