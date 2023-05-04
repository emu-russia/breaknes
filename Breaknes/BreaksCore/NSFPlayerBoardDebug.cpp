// This module aggregates all the debugging mechanisms of NSFPlayer board.

#include "pch.h"

using namespace BaseLogic;

#define BANKED_SRAM_NAME "BankedSRAM"
#define WRAM_NAME "WRAM"

namespace Breaknes
{
	void NSFPlayerBoard::AddBoardMemDescriptors()
	{
		// Banked SRAM

		MemDesciptor* sramRegion = new MemDesciptor;
		memset(sramRegion, 0, sizeof(MemDesciptor));
		strcpy(sramRegion->name, BANKED_SRAM_NAME);
		sramRegion->size = (int32_t)sram->Dbg_GetSize();
		dbg_hub->AddMemRegion(sramRegion, DumpSRAM, WriteSRAM, this, false);

		// WRAM

		MemDesciptor* wramRegion = new MemDesciptor;
		memset(wramRegion, 0, sizeof(MemDesciptor));
		strcpy(wramRegion->name, WRAM_NAME);
		wramRegion->size = (int32_t)wram->Dbg_GetSize();
		dbg_hub->AddMemRegion(wramRegion, DumpWRAM, WriteWRAM, this, false);
	}

	static SignalOffsetPair board_signals[] = {
		"SYNC", offsetof(NSFBoardDebugInfo, sync), 1,				// 6502 Core SYNC terminal
		"BoardCLK", offsetof(NSFBoardDebugInfo, CLK), 1,
		"Bank0", offsetof(NSFBoardDebugInfo, bank_reg[0]), 8,		// Bank switch registers
		"Bank1", offsetof(NSFBoardDebugInfo, bank_reg[1]), 8,
		"Bank2", offsetof(NSFBoardDebugInfo, bank_reg[2]), 8,
		"Bank3", offsetof(NSFBoardDebugInfo, bank_reg[3]), 8,
		"Bank4", offsetof(NSFBoardDebugInfo, bank_reg[4]), 8,
		"Bank5", offsetof(NSFBoardDebugInfo, bank_reg[5]), 8,
		"Bank6", offsetof(NSFBoardDebugInfo, bank_reg[6]), 8,
		"Bank7", offsetof(NSFBoardDebugInfo, bank_reg[7]), 8,
		"ABus", offsetof(NSFBoardDebugInfo, ABus), 16,
		"DBus", offsetof(NSFBoardDebugInfo, DBus), 8,
		"Reset", offsetof(NSFBoardDebugInfo, ResetPending), 1,
	};
	static size_t board_signals_count = sizeof(board_signals) / sizeof(board_signals[0]);

	void NSFPlayerBoard::AddDebugInfoProviders()
	{
		for (size_t n = 0; n < core_brk_wires_count; n++)
		{
			SignalOffsetPair* sp = &core_brk_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, CORE_BRK_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Core, entry, GetCoreDebugInfo, SetCoreDebugInfo, this);
		}

		for (size_t n = 0; n < core_disp_wires_count; n++)
		{
			SignalOffsetPair* sp = &core_disp_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, CORE_DISP_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Core, entry, GetCoreDebugInfo, SetCoreDebugInfo, this);
		}

		for (size_t n = 0; n < core_alu_wires_count; n++)
		{
			SignalOffsetPair* sp = &core_alu_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, CORE_ALU_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Core, entry, GetCoreDebugInfo, SetCoreDebugInfo, this);
		}

		for (size_t n = 0; n < core_bops_wires_count; n++)
		{
			SignalOffsetPair* sp = &core_bops_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, CORE_BOPS_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Core, entry, GetCoreDebugInfo, SetCoreDebugInfo, this);
		}

		for (size_t n = 0; n < core_fops_wires_count; n++)
		{
			SignalOffsetPair* sp = &core_fops_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, CORE_FOPS_CATEGORY);
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

		for (size_t n = 0; n < apu_clks_wires_count; n++)
		{
			SignalOffsetPair* sp = &apu_clks_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, APU_CLKS_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_APU, entry, GetApuDebugInfo, SetApuDebugInfo, this);
		}

		for (size_t n = 0; n < apu_core_wires_count; n++)
		{
			SignalOffsetPair* sp = &apu_core_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, APU_CORE_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_APU, entry, GetApuDebugInfo, SetApuDebugInfo, this);
		}

		for (size_t n = 0; n < apu_dma_wires_count; n++)
		{
			SignalOffsetPair* sp = &apu_dma_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, APU_DMA_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_APU, entry, GetApuDebugInfo, SetApuDebugInfo, this);
		}

		for (size_t n = 0; n < apu_regops_wires_count; n++)
		{
			SignalOffsetPair* sp = &apu_regops_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, APU_REGOPS_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_APU, entry, GetApuDebugInfo, SetApuDebugInfo, this);
		}

		for (size_t n = 0; n < apu_lc_wires_count; n++)
		{
			SignalOffsetPair* sp = &apu_lc_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, APU_LC_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_APU, entry, GetApuDebugInfo, SetApuDebugInfo, this);
		}

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

	uint8_t NSFPlayerBoard::DumpSRAM(void* opaque, size_t addr)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;
		return board->sram->Dbg_ReadByte(addr);
	}

	void NSFPlayerBoard::WriteSRAM(void* opaque, size_t addr, uint8_t data)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;
		board->sram->Dbg_WriteByte(addr, data);
	}

	uint8_t NSFPlayerBoard::DumpWRAM(void* opaque, size_t addr)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;
		return board->wram->Dbg_ReadByte(addr);
	}

	void NSFPlayerBoard::WriteWRAM(void* opaque, size_t addr, uint8_t data)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;
		board->wram->Dbg_WriteByte(addr, data);
	}

	uint32_t NSFPlayerBoard::GetCoreDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;

		if (!strcmp(entry->category, CORE_BRK_CATEGORY))
		{
			for (size_t n = 0; n < core_brk_wires_count; n++)
			{
				SignalOffsetPair* sp = &core_brk_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					return board->core->getDebugSingle((int)sp->offset);
				}
			}
		}

		else if (!strcmp(entry->category, CORE_DISP_CATEGORY))
		{
			for (size_t n = 0; n < core_disp_wires_count; n++)
			{
				SignalOffsetPair* sp = &core_disp_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					return board->core->getDebugSingle((int)sp->offset);
				}
			}
		}

		else if (!strcmp(entry->category, CORE_ALU_CATEGORY))
		{
			for (size_t n = 0; n < core_alu_wires_count; n++)
			{
				SignalOffsetPair* sp = &core_alu_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					return board->core->getDebugSingle((int)sp->offset);
				}
			}
		}

		else if (!strcmp(entry->category, CORE_BOPS_CATEGORY))
		{
			for (size_t n = 0; n < core_bops_wires_count; n++)
			{
				SignalOffsetPair* sp = &core_bops_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					return board->core->getDebugSingle((int)sp->offset);
				}
			}
		}

		else if (!strcmp(entry->category, CORE_FOPS_CATEGORY))
		{
			for (size_t n = 0; n < core_fops_wires_count; n++)
			{
				SignalOffsetPair* sp = &core_fops_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					return board->core->getDebugSingle((int)sp->offset);
				}
			}
		}

		return 0;
	}

	uint32_t NSFPlayerBoard::GetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;

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

	uint32_t NSFPlayerBoard::GetApuDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;

		if (!strcmp(entry->category, APU_CLKS_CATEGORY))
		{
			for (size_t n = 0; n < apu_clks_wires_count; n++)
			{
				SignalOffsetPair* sp = &apu_clks_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					APUSim::APU_Interconnects wires{};
					board->apu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, APU_CORE_CATEGORY))
		{
			for (size_t n = 0; n < apu_core_wires_count; n++)
			{
				SignalOffsetPair* sp = &apu_core_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					APUSim::APU_Interconnects wires{};
					board->apu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, APU_DMA_CATEGORY))
		{
			for (size_t n = 0; n < apu_dma_wires_count; n++)
			{
				SignalOffsetPair* sp = &apu_dma_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					APUSim::APU_Interconnects wires{};
					board->apu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, APU_REGOPS_CATEGORY))
		{
			for (size_t n = 0; n < apu_regops_wires_count; n++)
			{
				SignalOffsetPair* sp = &apu_regops_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					APUSim::APU_Interconnects wires{};
					board->apu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, APU_LC_CATEGORY))
		{
			for (size_t n = 0; n < apu_lc_wires_count; n++)
			{
				SignalOffsetPair* sp = &apu_lc_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					APUSim::APU_Interconnects wires{};
					board->apu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, APU_WIRES_CATEGORY))
		{
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
		}

		return 0;
	}

	uint32_t NSFPlayerBoard::GetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;

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

	void NSFPlayerBoard::SetCoreDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not need
	}

	void NSFPlayerBoard::SetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;

		for (size_t n = 0; n < core_regs_count; n++)
		{
			SignalOffsetPair* sp = &core_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				board->core->setUserRegSingle((int)sp->offset, value);
			}
		}
	}

	void NSFPlayerBoard::SetApuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not need
	}

	void NSFPlayerBoard::SetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;

		for (size_t n = 0; n < apu_regs_count; n++)
		{
			SignalOffsetPair* sp = &apu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				board->apu->SetDebugInfo_Reg((int)sp->offset, value);
			}
		}
	}

	void NSFPlayerBoard::GetDebugInfo(NSFBoardDebugInfo& info)
	{
		info.CLK = CLK;
		for (int i = 0; i < 8; i++)
		{
			info.bank_reg[i] = sram->GetBankReg(i);
		}
		info.sync = ToByte(SYNC);
		info.ABus = addr_bus;
		info.DBus = data_bus;
		info.ResetPending = pendingReset;
	}

	uint32_t NSFPlayerBoard::GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;

		for (size_t n = 0; n < board_signals_count; n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			if (!strcmp(sp->name, entry->name))
			{
				NSFBoardDebugInfo info{};
				board->GetDebugInfo(info);

				uint8_t* ptr = (uint8_t*)&info + sp->offset;
				return *(uint32_t*)ptr;
			}
		}

		return 0;
	}

	void NSFPlayerBoard::SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		NSFPlayerBoard* board = (NSFPlayerBoard*)opaque;

		// wow!
		bool Bank = entry->name[0] == 'B' && entry->name[1] == 'a' && entry->name[2] == 'n' && entry->name[3] == 'k';
		if (Bank)
		{
			int reg_id = entry->name[4] - '0';
			board->sram->SetBankReg(reg_id, (uint8_t)value);
		}
	}
}
