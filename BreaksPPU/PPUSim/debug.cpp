// Obtaining a rich PPU inner world for debugging.

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	void PPU::GetDebugInfo_Wires(PPU_Interconnects& wires)
	{
		wires.CLK = ToByte(wire.CLK);
		wires.n_CLK = ToByte(wire.n_CLK);
		wires.RES = ToByte(wire.RES);
		wires.RC = ToByte(wire.RC);
		wires.PCLK = ToByte(wire.PCLK);
		wires.n_PCLK = ToByte(wire.n_PCLK);

		wires.RnW = ToByte(wire.RnW);
		for (size_t n = 0; n < 3; n++)
		{
			wires.RS[n] = ToByte(wire.RS[n]);
		}
		wires.n_DBE = ToByte(wire.n_DBE);
		wires.n_RD = ToByte(wire.n_RD);
		wires.n_WR = ToByte(wire.n_WR);
		wires.n_W6_1 = ToByte(wire.n_W6_1);
		wires.n_W6_2 = ToByte(wire.n_W6_2);
		wires.n_W5_1 = ToByte(wire.n_W5_1);
		wires.n_W5_2 = ToByte(wire.n_W5_2);
		wires.n_R7 = ToByte(wire.n_R7);
		wires.n_W7 = ToByte(wire.n_W7);
		wires.n_W4 = ToByte(wire.n_W4);
		wires.n_W3 = ToByte(wire.n_W3);
		wires.n_R2 = ToByte(wire.n_R2);
		wires.n_W1 = ToByte(wire.n_W1);
		wires.n_W0 = ToByte(wire.n_W0);
		wires.n_R4 = ToByte(wire.n_R4);

		wires.I1_32 = ToByte(wire.I1_32);
		wires.OBSEL = ToByte(wire.OBSEL);
		wires.BGSEL = ToByte(wire.BGSEL);
		wires.O8_16 = ToByte(wire.O8_16);
		wires.n_SLAVE = ToByte(wire.n_SLAVE);
		wires.VBL = ToByte(wire.VBL);
		wires.BnW = ToByte(wire.BnW);
		wires.n_BGCLIP = ToByte(wire.n_BGCLIP);
		wires.n_OBCLIP = ToByte(wire.n_OBCLIP);
		wires.BGE = ToByte(wire.BGE);
		wires.BLACK = ToByte(wire.BLACK);
		wires.OBE = ToByte(wire.OBE);
		wires.n_TR = ToByte(wire.n_TR);
		wires.n_TG = ToByte(wire.n_TG);
		wires.n_TB = ToByte(wire.n_TB);

		wires.H0_Dash = ToByte(wire.H0_Dash);
		wires.H0_Dash2 = ToByte(wire.H0_Dash2);
		wires.nH1_Dash = ToByte(wire.nH1_Dash);
		wires.H1_Dash2 = ToByte(wire.H1_Dash2);
		wires.nH2_Dash = ToByte(wire.nH2_Dash);
		wires.H2_Dash2 = ToByte(wire.H2_Dash2);
		wires.H3_Dash2 = ToByte(wire.H3_Dash2);
		wires.H4_Dash2 = ToByte(wire.H4_Dash2);
		wires.H5_Dash2 = ToByte(wire.H5_Dash2);
		wires.EvenOddOut = ToByte(wire.EvenOddOut);
		wires.HC = ToByte(wire.HC);
		wires.VC = ToByte(wire.VC);

		wires.n_ZCOL0 = ToByte(wire.n_ZCOL0);
		wires.n_ZCOL1 = ToByte(wire.n_ZCOL1);
		wires.ZCOL2 = ToByte(wire.ZCOL2);
		wires.ZCOL3 = ToByte(wire.ZCOL3);
		wires.n_ZPRIO = ToByte(wire.n_ZPRIO);
		wires.n_SPR0HIT = ToByte(wire.n_SPR0HIT);
		for (size_t n = 0; n < 4; n++)
		{
			wires.EXT_In[n] = ToByte(wire.EXT_In[n]);
			wires.n_EXT_Out[n] = ToByte(wire.n_EXT_Out[n]);
		}

		wires.OB = Pack (wire.OB);			// Packed
		wires.n_SPR0_EV = ToByte(wire.n_SPR0_EV);
		wires.OFETCH = ToByte(wire.OFETCH);
		wires.SPR_OV = ToByte(wire.SPR_OV);
		wires.OAMCTR2 = ToByte(wire.OAMCTR2);
		wires.n_OAM = Pack(wire.n_OAM);		// Packed
		wires.OAM8 = ToByte(wire.OAM8);
		wires.PD_FIFO = ToByte(wire.PD_FIFO);
		wires.OV = Pack(wire.OV);			// Packed
		wires.n_WE = ToByte(wire.n_WE);

		wires.n_CLPB = ToByte(wire.n_CLPB);
		wires.CLPO = ToByte(wire.CLPO);
		wires.n_SH2 = ToByte(wire.n_SH2);
		wires.n_SH3 = ToByte(wire.n_SH3);
		wires.n_SH5 = ToByte(wire.n_SH5);
		wires.n_SH7 = ToByte(wire.n_SH7);

		wires.n_PA_Bot = Pack(wire.n_PA_Bot);
		for (size_t n = 0; n < 6; n++)
		{
			wires.n_PA_Top[n] = ToByte(wire.n_PA_Top[n]);
		}
		for (size_t n = 0; n < 4; n++)
		{
			wires.BGC[n] = ToByte(wire.BGC[n]);
		}
		for (size_t n = 0; n < 3; n++)
		{
			wires.FH[n] = ToByte(wire.FH[n]);
		}
		for (size_t n = 0; n < 3; n++)
		{
			wires.FV[n] = ToByte(wire.FV[n]);
		}
		wires.NTV = ToByte(wire.NTV);
		wires.NTH = ToByte(wire.NTH);
		for (size_t n = 0; n < 5; n++)
		{
			wires.TV[n] = ToByte(wire.TV[n]);
		}
		for (size_t n = 0; n < 5; n++)
		{
			wires.TH[n] = ToByte(wire.TH[n]);
		}
		for (size_t n = 0; n < 5; n++)
		{
			wires.THO[n] = ToByte(wire.THO[n]);
		}
		for (size_t n = 0; n < 5; n++)
		{
			wires.TVO[n] = ToByte(wire.TVO[n]);
		}
		for (size_t n = 0; n < 3; n++)
		{
			wires.FVO[n] = ToByte(wire.FVO[n]);
		}
		for (size_t n = 0; n < 3; n++)
		{
			wires.n_FVO[n] = ToByte(wire.n_FVO[n]);
		}

		wires.n_CB_DB = ToByte(wire.n_CB_DB);
		wires.n_BW = ToByte(wire.n_BW);
		wires.n_DB_CB = ToByte(wire.n_DB_CB);
		for (size_t n = 0; n < 5; n++)
		{
			wires.PAL[n] = ToByte(wire.PAL[n]);
		}
		for (size_t n = 0; n < 4; n++)
		{
			wires.n_CC[n] = ToByte(wire.n_CC[n]);
		}
		for (size_t n = 0; n < 2; n++)
		{
			wires.n_LL[n] = ToByte(wire.n_LL[n]);
		}

		wires.RD = ToByte(wire.RD);
		wires.WR = ToByte(wire.WR);
		wires.n_ALE = ToByte(wire.n_ALE);
		wires.TSTEP = ToByte(wire.TSTEP);
		wires.DB_PAR = ToByte(wire.DB_PAR);
		wires.PD_RB = ToByte(wire.PD_RB);
		wires.TH_MUX = ToByte(wire.TH_MUX);
		wires.XRB = ToByte(wire.XRB);

		wires.DB = DB;
		wires.PD = PD;
	}

	void PPU::GetDebugInfo_FSMStates(PPU_FSMStates& fsm_states)
	{
		fsm_states.S_EV = ToByte(fsm.SEV);
		fsm_states.CLIP_O = ToByte(fsm.CLIP_O);
		fsm_states.CLIP_B = ToByte(fsm.CLIP_B);
		fsm_states.Z_HPOS = ToByte(fsm.ZHPOS);
		fsm_states.n_EVAL = ToByte(fsm.n_EVAL);
		fsm_states.E_EV = ToByte(fsm.EEV);
		fsm_states.I_OAM2 = ToByte(fsm.IOAM2);
		fsm_states.PAR_O = ToByte(fsm.PARO);
		fsm_states.n_VIS = ToByte(fsm.nVIS);
		fsm_states.F_NT = ToByte(NOT(fsm.nFNT));	// Direct polarity returns for unification
		fsm_states.F_TB = ToByte(fsm.FTB);
		fsm_states.F_TA = ToByte(fsm.FTA);
		fsm_states.F_AT = ToByte(fsm.FAT);
		fsm_states.n_FO = ToByte(fsm.nFO);
		fsm_states.BPORCH = ToByte(fsm.BPORCH);
		fsm_states.SC_CNT = ToByte(fsm.SCCNT);
		fsm_states.BURST = ToByte(fsm.BURST);
		fsm_states.SYNC = ToByte(fsm.SYNC);
		fsm_states.n_PICTURE = ToByte(fsm.n_PICTURE);
		fsm_states.RESCL = ToByte(fsm.RESCL);
		fsm_states.VSYNC = ToByte(fsm.VSYNC);
		fsm_states.VB = ToByte(fsm.VB);
		fsm_states.BLNK = ToByte(fsm.BLNK);
		fsm_states.INT = ToByte(fsm.INT);
	}

	void PPU::GetDebugInfo_Regs(PPU_Registers& regs)
	{
		regs.HCounter = (uint32_t)h->get();
		regs.VCounter = (uint32_t)v->get();
		regs.CTRL0 = this->regs->Debug_GetCTRL0();		// $2000
		regs.CTRL1 = this->regs->Debug_GetCTRL1();	// $2001
		regs.MainOAMCounter = eval->Debug_GetMainCounter();		// $2003
		regs.TempOAMCounter = eval->Debug_GetTempCounter();

		TriState ob[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			ob[n] = oam->get_OB(n);
		}
		regs.OAMBuffer = Pack(ob);		// $2004

		regs.ReadBuffer = vram_ctrl->Debug_GetRB();			// $2007
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

	void PPU::Dbg_OAMWriteByte(size_t addr, uint8_t val)
	{
		oam->Dbg_OAMWriteByte(addr, val);
	}

	void PPU::Dbg_TempOAMWriteByte(size_t addr, uint8_t val)
	{
		oam->Dbg_TempOAMWriteByte(addr, val);
	}

	uint8_t PPU::Dbg_CRAMReadByte(size_t addr)
	{
		return cram->Dbg_CRAMReadByte(addr);
	}

	void PPU::Dbg_CRAMWriteByte(size_t addr, uint8_t val)
	{
		cram->Dbg_CRAMWriteByte(addr, val);
	}

	uint8_t PPU::Dbg_GetCRAMAddress()
	{
		uint8_t addr = 0;
		for (size_t n = 0; n < 5; n++)
		{
			addr |= ToByte(wire.PAL[n]) << n;
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

	void PPU::GetDebugInfo_OAMEval(OAMEvalWires& wires)
	{
		eval->GetDebugInfo(wires);
	}
}
