// This module aggregates all the debugging mechanisms of PPUPlayer board.

#include "pch.h"

using namespace BaseLogic;

#define VRAM_NAME "VRAM"
#define CRAM_NAME "Color RAM"
#define OAM_NAME "OAM"
#define OAM2_NAME "Temp OAM"
#define PPU_WIRES_CATEGORY "PPU Wires"
#define PPU_FSM_CATEGORY "PPU FSM"
#define PPU_EVAL_CATEGORY "PPU Eval"
#define PPU_REGS_CATEGORY "PPU Regs"
#define BOARD_CATEGORY "Board"

#define CRAM_SIZE (16+16)
#define OAM_SIZE 0x100
#define OAM2_SIZE 32

namespace Breaknes
{
	void PPUPlayerBoard::AddBoardMemDescriptors()
	{
		// VRAM

		MemDesciptor* vramRegion = new MemDesciptor;
		memset(vramRegion, 0, sizeof(MemDesciptor));
		strcpy_s(vramRegion->name, sizeof(vramRegion->name), VRAM_NAME);
		vramRegion->size = (int32_t)vram->Dbg_GetSize();
		dbg_hub->AddMemRegion(vramRegion, DumpVRAM, WriteVRAM, this, false);

		// CRAM

		MemDesciptor* cramRegion = new MemDesciptor;
		memset(cramRegion, 0, sizeof(MemDesciptor));
		strcpy_s(cramRegion->name, sizeof(cramRegion->name), CRAM_NAME);
		cramRegion->size = CRAM_SIZE;
		dbg_hub->AddMemRegion(cramRegion, DumpCRAM, WriteCRAM, this, false);

		// OAM

		MemDesciptor* oamRegion = new MemDesciptor;
		memset(oamRegion, 0, sizeof(MemDesciptor));
		strcpy_s(oamRegion->name, sizeof(oamRegion->name), OAM_NAME);
		oamRegion->size = OAM_SIZE;
		dbg_hub->AddMemRegion(oamRegion, DumpOAM, WriteOAM, this, false);

		// Temp OAM

		MemDesciptor* oam2Region = new MemDesciptor;
		memset(oam2Region, 0, sizeof(MemDesciptor));
		strcpy_s(oam2Region->name, sizeof(oam2Region->name), OAM2_NAME);
		oam2Region->size = OAM2_SIZE;
		dbg_hub->AddMemRegion(oam2Region, DumpTempOAM, WriteTempOAM, this, false);
	}

	SignalOffsetPair board_signals[] = {
		"BoardCLK", offsetof(PPUBoardDebugInfo, CLK), 1,
		"ALE", offsetof(PPUBoardDebugInfo, ALE), 1,
		"LS373 Latch", offsetof(PPUBoardDebugInfo, LS373_Latch), 8,
		"VRAM Address", offsetof(PPUBoardDebugInfo, VRAM_Addr), 16,
		"#VRAM_CS", offsetof(PPUBoardDebugInfo, n_VRAM_CS), 1,
		"VRAM_A10", offsetof(PPUBoardDebugInfo, VRAM_A10), 1,
		"PA", offsetof(PPUBoardDebugInfo, PA), 16,
		"#PA13", offsetof(PPUBoardDebugInfo, n_PA13), 1,
		"#RD", offsetof(PPUBoardDebugInfo, n_RD), 1,
		"#WR", offsetof(PPUBoardDebugInfo, n_WR), 1,
		"#INT", offsetof(PPUBoardDebugInfo, n_INT), 1,
		"PDBus", offsetof(PPUBoardDebugInfo, PD), 8,
		"CPUOpsProcessed", offsetof(PPUBoardDebugInfo, CPUOpsProcessed), 32,
	};

	void PPUPlayerBoard::AddDebugInfoProviders()
	{
		for (size_t n = 0; n < ppu_wires_count; n++)
		{
			SignalOffsetPair* sp = &ppu_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), PPU_WIRES_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < fsm_signals_count; n++)
		{
			SignalOffsetPair* sp = &fsm_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), PPU_FSM_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < eval_signals_count; n++)
		{
			SignalOffsetPair* sp = &eval_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), PPU_EVAL_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_regs_count; n++)
		{
			SignalOffsetPair* sp = &ppu_regs[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), PPU_REGS_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPURegs, entry, GetPpuRegsDebugInfo, SetPpuRegsDebugInfo, this);
		}

		for (size_t n = 0; n < _countof(board_signals); n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), BOARD_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Board, entry, GetBoardDebugInfo, SetBoardDebugInfo, this);
		}
	}

	uint8_t PPUPlayerBoard::DumpVRAM(void* opaque, size_t addr)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;
		return board->vram->Dbg_ReadByte(addr);
	}

	uint8_t PPUPlayerBoard::DumpCRAM(void* opaque, size_t addr)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;
		return board->ppu->Dbg_CRAMReadByte(addr);
	}

	uint8_t PPUPlayerBoard::DumpOAM(void* opaque, size_t addr)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;
		return board->ppu->Dbg_OAMReadByte(addr);
	}

	uint8_t PPUPlayerBoard::DumpTempOAM(void* opaque, size_t addr)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;
		return board->ppu->Dbg_TempOAMReadByte(addr);
	}

	void PPUPlayerBoard::WriteVRAM(void* opaque, size_t addr, uint8_t data)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;
		board->vram->Dbg_WriteByte(addr, data);
	}

	void PPUPlayerBoard::WriteCRAM(void* opaque, size_t addr, uint8_t data)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;
		board->ppu->Dbg_CRAMWriteByte(addr, data);
	}

	void PPUPlayerBoard::WriteOAM(void* opaque, size_t addr, uint8_t data)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;
		board->ppu->Dbg_OAMWriteByte(addr, data);
	}

	void PPUPlayerBoard::WriteTempOAM(void* opaque, size_t addr, uint8_t data)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;
		board->ppu->Dbg_TempOAMWriteByte(addr, data);
	}

	uint32_t PPUPlayerBoard::GetPpuDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;

		if (!strcmp(entry->category, PPU_WIRES_CATEGORY))
		{
			for (size_t n = 0; n < ppu_wires_count; n++)
			{
				SignalOffsetPair* sp = &ppu_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires{};
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_FSM_CATEGORY))
		{
			for (size_t n = 0; n < fsm_signals_count; n++)
			{
				SignalOffsetPair* sp = &fsm_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_FSMStates fsm_states{};
					board->ppu->GetDebugInfo_FSMStates(fsm_states);

					uint8_t* ptr = (uint8_t*)&fsm_states;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_EVAL_CATEGORY))
		{
			for (size_t n = 0; n < eval_signals_count; n++)
			{
				SignalOffsetPair* sp = &eval_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::OAMEvalWires wires{};
					board->ppu->GetDebugInfo_OAMEval(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		return 0;
	}

	void PPUPlayerBoard::SetPpuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{

	}

	uint32_t PPUPlayerBoard::GetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;

		for (size_t n = 0; n < ppu_regs_count; n++)
		{
			SignalOffsetPair* sp = &ppu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				return board->ppu->Dbg_ReadRegister((int)sp->offset);
			}
		}

		return 0;
	}

	void PPUPlayerBoard::SetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;

		for (size_t n = 0; n < ppu_regs_count; n++)
		{
			SignalOffsetPair* sp = &ppu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				board->ppu->Dbg_WriteRegister((int)sp->offset, value);
			}
		}
	}

	void PPUPlayerBoard::GetDebugInfo(PPUBoardDebugInfo& info)
	{
		info.CLK = CLK;
		info.ALE = ALE;
		info.LS373_Latch = LatchedAddress;
		info.VRAM_Addr = VRAM_Addr;
		info.n_VRAM_CS = n_VRAM_CS;
		info.VRAM_A10 = VRAM_A10;
		info.PA = ppu_addr;
		info.n_PA13 = n_PA13;
		info.n_RD = n_RD;
		info.n_WR = n_WR;
		info.n_INT = n_INT;
		info.PD = ad_bus;
		info.CPUOpsProcessed = CPUOpsProcessed;
	}

	uint32_t PPUPlayerBoard::GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		PPUPlayerBoard* board = (PPUPlayerBoard*)opaque;

		for (size_t n = 0; n < _countof(board_signals); n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			if (!strcmp(sp->name, entry->name))
			{
				PPUBoardDebugInfo info{};
				board->GetDebugInfo(info);

				uint8_t* ptr = (uint8_t*)&info + sp->offset;
				return *(uint32_t*)ptr;
			}
		}

		return 0;
	}

	void PPUPlayerBoard::SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not need
	}
}
