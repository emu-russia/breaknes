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
		uint8_t RD;
		uint8_t WR;
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
		uint8_t I2SEV;
		uint8_t OFETCH;
		uint8_t SPR_OV;
		uint8_t OAMCTR2;
		uint8_t n_OAM;		// Packed
		uint8_t OAM8;
		uint8_t Z_FIFO;
		uint8_t OV;			// Packed
		uint8_t CLPB;
		uint8_t CLPO;
		uint8_t n_PA_Top[6];
		uint8_t BGC[4];
		uint8_t THO[5];
		uint8_t n_CB_DB;
		uint8_t n_BW;
		uint8_t n_DB_CB;
		uint8_t PAL[5];
		uint8_t TSTEP;
		uint8_t DB_PAR;
		uint8_t PD_RB;
		uint8_t TH_MUX;
		uint8_t XRB;
		uint8_t DB;
		size_t HCounter;
		size_t VCounter;
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
		uint8_t F_NT;
		uint8_t F_TB;
		uint8_t F_TA;
		uint8_t F_AT;
		uint8_t n_FO;
		uint8_t BPORCH;
		uint8_t SC_CNT;
		uint8_t n_HB;
		uint8_t BURST;
		uint8_t HSYNC;
		uint8_t PICTURE;
		uint8_t RESCL;
		uint8_t VSYNC;
		uint8_t n_VSET;
		uint8_t VB;
		uint8_t BLNK;
		uint8_t INT;
	};
}
