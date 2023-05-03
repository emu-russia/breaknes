// I thought I could do without the debug version of the NESBoard, but I couldn't.

#include "pch.h"

using namespace BaseLogic;

#define WRAM_NAME "WRAM"
#define VRAM_NAME "VRAM"
#define BOARD_CATEGORY "Board"

#define CRAM_SIZE (16+16)
#define OAM_SIZE 0x100
#define OAM2_SIZE 32

namespace Breaknes
{
	void NESBoard::AddBoardMemDescriptors()
	{
		// WRAM

		MemDesciptor* wramRegion = new MemDesciptor;
		memset(wramRegion, 0, sizeof(MemDesciptor));
		strcpy(wramRegion->name, WRAM_NAME);
		wramRegion->size = (int32_t)wram->Dbg_GetSize();
		dbg_hub->AddMemRegion(wramRegion, DumpWRAM, WriteWRAM, this, false);

		// VRAM

		MemDesciptor* vramRegion = new MemDesciptor;
		memset(vramRegion, 0, sizeof(MemDesciptor));
		strcpy(vramRegion->name, VRAM_NAME);
		vramRegion->size = (int32_t)vram->Dbg_GetSize();
		dbg_hub->AddMemRegion(vramRegion, DumpVRAM, WriteVRAM, this, false);

		// CRAM

		MemDesciptor* cramRegion = new MemDesciptor;
		memset(cramRegion, 0, sizeof(MemDesciptor));
		strcpy(cramRegion->name, CRAM_NAME);
		cramRegion->size = CRAM_SIZE;
		dbg_hub->AddMemRegion(cramRegion, DumpCRAM, WriteCRAM, this, false);

		// OAM

		MemDesciptor* oamRegion = new MemDesciptor;
		memset(oamRegion, 0, sizeof(MemDesciptor));
		strcpy(oamRegion->name, OAM_NAME);
		oamRegion->size = OAM_SIZE;
		dbg_hub->AddMemRegion(oamRegion, DumpOAM, WriteOAM, this, false);

		// Temp OAM

		MemDesciptor* oam2Region = new MemDesciptor;
		memset(oam2Region, 0, sizeof(MemDesciptor));
		strcpy(oam2Region->name, OAM2_NAME);
		oam2Region->size = OAM2_SIZE;
		dbg_hub->AddMemRegion(oam2Region, DumpTempOAM, WriteTempOAM, this, false);
	}

	static SignalOffsetPair board_signals[] = {
		"BoardCLK", offsetof(NESBoardDebugInfo, CLK), 1,
		"ABus", offsetof(NESBoardDebugInfo, ABus), 16,
		"DBus", offsetof(NESBoardDebugInfo, DBus), 8,
	};
	static size_t board_signals_count = sizeof(board_signals) / sizeof(board_signals[0]);

	void NESBoard::AddDebugInfoProviders()
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

		for (size_t n = 0; n < ppu_clks_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_clks_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_CLKS_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_cpu_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_cpu_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_CPU_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_ctrl_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_ctrl_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_CTRL_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_hv_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_hv_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_HV_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_mux_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_mux_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_MUX_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_spg_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_spg_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_SPG_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_cram_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_cram_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_CRAM_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_vram_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_vram_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_VRAM_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_fsm_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_fsm_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_FSM_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_eval_signals_count; n++)
		{
			SignalOffsetPair* sp = &ppu_eval_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_EVAL_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_wires_count; n++)
		{
			SignalOffsetPair* sp = &ppu_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_WIRES_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, SetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < ppu_regs_count; n++)
		{
			SignalOffsetPair* sp = &ppu_regs[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy(entry->category, PPU_REGS_CATEGORY);
			strcpy(entry->name, sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPURegs, entry, GetPpuRegsDebugInfo, SetPpuRegsDebugInfo, this);
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

	uint8_t NESBoard::DumpWRAM(void* opaque, size_t addr)
	{
		NESBoard* board = (NESBoard*)opaque;
		return board->wram->Dbg_ReadByte(addr);
	}

	uint8_t NESBoard::DumpVRAM(void* opaque, size_t addr)
	{
		NESBoard* board = (NESBoard*)opaque;
		return board->vram->Dbg_ReadByte(addr);
	}

	uint8_t NESBoard::DumpCRAM(void* opaque, size_t addr)
	{
		NESBoard* board = (NESBoard*)opaque;
		return board->ppu->Dbg_CRAMReadByte(addr);
	}

	uint8_t NESBoard::DumpOAM(void* opaque, size_t addr)
	{
		NESBoard* board = (NESBoard*)opaque;
		return board->ppu->Dbg_OAMReadByte(addr);
	}

	uint8_t NESBoard::DumpTempOAM(void* opaque, size_t addr)
	{
		NESBoard* board = (NESBoard*)opaque;
		return board->ppu->Dbg_TempOAMReadByte(addr);
	}

	void NESBoard::WriteWRAM(void* opaque, size_t addr, uint8_t data)
	{
		NESBoard* board = (NESBoard*)opaque;
		board->wram->Dbg_WriteByte(addr, data);
	}

	void NESBoard::WriteVRAM(void* opaque, size_t addr, uint8_t data)
	{
		NESBoard* board = (NESBoard*)opaque;
		board->vram->Dbg_WriteByte(addr, data);
	}

	void NESBoard::WriteCRAM(void* opaque, size_t addr, uint8_t data)
	{
		NESBoard* board = (NESBoard*)opaque;
		board->ppu->Dbg_CRAMWriteByte(addr, data);
	}

	void NESBoard::WriteOAM(void* opaque, size_t addr, uint8_t data)
	{
		NESBoard* board = (NESBoard*)opaque;
		board->ppu->Dbg_OAMWriteByte(addr, data);
	}

	void NESBoard::WriteTempOAM(void* opaque, size_t addr, uint8_t data)
	{
		NESBoard* board = (NESBoard*)opaque;
		board->ppu->Dbg_TempOAMWriteByte(addr, data);
	}

	uint32_t NESBoard::GetCoreDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NESBoard* board = (NESBoard*)opaque;

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

	uint32_t NESBoard::GetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NESBoard* board = (NESBoard*)opaque;

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

	void NESBoard::SetCoreDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not need
	}

	void NESBoard::SetCoreRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		NESBoard* board = (NESBoard*)opaque;

		for (size_t n = 0; n < core_regs_count; n++)
		{
			SignalOffsetPair* sp = &core_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				board->core->setUserRegSingle((int)sp->offset, value);
			}
		}
	}

	uint32_t NESBoard::GetApuDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NESBoard* board = (NESBoard*)opaque;

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

	uint32_t NESBoard::GetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NESBoard* board = (NESBoard*)opaque;

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

	void NESBoard::SetApuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not need
	}

	void NESBoard::SetApuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		NESBoard* board = (NESBoard*)opaque;

		for (size_t n = 0; n < apu_regs_count; n++)
		{
			SignalOffsetPair* sp = &apu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				board->apu->SetDebugInfo_Reg((int)sp->offset, value);
			}
		}
	}

	uint32_t NESBoard::GetPpuDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NESBoard* board = (NESBoard*)opaque;

		if (!strcmp(entry->category, PPU_CLKS_CATEGORY))
		{
			for (size_t n = 0; n < ppu_clks_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_clks_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires{};
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_CPU_CATEGORY))
		{
			for (size_t n = 0; n < ppu_cpu_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_cpu_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires{};
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_CTRL_CATEGORY))
		{
			for (size_t n = 0; n < ppu_ctrl_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_ctrl_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires{};
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_HV_CATEGORY))
		{
			for (size_t n = 0; n < ppu_hv_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_hv_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires{};
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_MUX_CATEGORY))
		{
			for (size_t n = 0; n < ppu_mux_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_mux_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires{};
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_SPG_CATEGORY))
		{
			for (size_t n = 0; n < ppu_spg_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_spg_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires{};
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_CRAM_CATEGORY))
		{
			for (size_t n = 0; n < ppu_cram_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_cram_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires{};
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_VRAM_CATEGORY))
		{
			for (size_t n = 0; n < ppu_vram_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_vram_signals[n];

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
			for (size_t n = 0; n < ppu_fsm_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_fsm_signals[n];

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
			for (size_t n = 0; n < ppu_eval_signals_count; n++)
			{
				SignalOffsetPair* sp = &ppu_eval_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::OAMEvalWires wires{};
					board->ppu->GetDebugInfo_OAMEval(wires);

					uint8_t* ptr = (uint8_t*)&wires;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_WIRES_CATEGORY))
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

		return 0;
	}

	void NESBoard::SetPpuDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{

	}

	uint32_t NESBoard::GetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NESBoard* board = (NESBoard*)opaque;

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

	void NESBoard::SetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		NESBoard* board = (NESBoard*)opaque;

		for (size_t n = 0; n < ppu_regs_count; n++)
		{
			SignalOffsetPair* sp = &ppu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				board->ppu->Dbg_WriteRegister((int)sp->offset, value);
			}
		}
	}

	void NESBoard::GetDebugInfo(NESBoardDebugInfo& info)
	{
		info.CLK = CLK;
		info.ABus = addr_bus;
		info.DBus = data_bus;
	}

	uint32_t NESBoard::GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		NESBoard* board = (NESBoard*)opaque;

		for (size_t n = 0; n < board_signals_count; n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			if (!strcmp(sp->name, entry->name))
			{
				NESBoardDebugInfo info{};
				board->GetDebugInfo(info);

				uint8_t* ptr = (uint8_t*)&info + sp->offset;
				return *(uint32_t*)ptr;
			}
		}

		return 0;
	}

	void NESBoard::SetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint32_t value)
	{
		// not used
	}
}
