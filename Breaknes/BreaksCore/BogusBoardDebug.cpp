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
		strcpy_s(wramRegion->name, sizeof(wramRegion->name), WRAM_NAME);
		wramRegion->size = (int32_t)wram->Dbg_GetSize();
		dbg_hub->AddMemRegion(wramRegion, DumpWRAM, WriteWRAM, this, false);
	}

	struct SignalOffsetPair
	{
		const char* name;
		const size_t offset;
		const uint8_t bits;
	};

	static SignalOffsetPair core_wires[] = {
		"IR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::IR), 8,
		"/PRDY", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_PRDY), 1,
		"/NMIP", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_NMIP), 1,
		"/IRQP", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_IRQP), 1,
		"RESP", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::RESP), 1,
		"BRK6E", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::BRK6E), 1,
		"BRK7", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::BRK7), 1,
		"DORES", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DORES), 1,
		"/DONMI", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_DONMI), 1,
		"/T2", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T2), 1,
		"/T3", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T3), 1,
		"/T4", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T4), 1,
		"/T5", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T5), 1,
		"T0", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::T0), 1,
		"/T0", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T0), 1,
		"/T1X", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_T1X), 1,
		"Z_IR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_IR), 1,
		"FETCH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::FETCH), 1,
		"/ready", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_ready), 1,
		"WR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::WR), 1,
		"TRES2", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::TRES2), 1,
		"ACRL1", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ACRL1), 1,
		"ACRL2", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ACRL2), 1,
		"T1", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::T1), 1,
		"T6", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::RMW_T6), 1,
		"T7", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::RMW_T7), 1,
		"ENDS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ENDS), 1,
		"ENDX", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ENDX), 1,
		"TRES1", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::TRES1), 1,
		"/TRESX", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_TRESX), 1,
		"BRFW", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::BRFW), 1,
		"/BRTAKEN", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_BRTAKEN), 1,
		"ACR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ACR), 1,
		"AVR", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::AVR), 1,

		"Y_SB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Y_SB), 1,
		"SB_Y", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_Y), 1,
		"X_SB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::X_SB), 1,
		"SB_X", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_X), 1,
		"S_ADL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::S_ADL), 1,
		"S_SB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::S_SB), 1,
		"SB_S", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_S), 1,
		"S_S", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::S_S), 1,
		"NDB_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::NDB_ADD), 1,
		"DB_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_ADD), 1,
		"Z_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADD), 1,
		"SB_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_ADD), 1,
		"ADL_ADD", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADL_ADD), 1,
		"ANDS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ANDS), 1,
		"EORS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::EORS), 1,
		"ORS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ORS), 1,
		"SRS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SRS), 1,
		"SUMS", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SUMS), 1,
		"ADD_SB7", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADD_SB7), 1,
		"ADD_SB06", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADD_SB06), 1,
		"ADD_ADL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADD_ADL), 1,
		"SB_AC", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_AC), 1,
		"AC_SB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::AC_SB), 1,
		"AC_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::AC_DB), 1,
		"ADH_PCH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADH_PCH), 1,
		"PCH_PCH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCH_PCH), 1,
		"PCH_ADH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCH_ADH), 1,
		"PCH_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCH_DB), 1,
		"ADL_PCL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADL_PCL), 1,
		"PCL_PCL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCL_PCL), 1,
		"PCL_ADL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCL_ADL), 1,
		"PCL_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::PCL_DB), 1,
		"ADH_ABH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADH_ABH), 1,
		"ADL_ABL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ADL_ABL), 1,
		"Z_ADL0", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADL0), 1,
		"Z_ADL1", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADL1), 1,
		"Z_ADL2", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADL2), 1,
		"Z_ADH0", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADH0), 1,
		"Z_ADH17", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_ADH17), 1,
		"SB_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_DB), 1,
		"SB_ADH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::SB_ADH), 1,
		"DL_ADL", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DL_ADL), 1,
		"DL_ADH", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DL_ADH), 1,
		"DL_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DL_DB), 1,

		"P_DB", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::P_DB), 1,
		"DB_P", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_P), 1,
		"DBZ_Z", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DBZ_Z), 1,
		"DB_N", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_N), 1,
		"IR5_C", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::IR5_C), 1,
		"DB_C", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_C), 1,
		"ACR_C", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::ACR_C), 1,
		"IR5_D", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::IR5_D), 1,
		"IR5_I", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::IR5_I), 1,
		"DB_V", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::DB_V), 1,
		"AVR_V", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::AVR_V), 1,
		"Z_V", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::Z_V), 1,
		"n_ACIN", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_ACIN), 1,
		"n_DAA", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_DAA), 1,
		"n_DSA", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_DSA), 1,
		"n_1PC", offsetof(M6502Core::DebugInfo, M6502Core::DebugInfo::n_1PC), 1,
	};

	static SignalOffsetPair core_regs[] = {
		"PCH", offsetof(M6502Core::UserRegs, PCH), 8,
		"PCL", offsetof(M6502Core::UserRegs, PCL), 8,
		"PCHS", offsetof(M6502Core::UserRegs, PCHS), 8,
		"PCLS", offsetof(M6502Core::UserRegs, PCLS), 8,
		"A", offsetof(M6502Core::UserRegs, A), 8,
		"X", offsetof(M6502Core::UserRegs, X), 8,
		"Y", offsetof(M6502Core::UserRegs, Y), 8,
		"S", offsetof(M6502Core::UserRegs, S), 8,
		"C_OUT", offsetof(M6502Core::UserRegs, C_OUT), 8,
		"Z_OUT", offsetof(M6502Core::UserRegs, Z_OUT), 8,
		"I_OUT", offsetof(M6502Core::UserRegs, I_OUT), 8,
		"D_OUT", offsetof(M6502Core::UserRegs, D_OUT), 8,
		"V_OUT", offsetof(M6502Core::UserRegs, V_OUT), 8,
		"N_OUT", offsetof(M6502Core::UserRegs, N_OUT), 8,
	};

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

	void BogusBoard::AddDebugInfoProviders()
	{
		for (size_t n = 0; n < _countof(core_wires); n++)
		{
			SignalOffsetPair* sp = &core_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), CORE_WIRES_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Core, entry, GetCoreDebugInfo, SetCoreDebugInfo, this);
		}

		for (size_t n = 0; n < _countof(core_regs); n++)
		{
			SignalOffsetPair* sp = &core_regs[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), CORE_REGS_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			entry->bits = sp->bits;
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_CoreRegs, entry, GetCoreRegsDebugInfo, SetCoreRegsDebugInfo, this);
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

		for (size_t n = 0; n < _countof(core_wires); n++)
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

		for (size_t n = 0; n < _countof(core_regs); n++)
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

		for (size_t n = 0; n < _countof(core_regs); n++)
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

		for (size_t n = 0; n < _countof(board_signals); n++)
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
