// Debugging mechanisms of 6502.

#pragma once

namespace M6502Core
{
#pragma pack(push, 1)
	struct DebugInfo
	{
		// Regs & Buses

		uint8_t SB;
		uint8_t DB;
		uint8_t ADL;
		uint8_t ADH;

		uint8_t IR;
		uint8_t PD;
		uint8_t Y;
		uint8_t X;
		uint8_t S;
		uint8_t AI;
		uint8_t BI;
		uint8_t ADD;
		uint8_t AC;
		uint8_t PCL;
		uint8_t PCH;
		uint8_t PCLS;
		uint8_t PCHS;
		uint8_t ABL;
		uint8_t ABH;
		uint8_t DL;
		uint8_t DOR;

		uint8_t C_OUT;
		uint8_t Z_OUT;
		uint8_t I_OUT;
		uint8_t D_OUT;
		uint8_t B_OUT;
		uint8_t V_OUT;
		uint8_t N_OUT;

		// Internals

		uint8_t n_PRDY;
		uint8_t n_NMIP;
		uint8_t n_IRQP;
		uint8_t RESP;
		uint8_t BRK6E;
		uint8_t BRK7;
		uint8_t DORES;
		uint8_t n_DONMI;
		uint8_t n_T2;
		uint8_t n_T3;
		uint8_t n_T4;
		uint8_t n_T5;
		uint8_t T0;
		uint8_t n_T0;
		uint8_t n_T1X;
		uint8_t Z_IR;
		uint8_t FETCH;
		uint8_t n_ready;
		uint8_t WR;
		uint8_t TRES2;
		uint8_t ACRL1;
		uint8_t ACRL2;
		uint8_t T1;
		uint8_t RMW_T6;
		uint8_t RMW_T7;
		uint8_t ENDS;
		uint8_t ENDX;
		uint8_t TRES1;
		uint8_t n_TRESX;
		uint8_t BRFW;
		uint8_t n_BRTAKEN;
		uint8_t ACR;
		uint8_t AVR;

		// Decoder

		uint8_t decoder_out[Decoder::outputs_count];

		// Control commands

		uint8_t Y_SB;
		uint8_t SB_Y;
		uint8_t X_SB;
		uint8_t SB_X;
		uint8_t S_ADL;
		uint8_t S_SB;
		uint8_t SB_S;
		uint8_t S_S;
		uint8_t NDB_ADD;
		uint8_t DB_ADD;
		uint8_t Z_ADD;
		uint8_t SB_ADD;
		uint8_t ADL_ADD;
		uint8_t ANDS;
		uint8_t EORS;
		uint8_t ORS;
		uint8_t SRS;
		uint8_t SUMS;
		uint8_t ADD_SB7;
		uint8_t ADD_SB06;
		uint8_t ADD_ADL;
		uint8_t SB_AC;
		uint8_t AC_SB;
		uint8_t AC_DB;
		uint8_t ADH_PCH;
		uint8_t PCH_PCH;
		uint8_t PCH_ADH;
		uint8_t PCH_DB;
		uint8_t ADL_PCL;
		uint8_t PCL_PCL;
		uint8_t PCL_ADL;
		uint8_t PCL_DB;
		uint8_t ADH_ABH;
		uint8_t ADL_ABL;
		uint8_t Z_ADL0;
		uint8_t Z_ADL1;
		uint8_t Z_ADL2;
		uint8_t Z_ADH0;
		uint8_t Z_ADH17;
		uint8_t SB_DB;
		uint8_t SB_ADH;
		uint8_t DL_ADL;
		uint8_t DL_ADH;
		uint8_t DL_DB;

		uint8_t P_DB;
		uint8_t DB_P;
		uint8_t DBZ_Z;
		uint8_t DB_N;
		uint8_t IR5_C;
		uint8_t DB_C;
		uint8_t ACR_C;
		uint8_t IR5_D;
		uint8_t IR5_I;
		uint8_t DB_V;
		uint8_t AVR_V;
		uint8_t Z_V;

		uint8_t n_ACIN;
		uint8_t n_DAA;
		uint8_t n_DSA;
		uint8_t n_1PC;			// From Dispatcher
	};

	struct UserRegs
	{
		uint8_t A;
		uint8_t X;
		uint8_t Y;
		uint8_t S;
		uint8_t C_OUT;
		uint8_t Z_OUT;
		uint8_t I_OUT;
		uint8_t D_OUT;
		uint8_t V_OUT;
		uint8_t N_OUT;
		uint8_t PCH;
		uint8_t PCL;
		uint8_t PCHS;
		uint8_t PCLS;
	};
#pragma pack(pop)
}
