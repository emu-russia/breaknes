// Debugging capabilities of APUPlayer (very limited as we do not have a real 6502)

#include "pch.h"

using namespace BaseLogic;

#define WRAM_NAME "WRAM"

namespace Breaknes
{
	void APUPlayerBoard::AddBoardMemDescriptors()
	{
		// WRAM

		MemDesciptor* wramRegion = new MemDesciptor;
		memset(wramRegion, 0, sizeof(MemDesciptor));
		strcpy(wramRegion->name, WRAM_NAME);
		wramRegion->size = (int32_t)wram->Dbg_GetSize();
		dbg_hub->AddMemRegion(wramRegion, DumpWRAM, WriteWRAM, this, false);
	}

	static SignalOffsetPair board_signals[] = {
		"BoardCLK", offsetof(APUBoardDebugInfo, CLK), 1,
		"ABus", offsetof(APUBoardDebugInfo, ABus), 16,
		"DBus", offsetof(APUBoardDebugInfo, DBus), 8,
	};
	static size_t board_signals_count = sizeof(board_signals) / sizeof(board_signals[0]);

	void APUPlayerBoard::AddDebugInfoProviders()
	{
		for (size_t n = 0; n < apu_wires_count; n++)
		{
			SignalOffsetPair* sp = &apu_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, APU_WIRES_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_APU, entry, GetApuDebugInfo, SetApuDebugInfo, this);
		}

		for (size_t n = 0; n < apu_regs_count; n++)
		{
			SignalOffsetPair* sp = &apu_regs[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, APU_REGS_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_APURegs, entry, GetApuRegsDebugInfo, SetApuRegsDebugInfo, this);
		}

		for (size_t n = 0; n < board_signals_count; n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, BOARD_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Board, entry, GetBoardDebugInfo, SetBoardDebugInfo, this);
		}
	}

	uint8_t APUPlayerBoard::DumpWRAM(void* opaque, size_t addr)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;
		return board->wram->Dbg_ReadByte(addr);
	}

	void APUPlayerBoard::WriteWRAM(void* opaque, size_t addr, uint8_t data)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;
		board->wram->Dbg_WriteByte(addr, data);
	}

	uint32_t APUPlayerBoard::GetApuDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;

		for (size_t n = 0; n < apu_wires_count; n++)
		{
			SignalOffsetPair* sp = &apu_wires[n];

			if (!strcmp(sp->name, entry->name))
			{
				APUSim::APU_Interconnects wires{};
				board->apu->GetDebugInfo_Wires(wires);

				uint8_t* ptr = (uint8_t*)&wires;
				return ptr[sp->offset];
			}
		}

		return 0;
	}

	uint32_t APUPlayerBoard::GetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;

		for (size_t n = 0; n < apu_regs_count; n++)
		{
			SignalOffsetPair* sp = &apu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				return board->apu->GetDebugInfo_Reg((int)sp->offset);
			}
		}

		return 0;
	}

	void APUPlayerBoard::SetApuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not need
	}

	void APUPlayerBoard::SetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;

		for (size_t n = 0; n < apu_regs_count; n++)
		{
			SignalOffsetPair* sp = &apu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				board->apu->SetDebugInfo_Reg((int)sp->offset, value);
			}
		}
	}

	void APUPlayerBoard::GetDebugInfo(APUBoardDebugInfo& info)
	{
		info.CLK = CLK;
		info.ABus = addr_bus;
		info.DBus = data_bus;
	}

	uint32_t APUPlayerBoard::GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		APUPlayerBoard* board = (APUPlayerBoard*)opaque;

		for (size_t n = 0; n < board_signals_count; n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			if (!strcmp(sp->name, entry->name))
			{
				APUBoardDebugInfo info{};
				board->GetDebugInfo(info);

				uint8_t* ptr = (uint8_t*)&info + sp->offset;
				return *(uint32_t*)ptr;
			}
		}

		return 0;
	}

	void APUPlayerBoard::SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not used
	}
}
