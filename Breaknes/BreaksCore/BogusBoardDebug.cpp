#include "pch.h"

using namespace BaseLogic;

#define WRAM_NAME "WRAM"
#define CORE_WIRES_CATEGORY "Core Wires"
#define CORE_REGS_CATEGORY "Core Regs"
#define BOARD_CATEGORY "Board"

namespace Breaknes
{
	void BogusBoard::AddBoardMemDescriptors()
	{
		// WRAM

		MemDesciptor* wramRegion = new MemDesciptor;
		memset(wramRegion, 0, sizeof(MemDesciptor));
		strcpy(wramRegion->name, WRAM_NAME);
		wramRegion->size = (int32_t)wram->Dbg_GetSize();
		dbg_hub->AddMemRegion(wramRegion, DumpWRAM, WriteWRAM, this, false);
	}

	static SignalOffsetPair board_signals[] = {
		"/NMI", offsetof(BogusBoardDebugInfo, n_NMI), 1,
		"/IRQ", offsetof(BogusBoardDebugInfo, n_IRQ), 1,
		"/RES", offsetof(BogusBoardDebugInfo, n_RES), 1,
		"PHI0", offsetof(BogusBoardDebugInfo, PHI0), 1,
		"RDY", offsetof(BogusBoardDebugInfo, RDY), 1,
		"SO", offsetof(BogusBoardDebugInfo, SO), 1,
		"PHI1", offsetof(BogusBoardDebugInfo, PHI1), 1,
		"PHI2", offsetof(BogusBoardDebugInfo, PHI2), 1,
		"R/W", offsetof(BogusBoardDebugInfo, RnW), 1,
		"SYNC", offsetof(BogusBoardDebugInfo, SYNC), 1,
		"A", offsetof(BogusBoardDebugInfo, A), 16,
		"D", offsetof(BogusBoardDebugInfo, D), 8,
	};
	static size_t board_signals_count = sizeof(board_signals) / sizeof(board_signals[0]);

	void BogusBoard::AddDebugInfoProviders()
	{
		for (size_t n = 0; n < core_wires_count; n++)
		{
			SignalOffsetPair* sp = &core_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, CORE_WIRES_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Core, entry, GetCoreDebugInfo, SetCoreDebugInfo, this);
		}

		for (size_t n = 0; n < core_regs_count; n++)
		{
			SignalOffsetPair* sp = &core_regs[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, CORE_REGS_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_CoreRegs, entry, GetCoreRegsDebugInfo, SetCoreRegsDebugInfo, this);
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

	uint8_t BogusBoard::DumpWRAM(void* opaque, size_t addr)
	{
		BogusBoard* board = (BogusBoard*)opaque;
		return board->wram->Dbg_ReadByte(addr);
	}

	void BogusBoard::WriteWRAM(void* opaque, size_t addr, uint8_t data)
	{
		BogusBoard* board = (BogusBoard*)opaque;
		board->wram->Dbg_WriteByte(addr, data);
	}

	uint32_t BogusBoard::GetCoreDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		BogusBoard* board = (BogusBoard*)opaque;

		for (size_t n = 0; n < core_wires_count; n++)
		{
			SignalOffsetPair* sp = &core_wires[n];

			if (!strcmp(sp->name, entry->name))
			{
				return board->core->getDebugSingle((int)sp->offset);
			}
		}

		return 0;
	}

	uint32_t BogusBoard::GetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		BogusBoard* board = (BogusBoard*)opaque;

		for (size_t n = 0; n < core_regs_count; n++)
		{
			SignalOffsetPair* sp = &core_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				return board->core->getUserRegSingle((int)sp->offset);
			}
		}

		return 0;
	}

	void BogusBoard::SetCoreDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not need
	}

	void BogusBoard::SetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		BogusBoard* board = (BogusBoard*)opaque;

		for (size_t n = 0; n < core_regs_count; n++)
		{
			SignalOffsetPair* sp = &core_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				board->core->setUserRegSingle((int)sp->offset, value);
			}
		}
	}

	void BogusBoard::GetDebugInfo(BogusBoardDebugInfo& info)
	{
		info.n_NMI = ToByte(n_NMI);
		info.n_IRQ = ToByte(n_IRQ);
		info.n_RES = ToByte(n_RES);
		info.PHI0 = ToByte(CLK);
		info.RDY = ToByte(RDY);
		info.SO = ToByte(SO);
		info.PHI1 = ToByte(PHI1);
		info.PHI2 = ToByte(PHI2);
		info.RnW = ToByte(RnW);
		info.SYNC = ToByte(SYNC);
		info.A = addr_bus;
		info.D = data_bus;
	}

	uint32_t BogusBoard::GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		BogusBoard* board = (BogusBoard*)opaque;

		for (size_t n = 0; n < board_signals_count; n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			if (!strcmp(sp->name, entry->name))
			{
				BogusBoardDebugInfo info{};
				board->GetDebugInfo(info);

				uint8_t* ptr = (uint8_t*)&info + sp->offset;
				return *(uint32_t*)ptr;
			}
		}

		return 0;
	}

	void BogusBoard::SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		BogusBoard* board = (BogusBoard*)opaque;

		if (!strcmp(entry->name, "/NMI")) board->n_NMI = FromByte(value);
		else if (!strcmp(entry->name, "/IRQ")) board->n_IRQ = FromByte(value);
		else if (!strcmp(entry->name, "/RES")) board->n_RES = FromByte(value);
		else if (!strcmp(entry->name, "PHI0")) board->CLK = FromByte(value);
		else if (!strcmp(entry->name, "RDY")) board->RDY = FromByte(value);
		else if (!strcmp(entry->name, "SO")) board->SO = FromByte(value);
		else if (!strcmp(entry->name, "PHI1")) board->PHI1 = FromByte(value);
		else if (!strcmp(entry->name, "PHI2")) board->PHI2 = FromByte(value);
		else if (!strcmp(entry->name, "R/W")) board->RnW = FromByte(value);
		else if (!strcmp(entry->name, "SYNC")) board->SYNC = FromByte(value);
		else if (!strcmp(entry->name, "A")) board->addr_bus = (uint16_t)value;
		else if (!strcmp(entry->name, "D")) board->data_bus = (uint8_t)value;
	}
}
