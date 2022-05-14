// Obtaining a rich PPU inner world for debugging.

#pragma once

namespace PPUSim
{
	struct PPU_Interconnects
	{
		uint8_t RnW;
		uint8_t RS[3];
		uint8_t n_DBE;
		uint8_t n_ALE;
		uint8_t CLK;
		uint8_t n_CLK;
		uint8_t RES;
		uint8_t RC;
		uint8_t PCLK;
		uint8_t n_PCLK;
		uint8_t n_CC[4];
		uint8_t n_LL[2];
		uint8_t n_RD;
		uint8_t n_WR;
		uint8_t n_W6_1;
		uint8_t n_W6_2;
		uint8_t n_W5_1;
		uint8_t n_W5_2;
		uint8_t n_R7;
		uint8_t n_W7;
		uint8_t n_W4;
		uint8_t n_W3;
		uint8_t n_R2;
		uint8_t n_W1;
		uint8_t n_W0;
		uint8_t n_R4;
		uint8_t I1_32;
		uint8_t OBSEL;
		uint8_t BGSEL;
		uint8_t O8_16;
		uint8_t n_SLAVE;
		uint8_t VBL;
		uint8_t BnW;
		uint8_t n_BGCLIP;
		uint8_t n_OBCLIP;
		uint8_t BGE;
		uint8_t BLACK;
		uint8_t OBE;
		uint8_t n_TR;
		uint8_t n_TG;
		uint8_t n_TB;
		uint8_t H0_Dash;
		uint8_t H0_Dash2;
		uint8_t nH1_Dash;
		uint8_t H1_Dash2;
		uint8_t nH2_Dash;
		uint8_t H2_Dash2;
		uint8_t H3_Dash2;
		uint8_t H4_Dash2;
		uint8_t H5_Dash2;
		uint8_t EvenOddOut;
		uint8_t HC;
		uint8_t VC;
		uint8_t n_ZCOL0;
		uint8_t n_ZCOL1;
		uint8_t ZCOL2;
		uint8_t ZCOL3;
		uint8_t n_ZPRIO;
		uint8_t n_SPR0HIT;
		uint8_t EXT_In[4];
		uint8_t n_EXT_Out[4];
		uint8_t OB;			// Packed
		uint8_t n_SPR0_EV;
		uint8_t OFETCH;
		uint8_t SPR_OV;
		uint8_t OAMCTR2;
		uint8_t n_OAM;		// Packed
		uint8_t OAM8;
		uint8_t PD_FIFO;
		uint8_t OV;			// Packed
		uint8_t n_WE;
		uint8_t CLPB;
		uint8_t CLPO;
		uint8_t n_SH2;
		uint8_t n_SH3;
		uint8_t n_SH5;
		uint8_t n_SH7;
		uint8_t n_PA_Bot;	// Packed
		uint8_t n_PA_Top[6];
		uint8_t BGC[4];
		uint8_t FH[3];
		uint8_t FV[3];
		uint8_t NTV;
		uint8_t NTH;
		uint8_t TV[5];
		uint8_t TH[5];
		uint8_t THO[5];
		uint8_t TVO[5];
		uint8_t FVO[3];
		uint8_t n_FVO[3];
		uint8_t n_CB_DB;
		uint8_t n_BW;
		uint8_t n_DB_CB;
		uint8_t PAL[5];
		uint8_t RD;
		uint8_t WR;
		uint8_t TSTEP;
		uint8_t DB_PAR;
		uint8_t PD_RB;
		uint8_t TH_MUX;
		uint8_t XRB;
		uint8_t DB;
		uint8_t PD;
	};

	struct PPU_FSMStates
	{
		uint8_t S_EV;
		uint8_t CLIP_O;
		uint8_t CLIP_B;
		uint8_t Z_HPOS;
		uint8_t EVAL;
		uint8_t E_EV;
		uint8_t I_OAM2;
		uint8_t PAR_O;
		uint8_t n_VIS;
		uint8_t n_F_NT;
		uint8_t F_TB;
		uint8_t F_TA;
		uint8_t F_AT;
		uint8_t n_FO;
		uint8_t BPORCH;
		uint8_t SC_CNT;
		uint8_t n_HB;
		uint8_t BURST;
		uint8_t HSYNC;
		uint8_t n_PICTURE;
		uint8_t RESCL;
		uint8_t VSYNC;
		uint8_t n_VSET;
		uint8_t VB;
		uint8_t BLNK;
		uint8_t INT;
	};

	/// <summary>
	/// These signals sit deep in the depths of the OAM Eval logic.
	/// </summary>
	struct OAMEvalWires
	{
		uint8_t OMFG;		// 1: Use Mode4 for MainCounter
		uint8_t OMSTEP;		// 1: +1/+4 MainCounter
		uint8_t OMOUT;		// 1: Keep MainCounter
		uint8_t OMV;		// MainCounter overflow
		uint8_t OSTEP;		// 1: +1 TempCounter
		uint8_t ORES;		// 1: Reset TempCounter
		uint8_t TMV;		// TempCounter overflow
		uint8_t OAP;		// 1: OAMAddress is MainCounter, 0: OAMAddress is TempCounter + bits0-2 of MainCounter
		uint8_t COPY_STEP;	// 1: Propagate copy Jhonson Counter
		uint8_t DO_COPY;	// 1: Do copy sprite
		uint8_t COPY_OVF;	// 1: Block OVZ comparator
		uint8_t OVZ;		// 1: Sprite found
		uint8_t OBCmpr;		// 1: OB inputs for comparator
	};

	/// <summary>
	/// Various registers, buffers and counters inside the PPU.
	/// </summary>
	struct PPU_Registers
	{
		uint32_t HCounter;
		uint32_t VCounter;
		uint32_t CTRL0;
		uint32_t CTRL1;
		uint32_t MainOAMCounter;
		uint32_t TempOAMCounter;
		uint32_t OB;
		uint32_t RB;
		uint32_t SCC_FH;
		uint32_t SCC_FV;
		uint32_t SCC_NTV;
		uint32_t SCC_NTH;
		uint32_t SCC_TV;
		uint32_t SCC_TH;
	};
}
