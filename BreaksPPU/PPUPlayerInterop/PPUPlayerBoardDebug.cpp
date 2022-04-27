// This module aggregates all the debugging mechanisms of PPUPlayer board.

#include "pch.h"

using namespace BaseLogic;

#define VRAM_NAME "VRAM"
#define CHR_ROM_NAME "CHR-ROM"
#define PPU_WIRES_CATEGORY "PPU Wires"
#define PPU_FSM_CATEGORY "PPU FSM"

namespace PPUPlayer
{
	void Board::AddBoardMemDescriptors()
	{
		MemDesciptor* vramRegion = new MemDesciptor;
		memset(vramRegion, 0, sizeof(MemDesciptor));
		strcpy_s(vramRegion->name, sizeof(vramRegion->name), VRAM_NAME);
		vramRegion->size = vram->Dbg_GetSize();
		dbg_hub->AddMemRegion(vramRegion, DumpVRAM, this, false);

		// TBD: CRAM

		// TBD: OAM

		// TBD: OAM Temp
	}

	void Board::AddCartMemDescriptors()
	{
		MemDesciptor* chrRegion = new MemDesciptor;
		memset(chrRegion, 0, sizeof(MemDesciptor));
		strcpy_s(chrRegion->name, sizeof(chrRegion->name), CHR_ROM_NAME);
		chrRegion->size = cart->Dbg_GetCHRSize();
		dbg_hub->AddMemRegion(chrRegion, DumpCHR, this, false);
	}

	struct SignalOffsetPair
	{
		const char* name;
		const size_t offset;
	};

	SignalOffsetPair ppu_wires[] = {
		"R/W", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RnW),
		"RS[0]", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RS[0]),
		"RS[1]", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RS[1]),
		"RS[2]", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RS[2]),
		"/DBE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_DBE),
		"/ALE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ALE),
		"RD", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RD),
		"WR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::WR),
		"CLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CLK),
		"/CLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CLK),
		"RES", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RES),
		"RC", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RC),
		"PCLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PCLK),
		"/PCLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PCLK),
		"#CC0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[0]),
		"#CC1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[1]),
		"#CC2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[2]),
		"#CC3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[3]),
		"#LL0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_LL[0]),
		"#LL1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_LL[1]),
		"/RD", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_RD),
		"/WR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_WR),
		"/W6_1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W6_1),
		"/W6_2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W6_2),
		"/W5_1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W5_1),
		"/W5_2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W5_2),
		"/R7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_R7),
		"/W7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W7),
		"/W4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W4),
		"/W3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W3),
		"/R2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_R2),
		"/W1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W1),
		"/W0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W0),
		"/R4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_R4),
		"I1/32", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::I1_32),
		"OBSEL", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OBSEL),
		"BGSEL", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGSEL),
		"O8/16", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::O8_16),
		"/SLAVE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SLAVE),
		"VBL", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::VBL),
		"B/W", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BnW),
		"/BGCLIP", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_BGCLIP),
		"/OBCLIP", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OBCLIP),
		"BGE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGE),
		"BLACK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BLACK),
		"OBE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OBE),
		"/TR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TR),
		"/TG", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TG),
		"/TB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TB),
		"H0'", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H0_Dash),
		"H0''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H0_Dash2),
		"/H1'", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::nH1_Dash),
		"H1''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H1_Dash2),
		"/H2'", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::nH2_Dash),
		"H2''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H2_Dash2),
		"H3''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H3_Dash2),
		"H4''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H4_Dash2),
		"H5''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H5_Dash2),
		"EvenOddOut", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EvenOddOut),
		"HC", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::HC),
		"VC", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::VC),
		"/ZCOL0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ZCOL0),
		"/ZCOL1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ZCOL1),
		"ZCOL2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::ZCOL2),
		"ZCOL3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::ZCOL3),
		"/ZPRIO", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ZPRIO),
		"/SPR0HIT", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SPR0HIT),
		"EXT_IN0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[0]),
		"EXT_IN1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[1]),
		"EXT_IN2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[2]),
		"EXT_IN3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[3]),
		"/EXT_OUT0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[0]),
		"/EXT_OUT1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[1]),
		"/EXT_OUT2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[2]),
		"/EXT_OUT3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[3]),
		"OB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OB),
		"I2SEV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::I2SEV),
		"OFETCH", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OFETCH),
		"SPR_OV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::SPR_OV),
		"OAMCTR2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OAMCTR2),
		"/OAM0-7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OAM),
		"OAM8", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OAM8),
		"0/FIFO", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::Z_FIFO),
		"OV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OV),
		"CLPB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CLPB),
		"CLPO", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CLPO),
		"/SH2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SH2),
		"/SH3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SH3),
		"/SH5", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SH5),
		"/SH7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SH7),
		"/PA8", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[0]),
		"/PA9", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[1]),
		"/PA10", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[2]),
		"/PA11", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[3]),
		"/PA12", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[4]),
		"/PA13", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[5]),
		"BGC0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[0]),
		"BGC1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[1]),
		"BGC2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[2]),
		"BGC3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[3]),
		"THO0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[0]),
		"THO1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[1]),
		"THO2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[2]),
		"THO3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[3]),
		"THO4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[4]),
		"#CB/DB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CB_DB),
		"/BW", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_BW),
		"#DB/CB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_DB_CB),
		"PAL0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[0]),
		"PAL1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[1]),
		"PAL2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[2]),
		"PAL3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[3]),
		"PAL4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[4]),
		"TSTEP", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TSTEP),
		"DB/PAR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::DB_PAR),
		"PD/RB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PD_RB),
		"TH/MUX", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH_MUX),
		"XRB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::XRB),
		"DB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::DB),
		"PD", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PD),
	};

	SignalOffsetPair fsm_signals[] = {
		"S/EV", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::S_EV),
		"CLIP_O", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::CLIP_O),
		"CLIP_B", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::CLIP_B),
		"0/HPOS", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::Z_HPOS),
		"EVAL", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::EVAL),
		"E/EV", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::E_EV),
		"I/OAM2", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::I_OAM2),
		"PAR/O", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::PAR_O),
		"/VIS", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_VIS),
		"#F/NT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_F_NT),
		"F/TB", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_TB),
		"F/TA", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_TA),
		"F/AT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_AT),
		"/FO", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_FO),
		"BPORCH", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::BPORCH),
		"SC/CNT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::SC_CNT),
		"/HB", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_HB),
		"BURST", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::BURST),
		"HSYNC", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::HSYNC),
		"PICTURE", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::PICTURE),
		"RESCL", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::RESCL),
		"VSYNC", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::VSYNC),
		"/VSET", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_VSET),
		"VB", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::VB),
		"BLNK", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::BLNK),
		"INT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::INT),
	};

	void Board::AddDebugInfoProviders()
	{
		for (size_t n = 0; n < _countof(ppu_wires); n++)
		{
			SignalOffsetPair* sp = &ppu_wires[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), PPU_WIRES_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < _countof(fsm_signals); n++)
		{
			SignalOffsetPair* sp = &fsm_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), PPU_FSM_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, this);
		}
	}

	uint8_t Board::DumpVRAM(void* opaque, size_t addr)
	{
		Board* board = (Board*)opaque;
		return board->vram->Dbg_ReadByte(addr);
	}

	uint8_t Board::DumpCHR(void* opaque, size_t addr)
	{
		Board* board = (Board*)opaque;
		return board->cart->Dbg_ReadCHRByte(addr);
	}

	uint32_t Board::GetPpuDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		Board* board = (Board*)opaque;

		if (!strcmp(entry->category, PPU_WIRES_CATEGORY))
		{
			for (size_t n = 0; n < _countof(ppu_wires); n++)
			{
				SignalOffsetPair* sp = &ppu_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires;
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;

					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_FSM_CATEGORY))
		{
			for (size_t n = 0; n < _countof(fsm_signals); n++)
			{
				SignalOffsetPair* sp = &fsm_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_FSMStates fsm_states;
					board->ppu->GetDebugInfo_FSMStates(fsm_states);

					uint8_t* ptr = (uint8_t*)&fsm_states;

					return ptr[sp->offset];
				}
			}
		}

		return 0;
	}

	uint32_t Board::GetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry)
	{
		Board* board = (Board*)opaque;

		// TBD: Add regs dump in PPUSim.

		return 0;
	}
}
