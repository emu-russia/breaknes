// Debugging mechanisms of the APUSim.

#pragma once

namespace APUSim
{
#pragma pack(push, 1)
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
		uint32_t DBOutputLatch;		// 8
		uint32_t DBInputLatch;		// 8
		uint32_t OutReg;			// 3, latched
		uint32_t LengthCounterSQA;	// 8
		uint32_t LengthCounterSQB;	// 8
		uint32_t LengthCounterTRI;	// 8
		uint32_t LengthCounterRND;	// 8
		uint32_t SQFreqReg[2];		// 11
		uint32_t SQShiftReg[2];		// 3
		uint32_t SQFreqCounter[2];	// 11
		uint32_t SQVolumeReg[2];		// 4
		uint32_t SQDecayCounter[2];	// 4
		uint32_t SQEnvelopeCounter[2]; // 4
		uint32_t SQSweepReg[2];		// 3
		uint32_t SQSweepCounter[2];	// 3
		uint32_t SQDutyCounter[2];	// 3
		uint32_t TRILinearReg;		// 7
		uint32_t TRILinearCounter;	// 7
		uint32_t TRIFreqReg;		// 11
		uint32_t TRIFreqCounter;	// 11
		uint32_t TRIOutputCounter;	// 5
		uint32_t RNDFreqReg;		// 4
		uint32_t RNDVolumeReg;		// 4
		uint32_t RNDDecayCounter;	// 4
		uint32_t RNDEnvelopeCounter;	// 4
		uint32_t DPCMFreqReg;		// 4
		uint32_t DPCMSampleReg;		// 8
		uint32_t DPCMSampleCounter;	// 12
		uint32_t DPCMSampleBuffer;	// 8
		uint32_t DPCMSampleBitCounter;	// 3
		uint32_t DPCMAddressReg;	// 8
		uint32_t DPCMAddressCounter;	// 15
		uint32_t DPCMOutput;		// 7, counter + lsb
		uint32_t DMABuffer;			// 8
		uint32_t DMAAddress;		// 16, counter + register
	};
#pragma pack(pop)
}
