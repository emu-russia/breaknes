// Debugging mechanisms of the APU.

#pragma once

namespace APUSim
{
	struct APU_Interconnects
	{
		uint8_t n_CLK;
		uint8_t PHI0;
		uint8_t PHI1;
		uint8_t PHI2;
		uint8_t RDY;
		uint8_t ACLK;
		uint8_t n_ACLK;
		uint8_t RES;
		uint8_t n_M2;
		uint8_t n_NMI;
		uint8_t n_IRQ;
		uint8_t INT;
		uint8_t n_LFO1;
		uint8_t n_LFO2;
		uint8_t RnW;
		uint8_t SPR_CPU;
		uint8_t SPR_PPU;
		uint8_t RW;
		uint8_t RD;
		uint8_t WR;
		uint8_t n_DMC_AB;
		uint8_t RUNDMC;
		uint8_t DMCINT;
		uint8_t DMCRDY;
		uint8_t n_R4015;
		uint8_t n_R4016;
		uint8_t n_R4017;
		uint8_t n_R4018;
		uint8_t n_R4019;
		uint8_t n_R401A;
		uint8_t W4000;
		uint8_t W4001;
		uint8_t W4002;
		uint8_t W4003;
		uint8_t W4004;
		uint8_t W4005;
		uint8_t W4006;
		uint8_t W4007;
		uint8_t W4008;
		uint8_t W400A;
		uint8_t W400B;
		uint8_t W400C;
		uint8_t W400E;
		uint8_t W400F;
		uint8_t W4010;
		uint8_t W4011;
		uint8_t W4012;
		uint8_t W4013;
		uint8_t W4014;
		uint8_t W4015;
		uint8_t W4016;
		uint8_t W4017;
		uint8_t W401A;
		uint8_t SQA_LC;
		uint8_t SQB_LC;
		uint8_t TRI_LC;
		uint8_t RND_LC;
		uint8_t NOSQA;
		uint8_t NOSQB;
		uint8_t NOTRI;
		uint8_t NORND;
		uint8_t DBG;
		uint8_t n_DBGRD;
		uint8_t LOCK;
	};

	/// <summary>
	/// Various registers, buffers and counters inside the APU.
	/// </summary>
	struct APU_Registers
	{
		uint32_t Bogus;
	};
}
