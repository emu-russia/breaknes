// Debugging mechanisms of the APU.

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	void APU::GetDebugInfo_Wires(APU_Interconnects& wires)
	{
		wires.n_CLK = ToByte(wire.n_CLK);
		wires.PHI0 = ToByte(wire.PHI0);
		wires.PHI1 = ToByte(wire.PHI1);
		wires.PHI2 = ToByte(wire.PHI2);
		wires.RDY = ToByte(wire.RDY);
		wires.ACLK = ToByte(wire.ACLK);
		wires.n_ACLK = ToByte(wire.n_ACLK);
		wires.RES = ToByte(wire.RES);
		wires.n_M2 = ToByte(wire.n_M2);
		wires.n_NMI = ToByte(wire.n_NMI);
		wires.n_IRQ = ToByte(wire.n_IRQ);
		wires.INT = ToByte(wire.INT);
		wires.n_LFO1 = ToByte(wire.n_LFO1);
		wires.n_LFO2 = ToByte(wire.n_LFO2);
		wires.RnW = ToByte(wire.RnW);
		wires.SPR_CPU = ToByte(wire.SPR_CPU);
		wires.SPR_PPU = ToByte(wire.SPR_PPU);
		wires.RW = ToByte(wire.RW);
		wires.RD = ToByte(wire.RD);
		wires.WR = ToByte(wire.WR);
		wires.n_DMC_AB = ToByte(wire.n_DMC_AB);
		wires.RUNDMC = ToByte(wire.RUNDMC);
		wires.DMCINT = ToByte(wire.DMCINT);
		wires.DMCRDY = ToByte(wire.DMCRDY);
		wires.n_R4015 = ToByte(wire.n_R4015);
		wires.n_R4016 = ToByte(wire.n_R4016);
		wires.n_R4017 = ToByte(wire.n_R4017);
		wires.n_R4018 = ToByte(wire.n_R4018);
		wires.n_R4019 = ToByte(wire.n_R4019);
		wires.n_R401A = ToByte(wire.n_R401A);
		wires.W4000 = ToByte(wire.W4000);
		wires.W4001 = ToByte(wire.W4001);
		wires.W4002 = ToByte(wire.W4002);
		wires.W4003 = ToByte(wire.W4003);
		wires.W4004 = ToByte(wire.W4004);
		wires.W4005 = ToByte(wire.W4005);
		wires.W4006 = ToByte(wire.W4006);
		wires.W4007 = ToByte(wire.W4007);
		wires.W4008 = ToByte(wire.W4008);
		wires.W400A = ToByte(wire.W400A);
		wires.W400B = ToByte(wire.W400B);
		wires.W400C = ToByte(wire.W400C);
		wires.W400E = ToByte(wire.W400E);
		wires.W400F = ToByte(wire.W400F);
		wires.W4010 = ToByte(wire.W4010);
		wires.W4011 = ToByte(wire.W4011);
		wires.W4012 = ToByte(wire.W4012);
		wires.W4013 = ToByte(wire.W4013);
		wires.W4014 = ToByte(wire.W4014);
		wires.W4015 = ToByte(wire.W4015);
		wires.W4016 = ToByte(wire.W4016);
		wires.W4017 = ToByte(wire.W4017);
		wires.W401A = ToByte(wire.W401A);
		wires.SQA_LC = ToByte(wire.SQA_LC);
		wires.SQB_LC = ToByte(wire.SQB_LC);
		wires.TRI_LC = ToByte(wire.TRI_LC);
		wires.RND_LC = ToByte(wire.RND_LC);
		wires.NOSQA = ToByte(wire.NOSQA);
		wires.NOSQB = ToByte(wire.NOSQB);
		wires.NOTRI = ToByte(wire.NOTRI);
		wires.NORND = ToByte(wire.NORND);
		wires.DBG = ToByte(wire.DBG);
		wires.n_DBGRD = ToByte(wire.n_DBGRD);
		wires.LOCK = ToByte(wire.LOCK);
	}

	void APU::GetDebugInfo_Regs(APU_Registers& regs)
	{

	}
}
