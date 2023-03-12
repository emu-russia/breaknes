// Simple Mapper Simulator (7) - AOROM.

#include "pch.h"

using namespace BaseLogic;

#define AOROM_CATEGORY "AOROM"
#define CHR_ROM_NAME "CHR-ROM"

// TBD: Change the CHR-ROM entity to just CHR
// TBD: Add PRG dump

namespace Mappers
{
	AOROM::AOROM(Breaknes::ConnectorType p1, uint8_t* nesImage, size_t nesImageSize) : AbstractCartridge(p1, nesImage, nesImageSize)
	{
		printf("AOROM::AOROM()\n");

		NESHeader* head = (NESHeader*)nesImage;

		bool trainer = (head->Flags_6 & 0b100) != 0;

		// CHR RAM

		CHRSize = 0x4000;
		CHR = new uint8_t[CHRSize];
		memset(CHR, 0, CHRSize);

		// Load PRG ROM

		PRGSize = head->PRGSize * 0x4000;
		PRG = new uint8_t[PRGSize];

		uint8_t* prgPtr = nesImage + sizeof(NESHeader) + (trainer ? 512 : 0);
		memcpy(PRG, prgPtr, PRGSize);

		valid = true;

		AddCartMemDescriptors();
	}

	AOROM::~AOROM()
	{
		printf("AOROM::~AOROM()\n");

		if (PRG != nullptr)
			delete PRG;

		if (CHR != nullptr)
			delete CHR;
	}

	bool AOROM::Valid()
	{
		return valid;
	}

	void AOROM::sim(
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

		// Counter (as register) to select PRG Bank

		TriState nROMSEL = cart_in[(size_t)Breaknes::CartInput::nROMSEL];

		TriState vdd = TriState::One;
		TriState gnd = TriState::Zero;
		TriState CPU_RnW = cart_in[(size_t)Breaknes::CartInput::RnW];

		TriState P[4]{};

		P[0] = FromByte(((*cpu_data) >> 0) & 1);
		P[1] = FromByte(((*cpu_data) >> 1) & 1);
		P[2] = FromByte(((*cpu_data) >> 2) & 1);
		P[3] = FromByte(((*cpu_data) >> 4) & 1);	// VRAM_A10

		TriState RCO;		// unused
		TriState Q[4]{};

		counter.sim(nROMSEL, vdd, CPU_RnW, gnd, gnd, P, RCO, Q);

		// PPU Part

		TriState nRD = cart_in[(size_t)Breaknes::CartInput::nRD];
		TriState nWR = cart_in[(size_t)Breaknes::CartInput::nWR];

		// H/V Mirroring
		cart_out[(size_t)Breaknes::CartOutput::VRAM_A10] = Q[3];

		// Contains a jumper between `/PA13` and `/VRAM_CS`
		cart_out[(size_t)Breaknes::CartOutput::VRAM_nCS] = cart_in[(size_t)Breaknes::CartInput::nPA13];

		// CHR_A13 is actually `/CS` for CHR
		TriState nCHR_CS = FromByte((ppu_addr >> 13) & 1);

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

		size_t prg_address = (cpu_addr) |
			((size_t)ToByte(Q[0]) << 15) |		// A15
			((size_t)ToByte(Q[1]) << 16) |		// A16
			((size_t)ToByte(Q[2]) << 17);		// A17

		if (nROMSEL == TriState::Zero)
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

		TriState nIRQ = cart_out[(size_t)Breaknes::CartOutput::nIRQ];
		if (!(nIRQ == TriState::Zero || nIRQ == TriState::One))
		{
			cart_out[(size_t)Breaknes::CartOutput::nIRQ] = TriState::Z;
		}
	}

	void AOROM::AddCartMemDescriptors()
	{
		MemDesciptor* chrRegion = new MemDesciptor;
		memset(chrRegion, 0, sizeof(MemDesciptor));
		strcpy_s(chrRegion->name, sizeof(chrRegion->name), CHR_ROM_NAME);
		chrRegion->size = (int32_t)CHRSize;
		dbg_hub->AddMemRegion(chrRegion, Dbg_ReadCHRByte, Dbg_WriteCHRByte, this, true);
	}

	uint8_t AOROM::Dbg_ReadCHRByte(void* opaque, size_t addr)
	{
		AOROM* aorom = (AOROM*)opaque;

		if (!aorom->valid)
			return 0;

		if (addr < aorom->CHRSize)
		{
			return aorom->CHR[addr];
		}
		else
		{
			return 0;
		}
	}

	void AOROM::Dbg_WriteCHRByte(void* opaque, size_t addr, uint8_t data)
	{
		AOROM* aorom = (AOROM*)opaque;

		if (!aorom->valid)
			return;

		if (addr < aorom->CHRSize)
		{
			aorom->CHR[addr] = data;
		}
	}
}
