// Debugging mechanisms of 6502.

#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	void M6502::getDebug(DebugInfo* info)
	{
		TriState BRK6E = wire.BRK6E;

		info->SB = SB;
		info->DB = DB;
		info->ADL = ADL;
		info->ADH = ADH;

		info->IR = ir->IROut;
		info->PD = predecode->PD;
		info->Y = regs->getY();
		info->X = regs->getX();
		info->S = regs->getS();
		info->AI = alu->getAI();
		info->BI = alu->getBI();
		info->ADD = alu->getADD();
		info->AC = alu->getAC();
		info->PCL = pc->getPCL();
		info->PCH = pc->getPCH();
		info->PCLS = pc->getPCLS();
		info->PCHS = pc->getPCHS();
		info->ABL = addr_bus->getABL();
		info->ABH = addr_bus->getABH();
		info->DL = data_bus->getDL();
		info->DOR = data_bus->getDOR();

		info->C_OUT = ToByte(NOT(random->flags->getn_C_OUT()));
		info->Z_OUT = ToByte(NOT(random->flags->getn_Z_OUT()));
		info->I_OUT = ToByte(NOT(random->flags->getn_I_OUT(BRK6E)));
		info->D_OUT = ToByte(NOT(random->flags->getn_D_OUT()));
		info->B_OUT = ToByte(brk->getB_OUT(BRK6E));
		info->V_OUT = ToByte(NOT(random->flags->getn_V_OUT()));
		info->N_OUT = ToByte(NOT(random->flags->getn_N_OUT()));

		info->n_PRDY = ToByte(prdy_latch2.nget());
		info->n_NMIP = ToByte(NOT(nmip_ff.get()));
		info->n_IRQP = ToByte(irqp_latch.nget());
		info->RESP = ToByte(resp_latch.nget());
		info->BRK6E = ToByte(BRK6E);
		info->BRK7 = ToByte(wire.BRK7);
		info->DORES = ToByte(wire.DORES);
		info->n_DONMI = ToByte(wire.n_DONMI);
		info->n_T2 = ToByte(wire.n_T2);
		info->n_T3 = ToByte(wire.n_T3);
		info->n_T4 = ToByte(wire.n_T4);
		info->n_T5 = ToByte(wire.n_T5);
		info->T0 = ToByte(wire.T0);
		info->n_T0 = ToByte(wire.n_T0);
		info->n_T1X = ToByte(wire.n_T1X);
		info->Z_IR = ToByte(wire.Z_IR);
		info->FETCH = ToByte(wire.FETCH);
		info->n_ready = ToByte(wire.n_ready);
		info->WR = ToByte(wire.WR);
		info->TRES2 = ToByte(disp->getTRES2());
		info->ACRL1 = ToByte(wire.ACRL1);
		info->ACRL2 = ToByte(wire.ACRL2);
		info->T1 = ToByte(disp->getT1());
		info->RMW_T6 = ToByte(wire.RMW_T6);
		info->RMW_T7 = ToByte(wire.RMW_T7);
		info->ENDS = ToByte(wire.ENDS);
		info->ENDX = ToByte(wire.ENDX);
		info->TRES1 = ToByte(wire.TRES1);
		info->n_TRESX = ToByte(wire.n_TRESX);
		info->BRFW = ToByte(wire.BRFW);
		info->n_BRTAKEN = ToByte(wire.n_BRTAKEN);
		info->ACR = ToByte(alu->getACR());
		info->AVR = ToByte(alu->getAVR());

		for (size_t n = 0; n < Decoder::outputs_count; n++)
		{
			info->decoder_out[n] = ToByte(decoder_out[n]);
		}

		info->Y_SB = cmd.Y_SB;
		info->SB_Y = cmd.SB_Y;
		info->X_SB = cmd.X_SB;
		info->SB_X = cmd.SB_X;
		info->S_ADL = cmd.S_ADL;
		info->S_SB = cmd.S_SB;
		info->SB_S = cmd.SB_S;
		info->S_S = cmd.S_S;
		info->NDB_ADD = cmd.NDB_ADD;
		info->DB_ADD = cmd.DB_ADD;
		info->Z_ADD = cmd.Z_ADD;
		info->SB_ADD = cmd.SB_ADD;
		info->ADL_ADD = cmd.ADL_ADD;
		info->n_ACIN = cmd.n_ACIN;
		info->ANDS = cmd.ANDS;
		info->EORS = cmd.EORS;
		info->ORS = cmd.ORS;
		info->SRS = cmd.SRS;
		info->SUMS = cmd.SUMS;
		info->n_DAA = cmd.n_DAA;
		info->n_DSA = cmd.n_DSA;
		info->ADD_SB7 = cmd.ADD_SB7;
		info->ADD_SB06 = cmd.ADD_SB06;
		info->ADD_ADL = cmd.ADD_ADL;
		info->SB_AC = cmd.SB_AC;
		info->AC_SB = cmd.AC_SB;
		info->AC_DB = cmd.AC_DB;
		info->n_1PC = ToByte(wire.n_1PC);			// From Dispatcher
		info->ADH_PCH = cmd.ADH_PCH;
		info->PCH_PCH = cmd.PCH_PCH;
		info->PCH_ADH = cmd.PCH_ADH;
		info->PCH_DB = cmd.PCH_DB;
		info->ADL_PCL = cmd.ADL_PCL;
		info->PCL_PCL = cmd.PCL_PCL;
		info->PCL_ADL = cmd.PCL_ADL;
		info->PCL_DB = cmd.PCL_DB;
		info->ADH_ABH = cmd.ADH_ABH;
		info->ADL_ABL = cmd.ADL_ABL;
		info->Z_ADL0 = cmd.Z_ADL0;
		info->Z_ADL1 = cmd.Z_ADL1;
		info->Z_ADL2 = cmd.Z_ADL2;
		info->Z_ADH0 = cmd.Z_ADH0;
		info->Z_ADH17 = cmd.Z_ADH17;
		info->SB_DB = cmd.SB_DB;
		info->SB_ADH = cmd.SB_ADH;
		info->DL_ADL = cmd.DL_ADL;
		info->DL_ADH = cmd.DL_ADH;
		info->DL_DB = cmd.DL_DB;

		info->P_DB = cmd.P_DB;
		info->DB_P = cmd.DB_P;
		info->DBZ_Z = cmd.DBZ_Z;
		info->DB_N = cmd.DB_N;
		info->IR5_C = cmd.IR5_C;
		info->DB_C = cmd.DB_C;
		info->ACR_C = cmd.ACR_C;
		info->IR5_D = cmd.IR5_D;
		info->IR5_I = cmd.IR5_I;
		info->DB_V = cmd.DB_V;
		info->AVR_V = cmd.AVR_V;
		info->Z_V = cmd.Z_V;
	}

	void M6502::getUserRegs(UserRegs* userRegs)
	{
		userRegs->Y = regs->getY();
		userRegs->X = regs->getX();
		userRegs->S = regs->getS();
		userRegs->A = alu->getAC();
		userRegs->PCL = pc->getPCL();
		userRegs->PCH = pc->getPCH();

		userRegs->C_OUT = ToByte(NOT(random->flags->getn_C_OUT()));
		userRegs->Z_OUT = ToByte(NOT(random->flags->getn_Z_OUT()));
		userRegs->I_OUT = ToByte(NOT(random->flags->getn_I_OUT(wire.BRK6E)));
		userRegs->D_OUT = ToByte(NOT(random->flags->getn_D_OUT()));
		userRegs->V_OUT = ToByte(NOT(random->flags->getn_V_OUT()));
		userRegs->N_OUT = ToByte(NOT(random->flags->getn_N_OUT()));
	}
}
