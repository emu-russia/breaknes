// CartPcbCartridge - a cartridge that simulates a Pcb.

#include "CartPcbCartridge.h"

#include <cstring>

#include "../Breaknes/BreaksCore/DebugHub.h"

namespace CartPcb
{
	CartPcbCartridge::CartPcbCartridge(ConnectorType p1, Pcb* _pcb) : Cartridge(p1)
	{
		pcb = _pcb;
		valid = (pcb != nullptr);

		if (valid)
		{
			AddCartMemDescriptors();

			LOG_CART(Cat_Events, "Cartridge created: %s (%s)",
				pcb->GetBoardType().c_str(), pcb->GetBoardPcb().c_str());
		}
	}

	CartPcbCartridge::~CartPcbCartridge()
	{
	}

	bool CartPcbCartridge::Valid()
	{
		return valid;
	}

	void CartPcbCartridge::sim(
		BaseLogic::TriState cart_in[(size_t)CartInput::Max],
		BaseLogic::TriState cart_out[(size_t)CartOutput::Max],
		uint16_t cpu_addr,
		uint8_t* cpu_data, bool& cpu_data_dirty,
		uint16_t ppu_addr,
		uint8_t* ppu_data, bool& ppu_data_dirty,
		CartAudioOutSignal* snd_out,
		uint16_t* exp, bool& exp_dirty)
	{
		if (!valid)
			return;

		pcb->sim(
			cart_in,
			cart_out,
			cpu_addr,
			cpu_data, cpu_data_dirty,
			ppu_addr,
			ppu_data, ppu_data_dirty,
			snd_out,
			exp, exp_dirty);
	}

	uint8_t CartPcbCartridge::Dbg_ReadPRGByte(size_t cpu_addr)
	{
		if (!valid)
			return 0;

		return pcb->Dbg_ReadPRGByte(cpu_addr);
	}

	void CartPcbCartridge::SetLogMask(uint64_t mask)
	{
		if (pcb != nullptr)
		{
			pcb->SetLogMask(mask);
		}
	}

	void CartPcbCartridge::SetChipLogMask(uint64_t mask)
	{
		if (pcb != nullptr)
		{
			pcb->SetChipLogMask(mask);
		}
	}

	namespace
	{
		uint8_t Dbg_ReadPRGThunk(void* opaque, size_t addr)
		{
			CartPcbCartridge* cart = (CartPcbCartridge*)opaque;
			return cart->GetPcb()->Dbg_ReadPRGByte(addr);
		}

		uint8_t Dbg_ReadCHRThunk(void* opaque, size_t addr)
		{
			CartPcbCartridge* cart = (CartPcbCartridge*)opaque;
			return cart->GetPcb()->Dbg_ReadCHRByte(addr);
		}

		void Dbg_WriteCHRThunk(void* opaque, size_t addr, uint8_t data)
		{
			CartPcbCartridge* cart = (CartPcbCartridge*)opaque;
			cart->GetPcb()->Dbg_WriteCHRByte(addr, data);
		}

		uint8_t Dbg_ReadWRAMThunk(void* opaque, size_t addr)
		{
			CartPcbCartridge* cart = (CartPcbCartridge*)opaque;
			return cart->GetPcb()->Dbg_ReadWRAMByte(addr);
		}

		void Dbg_WriteWRAMThunk(void* opaque, size_t addr, uint8_t data)
		{
			CartPcbCartridge* cart = (CartPcbCartridge*)opaque;
			cart->GetPcb()->Dbg_WriteWRAMByte(addr, data);
		}
	}

	void CartPcbCartridge::AddCartMemDescriptors()
	{
		if (dbg_hub == nullptr)
			return;

		size_t prgSize = pcb->Dbg_GetPRGSize();
		size_t chrSize = pcb->Dbg_GetCHRSize();
		size_t wramSize = pcb->Dbg_GetWRAMSize();

		if (prgSize != 0)
		{
			MemDesciptor* prgRegion = new MemDesciptor;
			memset(prgRegion, 0, sizeof(MemDesciptor));
			strcpy(prgRegion->name, "PRG");
			prgRegion->size = (int32_t)prgSize;
			dbg_hub->AddMemRegion(prgRegion, Dbg_ReadPRGThunk, nullptr, this, true);
		}

		if (chrSize != 0)
		{
			MemDesciptor* chrRegion = new MemDesciptor;
			memset(chrRegion, 0, sizeof(MemDesciptor));
			strcpy(chrRegion->name, "CHR");
			chrRegion->size = (int32_t)chrSize;
			dbg_hub->AddMemRegion(chrRegion, Dbg_ReadCHRThunk, Dbg_WriteCHRThunk, this, true);
		}

		if (wramSize != 0)
		{
			MemDesciptor* wramRegion = new MemDesciptor;
			memset(wramRegion, 0, sizeof(MemDesciptor));
			strcpy(wramRegion->name, "WRAM");
			wramRegion->size = (int32_t)wramSize;
			dbg_hub->AddMemRegion(wramRegion, Dbg_ReadWRAMThunk, Dbg_WriteWRAMThunk, this, true);
		}
	}
}
