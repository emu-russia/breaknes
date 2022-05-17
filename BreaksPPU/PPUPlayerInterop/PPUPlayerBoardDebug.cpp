// This module aggregates all the debugging mechanisms of PPUPlayer board.

#include "pch.h"

using namespace BaseLogic;

#define VRAM_NAME "VRAM"
#define CRAM_NAME "Color RAM"
#define OAM_NAME "OAM"
#define OAM2_NAME "Temp OAM"
#define CHR_ROM_NAME "CHR-ROM"
#define PPU_WIRES_CATEGORY "PPU Wires"
#define PPU_FSM_CATEGORY "PPU FSM"
#define PPU_EVAL_CATEGORY "PPU Eval"
#define PPU_REGS_CATEGORY "PPU Regs"
#define BOARD_CATEGORY "PPUPlayer Board"
#define NROM_CATEGORY "NROM"

#define CRAM_SIZE (16+16)
#define OAM_SIZE 0x100
#define OAM2_SIZE 32

namespace PPUPlayer
{
	void Board::AddBoardMemDescriptors()
	{
		// VRAM

		MemDesciptor* vramRegion = new MemDesciptor;
		memset(vramRegion, 0, sizeof(MemDesciptor));
		strcpy_s(vramRegion->name, sizeof(vramRegion->name), VRAM_NAME);
		vramRegion->size = vram->Dbg_GetSize();
		dbg_hub->AddMemRegion(vramRegion, DumpVRAM, this, false);

		// CRAM

		MemDesciptor* cramRegion = new MemDesciptor;
		memset(cramRegion, 0, sizeof(MemDesciptor));
		strcpy_s(cramRegion->name, sizeof(cramRegion->name), CRAM_NAME);
		cramRegion->size = CRAM_SIZE;
		dbg_hub->AddMemRegion(cramRegion, DumpCRAM, this, false);

		// OAM

		MemDesciptor* oamRegion = new MemDesciptor;
		memset(oamRegion, 0, sizeof(MemDesciptor));
		strcpy_s(oamRegion->name, sizeof(oamRegion->name), OAM_NAME);
		oamRegion->size = OAM_SIZE;
		dbg_hub->AddMemRegion(oamRegion, DumpOAM, this, false);

		// Temp OAM

		MemDesciptor* oam2Region = new MemDesciptor;
		memset(oam2Region, 0, sizeof(MemDesciptor));
		strcpy_s(oam2Region->name, sizeof(oam2Region->name), OAM2_NAME);
		oam2Region->size = OAM2_SIZE;
		dbg_hub->AddMemRegion(oam2Region, DumpTempOAM, this, false);
	}

	void Board::AddCartMemDescriptors()
	{
		MemDesciptor* chrRegion = new MemDesciptor;
		memset(chrRegion, 0, sizeof(MemDesciptor));
		strcpy_s(chrRegion->name, sizeof(chrRegion->name), CHR_ROM_NAME);
		chrRegion->size = cart->Dbg_GetCHRSize();
		dbg_hub->AddMemRegion(chrRegion, DumpCHR, this, true);
	}

	struct SignalOffsetPair
	{
		const char* name;
		const size_t offset;
		const uint8_t bits;
	};

	SignalOffsetPair ppu_wires[] = {
		"R/W", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RnW), 1,
		"RS[0]", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RS[0]), 1,
		"RS[1]", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RS[1]), 1,
		"RS[2]", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RS[2]), 1,
		"/DBE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_DBE), 1,
		"/ALE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ALE), 1,
		"CLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CLK), 1,
		"/CLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CLK), 1,
		"RES", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RES), 1,
		"RC", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RC), 1,
		"PCLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PCLK), 1,
		"/PCLK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PCLK), 1,
		"#CC0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[0]), 1,
		"#CC1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[1]), 1,
		"#CC2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[2]), 1,
		"#CC3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CC[3]), 1,
		"#LL0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_LL[0]), 1,
		"#LL1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_LL[1]), 1,
		"/RD", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_RD), 1,
		"/WR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_WR), 1,
		"/W6_1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W6_1), 1,
		"/W6_2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W6_2), 1,
		"/W5_1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W5_1), 1,
		"/W5_2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W5_2), 1,
		"/R7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_R7), 1,
		"/W7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W7), 1,
		"/W4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W4), 1,
		"/W3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W3), 1,
		"/R2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_R2), 1,
		"/W1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W1), 1,
		"/W0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_W0), 1,
		"/R4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_R4), 1,
		"I1/32", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::I1_32), 1,
		"OBSEL", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OBSEL), 1,
		"BGSEL", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGSEL), 1,
		"O8/16", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::O8_16), 1,
		"/SLAVE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SLAVE), 1,
		"VBL", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::VBL), 1,
		"B/W", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BnW), 1,
		"/BGCLIP", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_BGCLIP), 1,
		"/OBCLIP", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OBCLIP), 1,
		"BGE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGE), 1,
		"BLACK", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BLACK), 1,
		"OBE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OBE), 1,
		"/TR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TR), 1,
		"/TG", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TG), 1,
		"/TB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TB), 1,
		"H0'", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H0_Dash), 1,
		"H0''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H0_Dash2), 1,
		"/H1'", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::nH1_Dash), 1,
		"H1''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H1_Dash2), 1,
		"/H2'", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::nH2_Dash), 1,
		"H2''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H2_Dash2), 1,
		"H3''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H3_Dash2), 1,
		"H4''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H4_Dash2), 1,
		"H5''", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::H5_Dash2), 1,
		"EvenOddOut", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EvenOddOut), 1,
		"HC", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::HC), 1,
		"VC", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::VC), 1,
		"/ZCOL0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ZCOL0), 1,
		"/ZCOL1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ZCOL1), 1,
		"ZCOL2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::ZCOL2), 1,
		"ZCOL3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::ZCOL3), 1,
		"/ZPRIO", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_ZPRIO), 1,
		"/SPR0HIT", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SPR0HIT), 1,
		"EXT_IN0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[0]), 1,
		"EXT_IN1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[1]), 1,
		"EXT_IN2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[2]), 1,
		"EXT_IN3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::EXT_In[3]), 1,
		"/EXT_OUT0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[0]), 1,
		"/EXT_OUT1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[1]), 1,
		"/EXT_OUT2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[2]), 1,
		"/EXT_OUT3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_EXT_Out[3]), 1,
		"OB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OB), 8,
		"/SPR0_EV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SPR0_EV), 1,
		"OFETCH", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OFETCH), 1,
		"SPR_OV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::SPR_OV), 1,
		"OAMCTR2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OAMCTR2), 1,
		"/OAM0-7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_OAM), 8,
		"OAM8", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OAM8), 1,
		"PD/FIFO", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PD_FIFO), 1,
		"OV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::OV), 8,
		"/WE", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_WE), 1,
		"CLPB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CLPB), 1,
		"CLPO", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::CLPO), 1,
		"/SH2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SH2), 1,
		"/SH3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SH3), 1,
		"/SH5", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SH5), 1,
		"/SH7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_SH7), 1,
		"/PA0-7", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Bot), 8,
		"/PA8", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[0]), 1,
		"/PA9", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[1]), 1,
		"/PA10", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[2]), 1,
		"/PA11", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[3]), 1,
		"/PA12", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[4]), 1,
		"/PA13", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_PA_Top[5]), 1,
		"BGC0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[0]), 1,
		"BGC1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[1]), 1,
		"BGC2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[2]), 1,
		"BGC3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::BGC[3]), 1,
		"FH0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FH[0]), 1,
		"FH1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FH[1]), 1,
		"FH2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FH[2]), 1,
		"FV0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FV[0]), 1,
		"FV1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FV[1]), 1,
		"FV2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FV[2]), 1,
		"NTV", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::NTV), 1,
		"NTH", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::NTH), 1,
		"TV0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[0]), 1,
		"TV1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[1]), 1,
		"TV2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[2]), 1,
		"TV3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[3]), 1,
		"TV4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TV[4]), 1,
		"TH0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[0]), 1,
		"TH1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[1]), 1,
		"TH2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[2]), 1,
		"TH3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[3]), 1,
		"TH4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH[4]), 1,
		"THO0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[0]), 1,
		"THO1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[1]), 1,
		"THO2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[2]), 1,
		"THO3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[3]), 1,
		"THO4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::THO[4]), 1,
		"TVO0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[0]), 1,
		"TVO1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[1]), 1,
		"TVO2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[2]), 1,
		"TVO3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[3]), 1,
		"TVO4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TVO[4]), 1,
		"FVO0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FVO[0]), 1,
		"FVO1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FVO[1]), 1,
		"FVO2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::FVO[2]), 1,
		"/FVO0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_FVO[0]), 1,
		"/FVO1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_FVO[1]), 1,
		"/FVO2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_FVO[2]), 1,
		"#CB/DB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_CB_DB), 1,
		"/BW", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_BW), 1,
		"#DB/CB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_DB_CB), 1,
		"PAL0", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[0]), 1,
		"PAL1", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[1]), 1,
		"PAL2", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[2]), 1,
		"PAL3", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[3]), 1,
		"PAL4", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PAL[4]), 1,
		"RD", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::RD), 1,
		"WR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::WR), 1,
		"TSTEP", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TSTEP), 1,
		"DB/PAR", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::DB_PAR), 1,
		"PD/RB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PD_RB), 1,
		"TH/MUX", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::TH_MUX), 1,
		"XRB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::XRB), 1,
		"DB", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::DB), 8,
		"PD", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PD), 8,
		"PrioZ", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::PrioZ), 8,
		"/TX", offsetof(PPUSim::PPU_Interconnects, PPUSim::PPU_Interconnects::n_TX), 8,
	};

	SignalOffsetPair fsm_signals[] = {
		"S/EV", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::S_EV), 1,
		"CLIP_O", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::CLIP_O), 1,
		"CLIP_B", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::CLIP_B), 1,
		"0/HPOS", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::Z_HPOS), 1,
		"EVAL", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::EVAL), 1,
		"E/EV", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::E_EV), 1,
		"I/OAM2", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::I_OAM2), 1,
		"PAR/O", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::PAR_O), 1,
		"/VIS", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_VIS), 1,
		"F/NT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_NT), 1,
		"F/TB", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_TB), 1,
		"F/TA", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_TA), 1,
		"F/AT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::F_AT), 1,
		"/FO", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_FO), 1,
		"BPORCH", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::BPORCH), 1,
		"SC/CNT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::SC_CNT), 1,
		"/HB", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_HB), 1,
		"BURST", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::BURST), 1,
		"HSYNC", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::HSYNC), 1,
		"/PICTURE", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_PICTURE), 1,
		"RESCL", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::RESCL), 1,
		"VSYNC", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::VSYNC), 1,
		"/VSET", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::n_VSET), 1,
		"VB", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::VB), 1,
		"BLNK", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::BLNK), 1,
		"INT", offsetof(PPUSim::PPU_FSMStates, PPUSim::PPU_FSMStates::INT), 1,
	};

	SignalOffsetPair eval_signals[] = {
		"OMFG", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OMFG), 1,
		"OMSTEP", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OMSTEP), 1,
		"OMOUT", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OMOUT), 1,
		"OMV", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OMV), 1,
		"OSTEP", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OSTEP), 1,
		"ORES", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::ORES), 1,
		"TMV", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::TMV), 1,
		"OAP", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OAP), 1,
		"COPY_STEP", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::COPY_STEP), 1,
		"DO_COPY", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::DO_COPY), 1,
		"COPY_OVF", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::COPY_OVF), 1,
		"OVZ", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OVZ), 1,
		"OBCmpr", offsetof(PPUSim::OAMEvalWires, PPUSim::OAMEvalWires::OBCmpr), 8,
	};

	SignalOffsetPair ppu_regs[] = {
		"HCounter", offsetof(PPUSim::PPU_Registers, HCounter), 16,
		"VCounter", offsetof(PPUSim::PPU_Registers, VCounter), 16,
		"CTRL0", offsetof(PPUSim::PPU_Registers, CTRL0), 8,
		"CTRL1", offsetof(PPUSim::PPU_Registers, CTRL1), 8,
		"MainOAMCounter", offsetof(PPUSim::PPU_Registers, MainOAMCounter), 8,
		"TempOAMCounter", offsetof(PPUSim::PPU_Registers, TempOAMCounter), 8,
		"OB", offsetof(PPUSim::PPU_Registers, OB), 8,
		"RB", offsetof(PPUSim::PPU_Registers, RB), 8,
		"SCC_FH", offsetof(PPUSim::PPU_Registers, SCC_FH), 8,
		"SCC_FV", offsetof(PPUSim::PPU_Registers, SCC_FV), 8,
		"SCC_NTV", offsetof(PPUSim::PPU_Registers, SCC_NTV), 8,
		"SCC_NTH", offsetof(PPUSim::PPU_Registers, SCC_NTH), 8,
		"SCC_TV", offsetof(PPUSim::PPU_Registers, SCC_TV), 8,
		"SCC_TH", offsetof(PPUSim::PPU_Registers, SCC_TH), 8,
	};

	SignalOffsetPair board_signals[] = {
		"BoardCLK", offsetof(BoardDebugInfo, CLK), 1,
		"ALE", offsetof(BoardDebugInfo, ALE), 1,
		"LS373 Latch", offsetof(BoardDebugInfo, LS373_Latch), 8,
		"VRAM Address", offsetof(BoardDebugInfo, VRAM_Addr), 16,
		"#VRAM_CS", offsetof(BoardDebugInfo, n_VRAM_CS), 1,
		"VRAM_A10", offsetof(BoardDebugInfo, VRAM_A10), 1,
		"PA", offsetof(BoardDebugInfo, PA), 16,
		"#PA13", offsetof(BoardDebugInfo, n_PA13), 1,
		"#RD", offsetof(BoardDebugInfo, n_RD), 1,
		"#WR", offsetof(BoardDebugInfo, n_WR), 1,
		"#INT", offsetof(BoardDebugInfo, n_INT), 1,
		"PDBus", offsetof(BoardDebugInfo, PD), 8,
	};

	SignalOffsetPair nrom_signals[] = {
		"Last PA", offsetof(NROM_DebugInfo, last_PA), 16,
		"Last /RD", offsetof(NROM_DebugInfo, last_nRD), 1,
		"Last /WR", offsetof(NROM_DebugInfo, last_nWR), 1,
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

		for (size_t n = 0; n < _countof(eval_signals); n++)
		{
			SignalOffsetPair* sp = &eval_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), PPU_EVAL_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPU, entry, GetPpuDebugInfo, this);
		}

		for (size_t n = 0; n < _countof(ppu_regs); n++)
		{
			SignalOffsetPair* sp = &ppu_regs[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), PPU_REGS_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_PPURegs, entry, GetPpuRegsDebugInfo, this);
		}

		for (size_t n = 0; n < _countof(board_signals); n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), BOARD_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Board, entry, GetBoardDebugInfo, this);
		}
	}

	void Board::AddCartDebugInfoProviders()
	{
		for (size_t n = 0; n < _countof(nrom_signals); n++)
		{
			SignalOffsetPair* sp = &nrom_signals[n];

			DebugInfoEntry* entry = new DebugInfoEntry;
			memset(entry, 0, sizeof(DebugInfoEntry));
			strcpy_s(entry->category, sizeof(entry->category), NROM_CATEGORY);
			strcpy_s(entry->name, sizeof(entry->name), sp->name);
			dbg_hub->AddDebugInfo(DebugInfoType::DebugInfoType_Cart, entry, GetCartDebugInfo, this);
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

	uint8_t Board::DumpCRAM(void* opaque, size_t addr)
	{
		Board* board = (Board*)opaque;
		return board->ppu->Dbg_CRAMReadByte(addr);
	}

	uint8_t Board::DumpOAM(void* opaque, size_t addr)
	{
		Board* board = (Board*)opaque;
		return board->ppu->Dbg_OAMReadByte(addr);
	}

	uint8_t Board::DumpTempOAM(void* opaque, size_t addr)
	{
		Board* board = (Board*)opaque;
		return board->ppu->Dbg_TempOAMReadByte(addr);
	}

	uint32_t Board::GetPpuDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits)
	{
		Board* board = (Board*)opaque;

		if (!strcmp(entry->category, PPU_WIRES_CATEGORY))
		{
			for (size_t n = 0; n < _countof(ppu_wires); n++)
			{
				SignalOffsetPair* sp = &ppu_wires[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::PPU_Interconnects wires{};
					board->ppu->GetDebugInfo_Wires(wires);

					uint8_t* ptr = (uint8_t*)&wires;

					bits = sp->bits;
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
					PPUSim::PPU_FSMStates fsm_states{};
					board->ppu->GetDebugInfo_FSMStates(fsm_states);

					uint8_t* ptr = (uint8_t*)&fsm_states;

					bits = sp->bits;
					return ptr[sp->offset];
				}
			}
		}

		else if (!strcmp(entry->category, PPU_EVAL_CATEGORY))
		{
			for (size_t n = 0; n < _countof(eval_signals); n++)
			{
				SignalOffsetPair* sp = &eval_signals[n];

				if (!strcmp(sp->name, entry->name))
				{
					PPUSim::OAMEvalWires wires{};
					board->ppu->GetDebugInfo_OAMEval(wires);

					uint8_t* ptr = (uint8_t*)&wires;

					bits = sp->bits;
					return ptr[sp->offset];
				}
			}
		}

		return 0;
	}

	uint32_t Board::GetPpuRegsDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits)
	{
		Board* board = (Board*)opaque;

		for (size_t n = 0; n < _countof(ppu_regs); n++)
		{
			SignalOffsetPair* sp = &ppu_regs[n];

			if (!strcmp(sp->name, entry->name))
			{
				PPUSim::PPU_Registers regs{};
				board->ppu->GetDebugInfo_Regs(regs);

				bits = sp->bits;
				uint8_t* ptr = (uint8_t*)&regs + sp->offset;
				return *(uint32_t*)ptr;
			}
		}

		return 0;
	}

	uint32_t Board::GetCartDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits)
	{
		Board* board = (Board*)opaque;

		if (!board->cart)
			return 0;

		for (size_t n = 0; n < _countof(nrom_signals); n++)
		{
			SignalOffsetPair* sp = &nrom_signals[n];

			if (!strcmp(sp->name, entry->name))
			{
				NROM_DebugInfo nrom_info{};
				board->cart->GetDebugInfo(nrom_info);

				bits = sp->bits;
				uint8_t* ptr = (uint8_t*)&nrom_info + sp->offset;
				return *(uint32_t *)ptr;
			}
		}

		return 0;
	}

	void Board::GetDebugInfo(BoardDebugInfo& info)
	{
		info.CLK = CLK;
		info.ALE = ALE;
		info.LS373_Latch = LatchedAddress;
		info.VRAM_Addr = VRAM_Addr;
		info.n_VRAM_CS = n_VRAM_CS;
		info.VRAM_A10 = VRAM_A10;

		info.PA = 0;
		for (size_t n = 0; n < _countof(PA); n++)
		{
			info.PA |= (PA[n] == TriState::One ? 1ULL : 0) << n;
		}

		info.n_PA13 = n_PA13;
		info.n_RD = n_RD;
		info.n_WR = n_WR;
		info.n_INT = n_INT;
		info.PD = ad_bus;
	}

	uint32_t Board::GetBoardDebugInfo(void* opaque, DebugInfoEntry* entry, uint8_t& bits)
	{
		Board* board = (Board*)opaque;

		for (size_t n = 0; n < _countof(board_signals); n++)
		{
			SignalOffsetPair* sp = &board_signals[n];

			if (!strcmp(sp->name, entry->name))
			{
				BoardDebugInfo info{};
				board->GetDebugInfo(info);

				bits = sp->bits;
				uint8_t* ptr = (uint8_t*)&info + sp->offset;
				return *(uint32_t*)ptr;
			}
		}

		return 0;
	}

	/// <summary>
	/// https://github.com/emu-russia/breaknes/issues/119
	/// 
	/// Output when TempOAM is filled with some values to make sure that it is correctly filled with values from the main OAM.
	/// </summary>
	void Board::DebugPrintFilledTempOAM()
	{
		bool notFF = false;
		uint8_t temp_oam[32]{};
		for (size_t n = 0; n < 32; n++)
		{
			temp_oam[n] = DumpTempOAM(this, n);
			if (!(temp_oam[n] == 0xff || temp_oam[n] == 0x00))
			{
				notFF = true;
			}
		}
		if (notFF)
		{
			printf("TempOAM not 0xFF at H:%zd, V:%zd\n", GetHCounter(), GetVCounter());
			size_t n = 0;
			for (size_t row = 0; row < 2; row++)
			{
				for (size_t col = 0; col < 16; col++)
				{
					printf("%02X ", temp_oam[n++]);
				}
				printf("\n");
			}
		}
	}

	/// <summary>
	/// Place a test value in Sprite #0 to test the OAMEval simulation.
	/// </summary>
	void Board::DebugOAMFillTestPattern()
	{
		ppu->Dbg_OAMWriteByte(0, 0);
		ppu->Dbg_OAMWriteByte(1, 0xaa);
		ppu->Dbg_OAMWriteByte(2, 1);
		ppu->Dbg_OAMWriteByte(3, 0x55);
	}
}
