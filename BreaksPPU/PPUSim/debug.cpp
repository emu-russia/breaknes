// Obtaining a rich PPU inner world for debugging.

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	void PPU::GetDebugInfo_Wires(PPU_Interconnects& wires)
	{
		wires.RnW = wire.RnW == TriState::One ? 1 : 0;
		for (size_t n = 0; n < 3; n++)
		{
			wires.RS[n] = wire.RS[n] == TriState::One ? 1 : 0;
		}
		wires.n_DBE = wire.n_DBE == TriState::One ? 1 : 0;
		wires.n_ALE = wire.n_ALE == TriState::One ? 1 : 0;
		wires.CLK = wire.CLK == TriState::One ? 1 : 0;
		wires.n_CLK = wire.n_CLK == TriState::One ? 1 : 0;
		wires.RES = wire.RES == TriState::One ? 1 : 0;
		wires.RC = wire.RC == TriState::One ? 1 : 0;
		wires.PCLK = wire.PCLK == TriState::One ? 1 : 0;
		wires.n_PCLK = wire.n_PCLK == TriState::One ? 1 : 0;
		for (size_t n = 0; n < 4; n++)
		{
			wires.n_CC[n] = wire.n_CC[n] == TriState::One ? 1 : 0;
		}
		for (size_t n = 0; n < 2; n++)
		{
			wires.n_LL[n] = wire.n_LL[n] == TriState::One ? 1 : 0;
		}

		wires.n_RD = wire.n_RD == TriState::One ? 1 : 0;
		wires.n_WR = wire.n_WR == TriState::One ? 1 : 0;
		wires.n_W6_1 = wire.n_W6_1 == TriState::One ? 1 : 0;
		wires.n_W6_2 = wire.n_W6_2 == TriState::One ? 1 : 0;
		wires.n_W5_1 = wire.n_W5_1 == TriState::One ? 1 : 0;
		wires.n_W5_2 = wire.n_W5_2 == TriState::One ? 1 : 0;
		wires.n_R7 = wire.n_R7 == TriState::One ? 1 : 0;
		wires.n_W7 = wire.n_W7 == TriState::One ? 1 : 0;
		wires.n_W4 = wire.n_W4 == TriState::One ? 1 : 0;
		wires.n_W3 = wire.n_W3 == TriState::One ? 1 : 0;
		wires.n_R2 = wire.n_R2 == TriState::One ? 1 : 0;
		wires.n_W1 = wire.n_W1 == TriState::One ? 1 : 0;
		wires.n_W0 = wire.n_W0 == TriState::One ? 1 : 0;
		wires.n_R4 = wire.n_R4 == TriState::One ? 1 : 0;

		wires.I1_32 = wire.I1_32 == TriState::One ? 1 : 0;
		wires.OBSEL = wire.OBSEL == TriState::One ? 1 : 0;
		wires.BGSEL = wire.BGSEL == TriState::One ? 1 : 0;
		wires.O8_16 = wire.O8_16 == TriState::One ? 1 : 0;
		wires.n_SLAVE = wire.n_SLAVE == TriState::One ? 1 : 0;
		wires.VBL = wire.VBL == TriState::One ? 1 : 0;
		wires.BnW = wire.BnW == TriState::One ? 1 : 0;
		wires.n_BGCLIP = wire.n_BGCLIP == TriState::One ? 1 : 0;
		wires.n_OBCLIP = wire.n_OBCLIP == TriState::One ? 1 : 0;
		wires.BGE = wire.BGE == TriState::One ? 1 : 0;
		wires.BLACK = wire.BLACK == TriState::One ? 1 : 0;
		wires.OBE = wire.OBE == TriState::One ? 1 : 0;
		wires.n_TR = wire.n_TR == TriState::One ? 1 : 0;
		wires.n_TG = wire.n_TG == TriState::One ? 1 : 0;
		wires.n_TB = wire.n_TB == TriState::One ? 1 : 0;

		wires.H0_Dash = wire.H0_Dash == TriState::One ? 1 : 0;
		wires.H0_Dash2 = wire.H0_Dash2 == TriState::One ? 1 : 0;
		wires.nH1_Dash = wire.nH1_Dash == TriState::One ? 1 : 0;
		wires.H1_Dash2 = wire.H1_Dash2 == TriState::One ? 1 : 0;
		wires.nH2_Dash = wire.nH2_Dash == TriState::One ? 1 : 0;
		wires.H2_Dash2 = wire.H2_Dash2 == TriState::One ? 1 : 0;
		wires.H3_Dash2 = wire.H3_Dash2 == TriState::One ? 1 : 0;
		wires.H4_Dash2 = wire.H4_Dash2 == TriState::One ? 1 : 0;
		wires.H5_Dash2 = wire.H5_Dash2 == TriState::One ? 1 : 0;
		wires.EvenOddOut = wire.EvenOddOut == TriState::One ? 1 : 0;
		wires.HC = wire.HC == TriState::One ? 1 : 0;
		wires.VC = wire.VC == TriState::One ? 1 : 0;

		wires.n_ZCOL0 = wire.n_ZCOL0 == TriState::One ? 1 : 0;
		wires.n_ZCOL1 = wire.n_ZCOL1 == TriState::One ? 1 : 0;
		wires.ZCOL2 = wire.ZCOL2 == TriState::One ? 1 : 0;
		wires.ZCOL3 = wire.ZCOL3 == TriState::One ? 1 : 0;
		wires.n_ZPRIO = wire.n_ZPRIO == TriState::One ? 1 : 0;
		wires.n_SPR0HIT = wire.n_SPR0HIT == TriState::One ? 1 : 0;
		for (size_t n = 0; n < 4; n++)
		{
			wires.EXT_In[n] = wire.EXT_In[n] == TriState::One ? 1 : 0;
			wires.n_EXT_Out[n] = wire.n_EXT_Out[n] == TriState::One ? 1 : 0;
		}

		wires.OB = Pack (wire.OB);			// Packed
		wires.I2SEV = wire.I2SEV == TriState::One ? 1 : 0;
		wires.OFETCH = wire.OFETCH == TriState::One ? 1 : 0;
		wires.SPR_OV = wire.SPR_OV == TriState::One ? 1 : 0;
		wires.OAMCTR2 = wire.OAMCTR2 == TriState::One ? 1 : 0;
		wires.n_OAM = Pack(wire.n_OAM);		// Packed
		wires.OAM8 = wire.OAM8 == TriState::One ? 1 : 0;
		wires.Z_FIFO = wire.Z_FIFO == TriState::One ? 1 : 0;
		wires.OV = Pack(wire.OV);			// Packed

		wires.CLPB = wire.CLPB == TriState::One ? 1 : 0;
		wires.CLPO = wire.CLPO == TriState::One ? 1 : 0;
		wires.n_SH2 = wire.n_SH2 == TriState::One ? 1 : 0;
		wires.n_SH3 = wire.n_SH3 == TriState::One ? 1 : 0;
		wires.n_SH5 = wire.n_SH5 == TriState::One ? 1 : 0;
		wires.n_SH7 = wire.n_SH7 == TriState::One ? 1 : 0;

		wires.n_PA_Bot = Pack(wire.n_PA_Bot);
		for (size_t n = 0; n < 6; n++)
		{
			wires.n_PA_Top[n] = wire.n_PA_Top[n] == TriState::One ? 1 : 0;
		}
		for (size_t n = 0; n < 4; n++)
		{
			wires.BGC[n] = wire.BGC[n] == TriState::One ? 1 : 0;
		}
		for (size_t n = 0; n < 3; n++)
		{
			wires.FH[n] = wire.FH[n] == TriState::One ? 1 : 0;
		}
		for (size_t n = 0; n < 3; n++)
		{
			wires.FV[n] = wire.FV[n] == TriState::One ? 1 : 0;
		}
		wires.NTV = wire.NTV == TriState::One ? 1 : 0;
		wires.NTH = wire.NTH == TriState::One ? 1 : 0;
		for (size_t n = 0; n < 5; n++)
		{
			wires.TV[n] = wire.TV[n] == TriState::One ? 1 : 0;
		}
		for (size_t n = 0; n < 5; n++)
		{
			wires.TH[n] = wire.TH[n] == TriState::One ? 1 : 0;
		}
		for (size_t n = 0; n < 5; n++)
		{
			wires.THO[n] = wire.THO[n] == TriState::One ? 1 : 0;
		}
		for (size_t n = 0; n < 5; n++)
		{
			wires.TVO[n] = wire.TVO[n] == TriState::One ? 1 : 0;
		}
		for (size_t n = 0; n < 3; n++)
		{
			wires.FVO[n] = wire.FVO[n] == TriState::One ? 1 : 0;
		}
		for (size_t n = 0; n < 3; n++)
		{
			wires.n_FVO[n] = wire.n_FVO[n] == TriState::One ? 1 : 0;
		}
		wires.n_CB_DB = wire.n_CB_DB == TriState::One ? 1 : 0;
		wires.n_BW = wire.n_BW == TriState::One ? 1 : 0;
		wires.n_DB_CB = wire.n_DB_CB == TriState::One ? 1 : 0;
		for (size_t n = 0; n < 5; n++)
		{
			wires.PAL[n] = wire.PAL[n] == TriState::One ? 1 : 0;
		}

		wires.RD = wire.RD == TriState::One ? 1 : 0;
		wires.WR = wire.WR == TriState::One ? 1 : 0;
		wires.TSTEP = wire.TSTEP == TriState::One ? 1 : 0;
		wires.DB_PAR = wire.DB_PAR == TriState::One ? 1 : 0;
		wires.PD_RB = wire.PD_RB == TriState::One ? 1 : 0;
		wires.TH_MUX = wire.TH_MUX == TriState::One ? 1 : 0;
		wires.XRB = wire.XRB == TriState::One ? 1 : 0;

		wires.DB = DB;
		wires.PD = PD;
	}

	void PPU::GetDebugInfo_FSMStates(PPU_FSMStates& fsm_states)
	{
		fsm_states.S_EV = fsm.SEV == TriState::One ? 1 : 0;
		fsm_states.CLIP_O = fsm.CLIP_O == TriState::One ? 1 : 0;
		fsm_states.CLIP_B = fsm.CLIP_B == TriState::One ? 1 : 0;
		fsm_states.Z_HPOS = fsm.ZHPOS == TriState::One ? 1 : 0;
		fsm_states.EVAL = fsm.EVAL == TriState::One ? 1 : 0;
		fsm_states.E_EV = fsm.EEV == TriState::One ? 1 : 0;
		fsm_states.I_OAM2 = fsm.IOAM2 == TriState::One ? 1 : 0;
		fsm_states.PAR_O = fsm.PARO == TriState::One ? 1 : 0;
		fsm_states.n_VIS = fsm.nVIS == TriState::One ? 1 : 0;
		fsm_states.n_F_NT = fsm.nFNT == TriState::One ? 1 : 0;
		fsm_states.F_TB = fsm.FTB == TriState::One ? 1 : 0;
		fsm_states.F_TA = fsm.FTA == TriState::One ? 1 : 0;
		fsm_states.F_AT = fsm.FAT == TriState::One ? 1 : 0;
		fsm_states.n_FO = fsm.nFO == TriState::One ? 1 : 0;
		fsm_states.BPORCH = fsm.BPORCH == TriState::One ? 1 : 0;
		fsm_states.SC_CNT = fsm.SCCNT == TriState::One ? 1 : 0;
		fsm_states.n_HB = fsm.nHB == TriState::One ? 1 : 0;
		fsm_states.BURST = fsm.BURST == TriState::One ? 1 : 0;
		fsm_states.HSYNC = fsm.HSYNC == TriState::One ? 1 : 0;
		fsm_states.n_PICTURE = fsm.n_PICTURE == TriState::One ? 1 : 0;
		fsm_states.RESCL = fsm.RESCL == TriState::One ? 1 : 0;
		fsm_states.VSYNC = fsm.VSYNC == TriState::One ? 1 : 0;
		fsm_states.n_VSET = fsm.nVSET == TriState::One ? 1 : 0;
		fsm_states.VB = fsm.VB == TriState::One ? 1 : 0;
		fsm_states.BLNK = fsm.BLNK == TriState::One ? 1 : 0;
		fsm_states.INT = fsm.INT == TriState::One ? 1 : 0;
	}

	void PPU::GetDebugInfo_Regs(PPU_Registers& regs)
	{
		regs.HCounter = (uint32_t)h->get();
		regs.VCounter = (uint32_t)v->get();
		regs.CTRL0 = this->regs->Debug_GetCTRL0();		// $2000
		regs.CTRL1 = this->regs->Debug_GetCTRL1();	// $2001
		regs.MainOAMCounter = eval->Debug_GetMainCounter();		// $2003
		regs.TempOAMCounter = eval->Debug_GetTempCounter();
		regs.OB = Pack(wire.OB);		// $2004
		regs.RB = vram_ctrl->Debug_GetRB();			// $2007
		regs.SCC_FH = Pack3(wire.FH);		// $2005/2006
		regs.SCC_FV = Pack3(wire.FV);
		regs.SCC_NTV = wire.NTV;
		regs.SCC_NTH = wire.NTH;
		regs.SCC_TV = Pack5(wire.TV);
		regs.SCC_TH = Pack5(wire.TH);
	}

	uint8_t PPU::Dbg_OAMReadByte(size_t addr)
	{
		return oam->Dbg_OAMReadByte(addr);
	}

	uint8_t PPU::Dbg_TempOAMReadByte(size_t addr)
	{
		return oam->Dbg_TempOAMReadByte(addr);
	}

	uint8_t PPU::Dbg_CRAMReadByte(size_t addr)
	{
		return cram->Dbg_CRAMReadByte(addr);
	}

	uint8_t PPU::Dbg_GetCRAMAddress()
	{
		uint8_t addr = 0;
		for (size_t n = 0; n < 5; n++)
		{
			addr |= (wire.PAL[n] == TriState::One ? 1 : 0) << n;
		}
		return addr;
	}

	uint16_t PPU::Dbg_GetPPUAddress()
	{
		uint8_t PABot = 0;
		for (size_t n = 0; n < 8; n++)
		{
			PABot |= (wire.n_PA_Bot[n] == TriState::Zero ? 1 : 0) << n;
		}

		uint8_t PATop = 0;
		for (size_t n = 0; n < 6; n++)
		{
			PATop |= (wire.n_PA_Top[n] == TriState::Zero ? 1 : 0) << n;
		}

		return ((uint16_t)PATop << 8) | PABot;
	}

	void PPU::Dbg_RandomizePicture(bool enable)
	{
		vid_out->Dbg_RandomizePicture(enable);
	}

	void PPU::Dbg_FixedPicture(bool enable)
	{
		vid_out->Dbg_FixedPicture(enable);
	}

	void PPU::Dbg_RenderAlwaysEnabled(bool enable)
	{
		regs->Debug_RenderAlwaysEnabled(enable);
	}
}
