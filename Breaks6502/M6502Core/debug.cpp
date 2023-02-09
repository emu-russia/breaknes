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

	uint8_t M6502::getDebugSingle(int ofs)
	{
		TriState BRK6E = wire.BRK6E;

		switch (ofs)
		{
			case offsetof(DebugInfo, SB): return SB;
			case offsetof(DebugInfo, DB): return DB;
			case offsetof(DebugInfo, ADL): return ADL;
			case offsetof(DebugInfo, ADH): return ADH;

			case offsetof(DebugInfo, IR): return ir->IROut;
			case offsetof(DebugInfo, PD): return predecode->PD;
			case offsetof(DebugInfo, Y): return regs->getY();
			case offsetof(DebugInfo, X): return regs->getX();
			case offsetof(DebugInfo, S): return regs->getS();
			case offsetof(DebugInfo, AI): return alu->getAI();
			case offsetof(DebugInfo, BI): return alu->getBI();
			case offsetof(DebugInfo, ADD): return alu->getADD();
			case offsetof(DebugInfo, AC): return alu->getAC();
			case offsetof(DebugInfo, PCL): return pc->getPCL();
			case offsetof(DebugInfo, PCH): return pc->getPCH();
			case offsetof(DebugInfo, PCLS): return pc->getPCLS();
			case offsetof(DebugInfo, PCHS): return pc->getPCHS();
			case offsetof(DebugInfo, ABL): return addr_bus->getABL();
			case offsetof(DebugInfo, ABH): return addr_bus->getABH();
			case offsetof(DebugInfo, DL): return data_bus->getDL();
			case offsetof(DebugInfo, DOR): return data_bus->getDOR();

			case offsetof(DebugInfo, C_OUT): return ToByte(NOT(random->flags->getn_C_OUT()));
			case offsetof(DebugInfo, Z_OUT): return ToByte(NOT(random->flags->getn_Z_OUT()));
			case offsetof(DebugInfo, I_OUT): return ToByte(NOT(random->flags->getn_I_OUT(BRK6E)));
			case offsetof(DebugInfo, D_OUT): return ToByte(NOT(random->flags->getn_D_OUT()));
			case offsetof(DebugInfo, B_OUT): return ToByte(brk->getB_OUT(BRK6E));
			case offsetof(DebugInfo, V_OUT): return ToByte(NOT(random->flags->getn_V_OUT()));
			case offsetof(DebugInfo, N_OUT): return ToByte(NOT(random->flags->getn_N_OUT()));

			case offsetof(DebugInfo, n_PRDY): return ToByte(prdy_latch2.nget());
			case offsetof(DebugInfo, n_NMIP): return ToByte(NOT(nmip_ff.get()));
			case offsetof(DebugInfo, n_IRQP): return ToByte(irqp_latch.nget());
			case offsetof(DebugInfo, RESP): return ToByte(resp_latch.nget());
			case offsetof(DebugInfo, BRK6E): return ToByte(BRK6E);
			case offsetof(DebugInfo, BRK7): return ToByte(wire.BRK7);
			case offsetof(DebugInfo, DORES): return ToByte(wire.DORES);
			case offsetof(DebugInfo, n_DONMI): return ToByte(wire.n_DONMI);
			case offsetof(DebugInfo, n_T2): return ToByte(wire.n_T2);
			case offsetof(DebugInfo, n_T3): return ToByte(wire.n_T3);
			case offsetof(DebugInfo, n_T4): return ToByte(wire.n_T4);
			case offsetof(DebugInfo, n_T5): return ToByte(wire.n_T5);
			case offsetof(DebugInfo, T0): return ToByte(wire.T0);
			case offsetof(DebugInfo, n_T0): return ToByte(wire.n_T0);
			case offsetof(DebugInfo, n_T1X): return ToByte(wire.n_T1X);
			case offsetof(DebugInfo, Z_IR): return ToByte(wire.Z_IR);
			case offsetof(DebugInfo, FETCH): return ToByte(wire.FETCH);
			case offsetof(DebugInfo, n_ready): return ToByte(wire.n_ready);
			case offsetof(DebugInfo, WR): return ToByte(wire.WR);
			case offsetof(DebugInfo, TRES2): return ToByte(disp->getTRES2());
			case offsetof(DebugInfo, ACRL1): return ToByte(wire.ACRL1);
			case offsetof(DebugInfo, ACRL2): return ToByte(wire.ACRL2);
			case offsetof(DebugInfo, T1): return ToByte(disp->getT1());
			case offsetof(DebugInfo, RMW_T6): return ToByte(wire.RMW_T6);
			case offsetof(DebugInfo, RMW_T7): return ToByte(wire.RMW_T7);
			case offsetof(DebugInfo, ENDS): return ToByte(wire.ENDS);
			case offsetof(DebugInfo, ENDX): return ToByte(wire.ENDX);
			case offsetof(DebugInfo, TRES1): return ToByte(wire.TRES1);
			case offsetof(DebugInfo, n_TRESX): return ToByte(wire.n_TRESX);
			case offsetof(DebugInfo, BRFW): return ToByte(wire.BRFW);
			case offsetof(DebugInfo, n_BRTAKEN): return ToByte(wire.n_BRTAKEN);
			case offsetof(DebugInfo, ACR): return ToByte(alu->getACR());
			case offsetof(DebugInfo, AVR): return ToByte(alu->getAVR());
	
			case offsetof(DebugInfo, Y_SB): return cmd.Y_SB;
			case offsetof(DebugInfo, SB_Y): return cmd.SB_Y;
			case offsetof(DebugInfo, X_SB): return cmd.X_SB;
			case offsetof(DebugInfo, SB_X): return cmd.SB_X;
			case offsetof(DebugInfo, S_ADL): return cmd.S_ADL;
			case offsetof(DebugInfo, S_SB): return cmd.S_SB;
			case offsetof(DebugInfo, SB_S): return cmd.SB_S;
			case offsetof(DebugInfo, S_S): return cmd.S_S;
			case offsetof(DebugInfo, NDB_ADD): return cmd.NDB_ADD;
			case offsetof(DebugInfo, DB_ADD): return cmd.DB_ADD;
			case offsetof(DebugInfo, Z_ADD): return cmd.Z_ADD;
			case offsetof(DebugInfo, SB_ADD): return cmd.SB_ADD;
			case offsetof(DebugInfo, ADL_ADD): return cmd.ADL_ADD;
			case offsetof(DebugInfo, n_ACIN): return cmd.n_ACIN;
			case offsetof(DebugInfo, ANDS): return cmd.ANDS;
			case offsetof(DebugInfo, EORS): return cmd.EORS;
			case offsetof(DebugInfo, ORS): return cmd.ORS;
			case offsetof(DebugInfo, SRS): return cmd.SRS;
			case offsetof(DebugInfo, SUMS): return cmd.SUMS;
			case offsetof(DebugInfo, n_DAA): return cmd.n_DAA;
			case offsetof(DebugInfo, n_DSA): return cmd.n_DSA;
			case offsetof(DebugInfo, ADD_SB7): return cmd.ADD_SB7;
			case offsetof(DebugInfo, ADD_SB06): return cmd.ADD_SB06;
			case offsetof(DebugInfo, ADD_ADL): return cmd.ADD_ADL;
			case offsetof(DebugInfo, SB_AC): return cmd.SB_AC;
			case offsetof(DebugInfo, AC_SB): return cmd.AC_SB;
			case offsetof(DebugInfo, AC_DB): return cmd.AC_DB;
			case offsetof(DebugInfo, n_1PC): return ToByte(wire.n_1PC);
			case offsetof(DebugInfo, ADH_PCH): return cmd.ADH_PCH;
			case offsetof(DebugInfo, PCH_PCH): return cmd.PCH_PCH;
			case offsetof(DebugInfo, PCH_ADH): return cmd.PCH_ADH;
			case offsetof(DebugInfo, PCH_DB): return cmd.PCH_DB;
			case offsetof(DebugInfo, ADL_PCL): return cmd.ADL_PCL;
			case offsetof(DebugInfo, PCL_PCL): return cmd.PCL_PCL;
			case offsetof(DebugInfo, PCL_ADL): return cmd.PCL_ADL;
			case offsetof(DebugInfo, PCL_DB): return cmd.PCL_DB;
			case offsetof(DebugInfo, ADH_ABH): return cmd.ADH_ABH;
			case offsetof(DebugInfo, ADL_ABL): return cmd.ADL_ABL;
			case offsetof(DebugInfo, Z_ADL0): return cmd.Z_ADL0;
			case offsetof(DebugInfo, Z_ADL1): return cmd.Z_ADL1;
			case offsetof(DebugInfo, Z_ADL2): return cmd.Z_ADL2;
			case offsetof(DebugInfo, Z_ADH0): return cmd.Z_ADH0;
			case offsetof(DebugInfo, Z_ADH17): return cmd.Z_ADH17;
			case offsetof(DebugInfo, SB_DB): return cmd.SB_DB;
			case offsetof(DebugInfo, SB_ADH): return cmd.SB_ADH;
			case offsetof(DebugInfo, DL_ADL): return cmd.DL_ADL;
			case offsetof(DebugInfo, DL_ADH): return cmd.DL_ADH;
			case offsetof(DebugInfo, DL_DB): return cmd.DL_DB;
			case offsetof(DebugInfo, P_DB): return cmd.P_DB;
			case offsetof(DebugInfo, DB_P): return cmd.DB_P;
			case offsetof(DebugInfo, DBZ_Z): return cmd.DBZ_Z;
			case offsetof(DebugInfo, DB_N): return cmd.DB_N;
			case offsetof(DebugInfo, IR5_C): return cmd.IR5_C;
			case offsetof(DebugInfo, DB_C): return cmd.DB_C;
			case offsetof(DebugInfo, ACR_C): return cmd.ACR_C;
			case offsetof(DebugInfo, IR5_D): return cmd.IR5_D;
			case offsetof(DebugInfo, IR5_I): return cmd.IR5_I;
			case offsetof(DebugInfo, DB_V): return cmd.DB_V;
			case offsetof(DebugInfo, AVR_V): return cmd.AVR_V;
			case offsetof(DebugInfo, Z_V): return cmd.Z_V;

			default:
				return 0;
		}
	}

	void M6502::setDebugSingle(int ofs, uint8_t val)
	{
		TriState BRK6E = wire.BRK6E;

		switch (ofs)
		{
			case offsetof(DebugInfo, SB): SB = val; break;
			case offsetof(DebugInfo, DB): DB = val; break;
			case offsetof(DebugInfo, ADL): ADL = val; break;
			case offsetof(DebugInfo, ADH): ADH = val; break;

			case offsetof(DebugInfo, IR): ir->IROut = val; break;
			case offsetof(DebugInfo, PD): predecode->PD = val; break;
			case offsetof(DebugInfo, Y): regs->setY(val); break;
			case offsetof(DebugInfo, X): regs->setX(val); break;
			case offsetof(DebugInfo, S): regs->setS(val); break;
			case offsetof(DebugInfo, AI): alu->setAI(val); break;
			case offsetof(DebugInfo, BI): alu->setBI(val); break;
			case offsetof(DebugInfo, ADD): alu->setADD(val); break;
			case offsetof(DebugInfo, AC): alu->setAC(val); break;
			case offsetof(DebugInfo, PCL): pc->setPCL(val); break;
			case offsetof(DebugInfo, PCH): pc->setPCH(val); break;
			case offsetof(DebugInfo, PCLS): pc->setPCLS(val); break;
			case offsetof(DebugInfo, PCHS): pc->setPCHS(val); break;
			case offsetof(DebugInfo, ABL): addr_bus->setABL(val); break;
			case offsetof(DebugInfo, ABH): addr_bus->setABH(val); break;
			case offsetof(DebugInfo, DL): data_bus->setDL(val); break;
			case offsetof(DebugInfo, DOR): data_bus->setDOR(val); break;

			case offsetof(DebugInfo, C_OUT): random->flags->set_C_OUT(FromByte(val)); break;
			case offsetof(DebugInfo, Z_OUT): random->flags->set_Z_OUT(FromByte(val)); break;
			case offsetof(DebugInfo, I_OUT): random->flags->set_I_OUT(FromByte(val)); break;
			case offsetof(DebugInfo, D_OUT): random->flags->set_D_OUT(FromByte(val)); break;
			case offsetof(DebugInfo, V_OUT): random->flags->set_V_OUT(FromByte(val)); break;
			case offsetof(DebugInfo, N_OUT): random->flags->set_N_OUT(FromByte(val)); break;

			case offsetof(DebugInfo, BRK6E): wire.BRK6E = FromByte(val); break;
			case offsetof(DebugInfo, BRK7): wire.BRK7 = FromByte(val); break;
			case offsetof(DebugInfo, DORES): wire.DORES = FromByte(val); break;
			case offsetof(DebugInfo, n_DONMI): wire.n_DONMI = FromByte(val); break;
			case offsetof(DebugInfo, n_T2): wire.n_T2 = FromByte(val); break;
			case offsetof(DebugInfo, n_T3): wire.n_T3 = FromByte(val); break;
			case offsetof(DebugInfo, n_T4): wire.n_T4 = FromByte(val); break;
			case offsetof(DebugInfo, n_T5): wire.n_T5 = FromByte(val); break;
			case offsetof(DebugInfo, T0): wire.T0 = FromByte(val); break;
			case offsetof(DebugInfo, n_T0): wire.n_T0 = FromByte(val); break;
			case offsetof(DebugInfo, n_T1X): wire.n_T1X = FromByte(val); break;
			case offsetof(DebugInfo, Z_IR): wire.Z_IR = FromByte(val); break;
			case offsetof(DebugInfo, FETCH): wire.FETCH = FromByte(val); break;
			case offsetof(DebugInfo, n_ready): wire.n_ready = FromByte(val); break;
			case offsetof(DebugInfo, WR): wire.WR = FromByte(val); break;
			case offsetof(DebugInfo, ACRL1): wire.ACRL1 = FromByte(val); break;
			case offsetof(DebugInfo, ACRL2): wire.ACRL2 = FromByte(val); break;
			case offsetof(DebugInfo, RMW_T6): wire.RMW_T6 = FromByte(val); break;
			case offsetof(DebugInfo, RMW_T7): wire.RMW_T7 = FromByte(val); break;
			case offsetof(DebugInfo, ENDS): wire.ENDS = FromByte(val); break;
			case offsetof(DebugInfo, ENDX): wire.ENDX = FromByte(val); break;
			case offsetof(DebugInfo, TRES1): wire.TRES1 = FromByte(val); break;
			case offsetof(DebugInfo, n_TRESX): wire.n_TRESX = FromByte(val); break;
			case offsetof(DebugInfo, BRFW): wire.BRFW = FromByte(val); break;
			case offsetof(DebugInfo, n_BRTAKEN): wire.n_BRTAKEN = FromByte(val); break;
			case offsetof(DebugInfo, AVR): alu->setAVR(FromByte(val)); break;
	
			case offsetof(DebugInfo, Y_SB): cmd.Y_SB = val; break;
			case offsetof(DebugInfo, SB_Y): cmd.SB_Y = val; break;
			case offsetof(DebugInfo, X_SB): cmd.X_SB = val; break;
			case offsetof(DebugInfo, SB_X): cmd.SB_X = val; break;
			case offsetof(DebugInfo, S_ADL): cmd.S_ADL = val; break;
			case offsetof(DebugInfo, S_SB): cmd.S_SB = val; break;
			case offsetof(DebugInfo, SB_S): cmd.SB_S = val; break;
			case offsetof(DebugInfo, S_S): cmd.S_S = val; break;
			case offsetof(DebugInfo, NDB_ADD): cmd.NDB_ADD = val; break;
			case offsetof(DebugInfo, DB_ADD): cmd.DB_ADD = val; break;
			case offsetof(DebugInfo, Z_ADD): cmd.Z_ADD = val; break;
			case offsetof(DebugInfo, SB_ADD): cmd.SB_ADD = val; break;
			case offsetof(DebugInfo, ADL_ADD): cmd.ADL_ADD = val; break;
			case offsetof(DebugInfo, n_ACIN): cmd.n_ACIN = val; break;
			case offsetof(DebugInfo, ANDS): cmd.ANDS = val; break;
			case offsetof(DebugInfo, EORS): cmd.EORS = val; break;
			case offsetof(DebugInfo, ORS): cmd.ORS = val; break;
			case offsetof(DebugInfo, SRS): cmd.SRS = val; break;
			case offsetof(DebugInfo, SUMS): cmd.SUMS = val; break;
			case offsetof(DebugInfo, n_DAA): cmd.n_DAA = val; break;
			case offsetof(DebugInfo, n_DSA): cmd.n_DSA = val; break;
			case offsetof(DebugInfo, ADD_SB7): cmd.ADD_SB7 = val; break;
			case offsetof(DebugInfo, ADD_SB06): cmd.ADD_SB06 = val; break;
			case offsetof(DebugInfo, ADD_ADL): cmd.ADD_ADL = val; break;
			case offsetof(DebugInfo, SB_AC): cmd.SB_AC = val; break;
			case offsetof(DebugInfo, AC_SB): cmd.AC_SB = val; break;
			case offsetof(DebugInfo, AC_DB): cmd.AC_DB = val; break;
			case offsetof(DebugInfo, n_1PC): wire.n_1PC = FromByte(val); break;
			case offsetof(DebugInfo, ADH_PCH): cmd.ADH_PCH = val; break;
			case offsetof(DebugInfo, PCH_PCH): cmd.PCH_PCH = val; break;
			case offsetof(DebugInfo, PCH_ADH): cmd.PCH_ADH = val; break;
			case offsetof(DebugInfo, PCH_DB): cmd.PCH_DB = val; break;
			case offsetof(DebugInfo, ADL_PCL): cmd.ADL_PCL = val; break;
			case offsetof(DebugInfo, PCL_PCL): cmd.PCL_PCL = val; break;
			case offsetof(DebugInfo, PCL_ADL): cmd.PCL_ADL = val; break;
			case offsetof(DebugInfo, PCL_DB): cmd.PCL_DB = val; break;
			case offsetof(DebugInfo, ADH_ABH): cmd.ADH_ABH = val; break;
			case offsetof(DebugInfo, ADL_ABL): cmd.ADL_ABL = val; break;
			case offsetof(DebugInfo, Z_ADL0): cmd.Z_ADL0 = val; break;
			case offsetof(DebugInfo, Z_ADL1): cmd.Z_ADL1 = val; break;
			case offsetof(DebugInfo, Z_ADL2): cmd.Z_ADL2 = val; break;
			case offsetof(DebugInfo, Z_ADH0): cmd.Z_ADH0 = val; break;
			case offsetof(DebugInfo, Z_ADH17): cmd.Z_ADH17 = val; break;
			case offsetof(DebugInfo, SB_DB): cmd.SB_DB = val; break;
			case offsetof(DebugInfo, SB_ADH): cmd.SB_ADH = val; break;
			case offsetof(DebugInfo, DL_ADL): cmd.DL_ADL = val; break;
			case offsetof(DebugInfo, DL_ADH): cmd.DL_ADH = val; break;
			case offsetof(DebugInfo, DL_DB): cmd.DL_DB = val; break;
			case offsetof(DebugInfo, P_DB): cmd.P_DB = val; break;
			case offsetof(DebugInfo, DB_P): cmd.DB_P = val; break;
			case offsetof(DebugInfo, DBZ_Z): cmd.DBZ_Z = val; break;
			case offsetof(DebugInfo, DB_N): cmd.DB_N = val; break;
			case offsetof(DebugInfo, IR5_C): cmd.IR5_C = val; break;
			case offsetof(DebugInfo, DB_C): cmd.DB_C = val; break;
			case offsetof(DebugInfo, ACR_C): cmd.ACR_C = val; break;
			case offsetof(DebugInfo, IR5_D): cmd.IR5_D = val; break;
			case offsetof(DebugInfo, IR5_I): cmd.IR5_I = val; break;
			case offsetof(DebugInfo, DB_V): cmd.DB_V = val; break;
			case offsetof(DebugInfo, AVR_V): cmd.AVR_V = val; break;
			case offsetof(DebugInfo, Z_V): cmd.Z_V = val; break;

			default:
				break;
		}
	}

	uint8_t M6502::getUserRegSingle(int ofs)
	{
		switch (ofs)
		{
			case offsetof(UserRegs, Y): return regs->getY();
			case offsetof(UserRegs, X): return regs->getX();
			case offsetof(UserRegs, S): return regs->getS();
			case offsetof(UserRegs, A): return alu->getAC();
			case offsetof(UserRegs, PCL): return pc->getPCL();
			case offsetof(UserRegs, PCH): return pc->getPCH();

			case offsetof(UserRegs, C_OUT): return ToByte(NOT(random->flags->getn_C_OUT()));
			case offsetof(UserRegs, Z_OUT): return ToByte(NOT(random->flags->getn_Z_OUT()));
			case offsetof(UserRegs, I_OUT): return ToByte(NOT(random->flags->getn_I_OUT(wire.BRK6E)));
			case offsetof(UserRegs, D_OUT): return ToByte(NOT(random->flags->getn_D_OUT()));
			case offsetof(UserRegs, V_OUT): return ToByte(NOT(random->flags->getn_V_OUT()));
			case offsetof(UserRegs, N_OUT): return ToByte(NOT(random->flags->getn_N_OUT()));

			default:
				return 0;
		}
	}

	void M6502::setUserRegSingle(int ofs, uint8_t val)
	{
		switch (ofs)
		{
			case offsetof(UserRegs, Y): regs->setY(val); break;
			case offsetof(UserRegs, X): regs->setX(val); break;
			case offsetof(UserRegs, S): regs->setS(val); break;
			case offsetof(UserRegs, A): alu->setAC(val); break;
			case offsetof(UserRegs, PCL): pc->setPCL(val); break;
			case offsetof(UserRegs, PCH): pc->setPCH(val); break;

			case offsetof(UserRegs, C_OUT): random->flags->set_C_OUT(FromByte(val)); break;
			case offsetof(UserRegs, Z_OUT): random->flags->set_Z_OUT(FromByte(val)); break;
			case offsetof(UserRegs, I_OUT): random->flags->set_I_OUT(FromByte(val)); break;
			case offsetof(UserRegs, D_OUT): random->flags->set_D_OUT(FromByte(val)); break;
			case offsetof(UserRegs, V_OUT): random->flags->set_V_OUT(FromByte(val)); break;
			case offsetof(UserRegs, N_OUT): random->flags->set_N_OUT(FromByte(val)); break;

			default:
				break;
		}
	}
}
