// Debugging mechanisms of the APUSim.

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
		wires.RDY2 = ToByte(wire.RDY2);
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
		wires.SYNC = ToByte(wire.SYNC);
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
		regs.DBInputLatch = pads->Get_DBInputLatch();
		regs.DBOutputLatch = pads->Get_DBOutputLatch();
		regs.OutReg = pads->Get_OutReg();

		regs.LengthCounterSQA = lc[0]->Debug_GetCnt();
		regs.LengthCounterSQB = lc[1]->Debug_GetCnt();
		regs.LengthCounterTRI = lc[2]->Debug_GetCnt();
		regs.LengthCounterRND = lc[3]->Debug_GetCnt();

		regs.SQFreqReg[0] = square[0]->Get_FreqReg();
		regs.SQShiftReg[0] = square[0]->Get_ShiftReg();
		regs.SQFreqCounter[0] = square[0]->Get_FreqCounter();
		regs.SQSweepReg[0] = square[0]->Get_SweepReg();
		regs.SQSweepCounter[0] = square[0]->Get_SweepCounter();
		regs.SQDutyCounter[0] = square[0]->Get_DutyCounter();

		regs.SQFreqReg[1] = square[1]->Get_FreqReg();
		regs.SQShiftReg[1] = square[1]->Get_ShiftReg();
		regs.SQFreqCounter[1] = square[1]->Get_FreqCounter();
		regs.SQSweepReg[1] = square[1]->Get_SweepReg();
		regs.SQSweepCounter[1] = square[1]->Get_SweepCounter();
		regs.SQDutyCounter[1] = square[1]->Get_DutyCounter();

		regs.SQVolumeReg[0] = square[0]->env_unit->Debug_Get_VolumeReg();
		regs.SQDecayCounter[0] = square[0]->env_unit->Debug_Get_DecayCounter();
		regs.SQEnvelopeCounter[0] = square[0]->env_unit->Debug_Get_EnvCounter();

		regs.SQVolumeReg[1] = square[1]->env_unit->Debug_Get_VolumeReg();
		regs.SQDecayCounter[1] = square[1]->env_unit->Debug_Get_DecayCounter();
		regs.SQEnvelopeCounter[1] = square[1]->env_unit->Debug_Get_EnvCounter();

		regs.TRILinearReg = tri->Get_LinearReg();
		regs.TRILinearCounter = tri->Get_LinearCounter();
		regs.TRIFreqReg = tri->Get_FreqReg();
		regs.TRIFreqCounter = tri->Get_FreqCounter();
		regs.TRIOutputCounter = tri->Get_OutputCounter();

		regs.RNDFreqReg = noise->Get_FreqReg();
		regs.RNDVolumeReg = noise->env_unit->Debug_Get_VolumeReg();
		regs.RNDDecayCounter = noise->env_unit->Debug_Get_DecayCounter();
		regs.RNDEnvelopeCounter = noise->env_unit->Debug_Get_EnvCounter();

		regs.DPCMFreqReg = dpcm->Get_FreqReg();
		regs.DPCMSampleReg = dpcm->Get_SampleReg();
		regs.DPCMSampleCounter = dpcm->Get_SampleCounter();
		regs.DPCMSampleBuffer = dpcm->Get_SampleBuffer();
		regs.DPCMSampleBitCounter = dpcm->Get_SampleBitCounter();
		regs.DPCMAddressReg = dpcm->Get_AddressReg();
		regs.DPCMAddressCounter = dpcm->Get_AddressCounter();
		regs.DPCMOutput = dpcm->Get_Output();

		regs.DMABuffer = dma->Get_DMABuffer();
		regs.DMAAddress = dma->Get_DMAAddress();

		regs.Status = Dbg_GetStatus();
	}

	uint8_t APU::GetDebugInfo_Wire(int ofs)
	{
		switch (ofs)
		{
			case offsetof(APU_Interconnects, n_CLK): return ToByte(wire.n_CLK);
			case offsetof(APU_Interconnects, PHI0): return ToByte(wire.PHI0);
			case offsetof(APU_Interconnects, PHI1): return ToByte(wire.PHI1);
			case offsetof(APU_Interconnects, PHI2): return ToByte(wire.PHI2);
			case offsetof(APU_Interconnects, RDY): return ToByte(wire.RDY);
			case offsetof(APU_Interconnects, RDY2): return ToByte(wire.RDY2);
			case offsetof(APU_Interconnects, ACLK): return ToByte(wire.ACLK);
			case offsetof(APU_Interconnects, n_ACLK): return ToByte(wire.n_ACLK);
			case offsetof(APU_Interconnects, RES): return ToByte(wire.RES);
			case offsetof(APU_Interconnects, n_M2): return ToByte(wire.n_M2);
			case offsetof(APU_Interconnects, n_NMI): return ToByte(wire.n_NMI);
			case offsetof(APU_Interconnects, n_IRQ): return ToByte(wire.n_IRQ);
			case offsetof(APU_Interconnects, INT): return ToByte(wire.INT);
			case offsetof(APU_Interconnects, n_LFO1): return ToByte(wire.n_LFO1);
			case offsetof(APU_Interconnects, n_LFO2): return ToByte(wire.n_LFO2);
			case offsetof(APU_Interconnects, RnW): return ToByte(wire.RnW);
			case offsetof(APU_Interconnects, SPR_CPU): return ToByte(wire.SPR_CPU);
			case offsetof(APU_Interconnects, SPR_PPU): return ToByte(wire.SPR_PPU);
			case offsetof(APU_Interconnects, RW): return ToByte(wire.RW);
			case offsetof(APU_Interconnects, RD): return ToByte(wire.RD);
			case offsetof(APU_Interconnects, WR): return ToByte(wire.WR);
			case offsetof(APU_Interconnects, SYNC): return ToByte(wire.SYNC);
			case offsetof(APU_Interconnects, n_DMC_AB): return ToByte(wire.n_DMC_AB);
			case offsetof(APU_Interconnects, RUNDMC): return ToByte(wire.RUNDMC);
			case offsetof(APU_Interconnects, DMCINT): return ToByte(wire.DMCINT);
			case offsetof(APU_Interconnects, DMCRDY): return ToByte(wire.DMCRDY);
			case offsetof(APU_Interconnects, n_R4015): return ToByte(wire.n_R4015);
			case offsetof(APU_Interconnects, n_R4016): return ToByte(wire.n_R4016);
			case offsetof(APU_Interconnects, n_R4017): return ToByte(wire.n_R4017);
			case offsetof(APU_Interconnects, n_R4018): return ToByte(wire.n_R4018);
			case offsetof(APU_Interconnects, n_R4019): return ToByte(wire.n_R4019);
			case offsetof(APU_Interconnects, n_R401A): return ToByte(wire.n_R401A);
			case offsetof(APU_Interconnects, W4000): return ToByte(wire.W4000);
			case offsetof(APU_Interconnects, W4001): return ToByte(wire.W4001);
			case offsetof(APU_Interconnects, W4002): return ToByte(wire.W4002);
			case offsetof(APU_Interconnects, W4003): return ToByte(wire.W4003);
			case offsetof(APU_Interconnects, W4004): return ToByte(wire.W4004);
			case offsetof(APU_Interconnects, W4005): return ToByte(wire.W4005);
			case offsetof(APU_Interconnects, W4006): return ToByte(wire.W4006);
			case offsetof(APU_Interconnects, W4007): return ToByte(wire.W4007);
			case offsetof(APU_Interconnects, W4008): return ToByte(wire.W4008);
			case offsetof(APU_Interconnects, W400A): return ToByte(wire.W400A);
			case offsetof(APU_Interconnects, W400B): return ToByte(wire.W400B);
			case offsetof(APU_Interconnects, W400C): return ToByte(wire.W400C);
			case offsetof(APU_Interconnects, W400E): return ToByte(wire.W400E);
			case offsetof(APU_Interconnects, W400F): return ToByte(wire.W400F);
			case offsetof(APU_Interconnects, W4010): return ToByte(wire.W4010);
			case offsetof(APU_Interconnects, W4011): return ToByte(wire.W4011);
			case offsetof(APU_Interconnects, W4012): return ToByte(wire.W4012);
			case offsetof(APU_Interconnects, W4013): return ToByte(wire.W4013);
			case offsetof(APU_Interconnects, W4014): return ToByte(wire.W4014);
			case offsetof(APU_Interconnects, W4015): return ToByte(wire.W4015);
			case offsetof(APU_Interconnects, W4016): return ToByte(wire.W4016);
			case offsetof(APU_Interconnects, W4017): return ToByte(wire.W4017);
			case offsetof(APU_Interconnects, W401A): return ToByte(wire.W401A);
			case offsetof(APU_Interconnects, SQA_LC): return ToByte(wire.SQA_LC);
			case offsetof(APU_Interconnects, SQB_LC): return ToByte(wire.SQB_LC);
			case offsetof(APU_Interconnects, TRI_LC): return ToByte(wire.TRI_LC);
			case offsetof(APU_Interconnects, RND_LC): return ToByte(wire.RND_LC);
			case offsetof(APU_Interconnects, NOSQA): return ToByte(wire.NOSQA);
			case offsetof(APU_Interconnects, NOSQB): return ToByte(wire.NOSQB);
			case offsetof(APU_Interconnects, NOTRI): return ToByte(wire.NOTRI);
			case offsetof(APU_Interconnects, NORND): return ToByte(wire.NORND);
			case offsetof(APU_Interconnects, DBG): return ToByte(wire.DBG);
			case offsetof(APU_Interconnects, n_DBGRD): return ToByte(wire.n_DBGRD);
			case offsetof(APU_Interconnects, LOCK): return ToByte(wire.LOCK);

			default:
				return 0;
		}
	}

	void APU::SetDebugInfo_Wire(int ofs, uint8_t val)
	{
		switch (ofs)
		{
			case offsetof(APU_Interconnects, n_CLK): wire.n_CLK = FromByte(val); break;
			case offsetof(APU_Interconnects, PHI0): wire.PHI0 = FromByte(val); break;
			case offsetof(APU_Interconnects, PHI1): wire.PHI1 = FromByte(val); break;
			case offsetof(APU_Interconnects, PHI2): wire.PHI2 = FromByte(val); break;
			case offsetof(APU_Interconnects, RDY): wire.RDY = FromByte(val); break;
			case offsetof(APU_Interconnects, RDY2): wire.RDY2 = FromByte(val); break;
			case offsetof(APU_Interconnects, ACLK): wire.ACLK = FromByte(val); break;
			case offsetof(APU_Interconnects, n_ACLK): wire.n_ACLK = FromByte(val); break;
			case offsetof(APU_Interconnects, RES): wire.RES = FromByte(val); break;
			case offsetof(APU_Interconnects, n_M2): wire.n_M2 = FromByte(val); break;
			case offsetof(APU_Interconnects, n_NMI): wire.n_NMI = FromByte(val); break;
			case offsetof(APU_Interconnects, n_IRQ): wire.n_IRQ = FromByte(val); break;
			case offsetof(APU_Interconnects, INT): wire.INT = FromByte(val); break;
			case offsetof(APU_Interconnects, n_LFO1): wire.n_LFO1 = FromByte(val); break;
			case offsetof(APU_Interconnects, n_LFO2): wire.n_LFO2 = FromByte(val); break;
			case offsetof(APU_Interconnects, RnW): wire.RnW = FromByte(val); break;
			case offsetof(APU_Interconnects, SPR_CPU): wire.SPR_CPU = FromByte(val); break;
			case offsetof(APU_Interconnects, SPR_PPU): wire.SPR_PPU = FromByte(val); break;
			case offsetof(APU_Interconnects, RW): wire.RW = FromByte(val); break;
			case offsetof(APU_Interconnects, RD): wire.RD = FromByte(val); break;
			case offsetof(APU_Interconnects, WR): wire.WR = FromByte(val); break;
			case offsetof(APU_Interconnects, SYNC): wire.SYNC = FromByte(val); break;
			case offsetof(APU_Interconnects, n_DMC_AB): wire.n_DMC_AB = FromByte(val); break;
			case offsetof(APU_Interconnects, RUNDMC): wire.RUNDMC = FromByte(val); break;
			case offsetof(APU_Interconnects, DMCINT): wire.DMCINT = FromByte(val); break;
			case offsetof(APU_Interconnects, DMCRDY): wire.DMCRDY = FromByte(val); break;
			case offsetof(APU_Interconnects, n_R4015): wire.n_R4015 = FromByte(val); break;
			case offsetof(APU_Interconnects, n_R4016): wire.n_R4016 = FromByte(val); break;
			case offsetof(APU_Interconnects, n_R4017): wire.n_R4017 = FromByte(val); break;
			case offsetof(APU_Interconnects, n_R4018): wire.n_R4018 = FromByte(val); break;
			case offsetof(APU_Interconnects, n_R4019): wire.n_R4019 = FromByte(val); break;
			case offsetof(APU_Interconnects, n_R401A): wire.n_R401A = FromByte(val); break;
			case offsetof(APU_Interconnects, W4000): wire.W4000 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4001): wire.W4001 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4002): wire.W4002 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4003): wire.W4003 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4004): wire.W4004 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4005): wire.W4005 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4006): wire.W4006 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4007): wire.W4007 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4008): wire.W4008 = FromByte(val); break;
			case offsetof(APU_Interconnects, W400A): wire.W400A = FromByte(val); break;
			case offsetof(APU_Interconnects, W400B): wire.W400B = FromByte(val); break;
			case offsetof(APU_Interconnects, W400C): wire.W400C = FromByte(val); break;
			case offsetof(APU_Interconnects, W400E): wire.W400E = FromByte(val); break;
			case offsetof(APU_Interconnects, W400F): wire.W400F = FromByte(val); break;
			case offsetof(APU_Interconnects, W4010): wire.W4010 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4011): wire.W4011 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4012): wire.W4012 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4013): wire.W4013 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4014): wire.W4014 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4015): wire.W4015 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4016): wire.W4016 = FromByte(val); break;
			case offsetof(APU_Interconnects, W4017): wire.W4017 = FromByte(val); break;
			case offsetof(APU_Interconnects, W401A): wire.W401A = FromByte(val); break;
			case offsetof(APU_Interconnects, SQA_LC): wire.SQA_LC = FromByte(val); break;
			case offsetof(APU_Interconnects, SQB_LC): wire.SQB_LC = FromByte(val); break;
			case offsetof(APU_Interconnects, TRI_LC): wire.TRI_LC = FromByte(val); break;
			case offsetof(APU_Interconnects, RND_LC): wire.RND_LC = FromByte(val); break;
			case offsetof(APU_Interconnects, NOSQA): wire.NOSQA = FromByte(val); break;
			case offsetof(APU_Interconnects, NOSQB): wire.NOSQB = FromByte(val); break;
			case offsetof(APU_Interconnects, NOTRI): wire.NOTRI = FromByte(val); break;
			case offsetof(APU_Interconnects, NORND): wire.NORND = FromByte(val); break;
			case offsetof(APU_Interconnects, DBG): wire.DBG = FromByte(val); break;
			case offsetof(APU_Interconnects, n_DBGRD): wire.n_DBGRD = FromByte(val); break;
			case offsetof(APU_Interconnects, LOCK): wire.LOCK = FromByte(val); break;

			default:
				break;
		}
	}

	uint32_t APU::GetDebugInfo_Reg(int ofs)
	{
		switch (ofs)
		{
			case offsetof(APU_Registers, DBOutputLatch): return pads->Get_DBOutputLatch();
			case offsetof(APU_Registers, DBInputLatch): return pads->Get_DBInputLatch();
			case offsetof(APU_Registers, OutReg): return pads->Get_OutReg();
			case offsetof(APU_Registers, LengthCounterSQA): return lc[0]->Debug_GetCnt();
			case offsetof(APU_Registers, LengthCounterSQB): return lc[1]->Debug_GetCnt();
			case offsetof(APU_Registers, LengthCounterTRI): return lc[2]->Debug_GetCnt();
			case offsetof(APU_Registers, LengthCounterRND): return lc[3]->Debug_GetCnt();
			case offsetof(APU_Registers, SQFreqReg[0]): return square[0]->Get_FreqReg();
			case offsetof(APU_Registers, SQFreqReg[1]): return square[1]->Get_FreqReg();
			case offsetof(APU_Registers, SQShiftReg[0]): return square[0]->Get_ShiftReg();
			case offsetof(APU_Registers, SQShiftReg[1]): return square[1]->Get_ShiftReg();
			case offsetof(APU_Registers, SQFreqCounter[0]): return square[0]->Get_FreqCounter();
			case offsetof(APU_Registers, SQFreqCounter[1]): return square[1]->Get_FreqCounter();
			case offsetof(APU_Registers, SQVolumeReg[0]): return square[0]->env_unit->Debug_Get_VolumeReg();
			case offsetof(APU_Registers, SQVolumeReg[1]): return square[1]->env_unit->Debug_Get_VolumeReg();
			case offsetof(APU_Registers, SQDecayCounter[0]): return square[0]->env_unit->Debug_Get_DecayCounter();
			case offsetof(APU_Registers, SQDecayCounter[1]): return square[1]->env_unit->Debug_Get_DecayCounter();
			case offsetof(APU_Registers, SQEnvelopeCounter[0]): return square[0]->env_unit->Debug_Get_EnvCounter();
			case offsetof(APU_Registers, SQEnvelopeCounter[1]): return square[1]->env_unit->Debug_Get_EnvCounter();
			case offsetof(APU_Registers, SQSweepReg[0]): return square[0]->Get_SweepReg();
			case offsetof(APU_Registers, SQSweepReg[1]): return square[1]->Get_SweepReg();
			case offsetof(APU_Registers, SQSweepCounter[0]): return square[0]->Get_SweepCounter();
			case offsetof(APU_Registers, SQSweepCounter[1]): return square[1]->Get_SweepCounter();
			case offsetof(APU_Registers, SQDutyCounter[0]): return square[0]->Get_DutyCounter();
			case offsetof(APU_Registers, SQDutyCounter[1]): return square[1]->Get_DutyCounter();
			case offsetof(APU_Registers, TRILinearReg): return tri->Get_LinearReg();
			case offsetof(APU_Registers, TRILinearCounter): return tri->Get_LinearCounter();
			case offsetof(APU_Registers, TRIFreqReg): return tri->Get_FreqReg();
			case offsetof(APU_Registers, TRIFreqCounter): return tri->Get_FreqCounter();
			case offsetof(APU_Registers, TRIOutputCounter): return tri->Get_OutputCounter();
			case offsetof(APU_Registers, RNDFreqReg): return noise->Get_FreqReg();
			case offsetof(APU_Registers, RNDVolumeReg): return noise->env_unit->Debug_Get_VolumeReg();
			case offsetof(APU_Registers, RNDDecayCounter): return noise->env_unit->Debug_Get_DecayCounter();
			case offsetof(APU_Registers, RNDEnvelopeCounter): return noise->env_unit->Debug_Get_EnvCounter();
			case offsetof(APU_Registers, DPCMFreqReg): return dpcm->Get_FreqReg();
			case offsetof(APU_Registers, DPCMSampleReg): return dpcm->Get_SampleReg();
			case offsetof(APU_Registers, DPCMSampleCounter): return dpcm->Get_SampleCounter();
			case offsetof(APU_Registers, DPCMSampleBuffer): return dpcm->Get_SampleBuffer();
			case offsetof(APU_Registers, DPCMSampleBitCounter): return dpcm->Get_SampleBitCounter();
			case offsetof(APU_Registers, DPCMAddressReg): return dpcm->Get_AddressReg();
			case offsetof(APU_Registers, DPCMAddressCounter): return dpcm->Get_AddressCounter();
			case offsetof(APU_Registers, DPCMOutput): return dpcm->Get_Output();
			case offsetof(APU_Registers, DMABuffer): return dma->Get_DMABuffer();
			case offsetof(APU_Registers, DMAAddress): return dma->Get_DMAAddress();
			case offsetof(APU_Registers, Status): return Dbg_GetStatus();

			default:
				break;
		}

		return 0;
	}

	void APU::SetDebugInfo_Reg(int ofs, uint32_t val)
	{
		switch (ofs)
		{
			case offsetof(APU_Registers, DBOutputLatch): pads->Set_DBOutputLatch(val); break;
			case offsetof(APU_Registers, DBInputLatch): pads->Set_DBInputLatch(val); break;
			case offsetof(APU_Registers, OutReg): pads->Set_OutReg(val); break;
			case offsetof(APU_Registers, LengthCounterSQA): lc[0]->Debug_SetCnt(val); break;
			case offsetof(APU_Registers, LengthCounterSQB): lc[1]->Debug_SetCnt(val); break;
			case offsetof(APU_Registers, LengthCounterTRI): lc[2]->Debug_SetCnt(val); break;
			case offsetof(APU_Registers, LengthCounterRND): lc[3]->Debug_SetCnt(val); break;
			case offsetof(APU_Registers, SQFreqReg[0]): square[0]->Set_FreqReg(val); break;
			case offsetof(APU_Registers, SQFreqReg[1]): square[1]->Set_FreqReg(val); break;
			case offsetof(APU_Registers, SQShiftReg[0]): square[0]->Set_ShiftReg(val); break;
			case offsetof(APU_Registers, SQShiftReg[1]): square[1]->Set_ShiftReg(val); break;
			case offsetof(APU_Registers, SQFreqCounter[0]): square[0]->Set_FreqCounter(val); break;
			case offsetof(APU_Registers, SQFreqCounter[1]): square[1]->Set_FreqCounter(val); break;
			case offsetof(APU_Registers, SQVolumeReg[0]): square[0]->env_unit->Debug_Set_VolumeReg(val); break;
			case offsetof(APU_Registers, SQVolumeReg[1]): square[1]->env_unit->Debug_Set_VolumeReg(val); break;
			case offsetof(APU_Registers, SQDecayCounter[0]): square[0]->env_unit->Debug_Set_DecayCounter(val); break;
			case offsetof(APU_Registers, SQDecayCounter[1]): square[1]->env_unit->Debug_Set_DecayCounter(val); break;
			case offsetof(APU_Registers, SQEnvelopeCounter[0]): square[0]->env_unit->Debug_Set_EnvCounter(val); break;
			case offsetof(APU_Registers, SQEnvelopeCounter[1]): square[1]->env_unit->Debug_Set_EnvCounter(val); break;
			case offsetof(APU_Registers, SQSweepReg[0]): square[0]->Set_SweepReg(val); break;
			case offsetof(APU_Registers, SQSweepReg[1]): square[1]->Set_SweepReg(val); break;
			case offsetof(APU_Registers, SQSweepCounter[0]): square[0]->Set_SweepCounter(val); break;
			case offsetof(APU_Registers, SQSweepCounter[1]): square[1]->Set_SweepCounter(val); break;
			case offsetof(APU_Registers, SQDutyCounter[0]): square[0]->Set_DutyCounter(val); break;
			case offsetof(APU_Registers, SQDutyCounter[1]): square[1]->Set_DutyCounter(val); break;
			case offsetof(APU_Registers, TRILinearReg): tri->Set_LinearReg(val); break;
			case offsetof(APU_Registers, TRILinearCounter): tri->Set_LinearCounter(val); break;
			case offsetof(APU_Registers, TRIFreqReg): tri->Set_FreqReg(val); break;
			case offsetof(APU_Registers, TRIFreqCounter): tri->Set_FreqCounter(val); break;
			case offsetof(APU_Registers, TRIOutputCounter): tri->Set_OutputCounter(val); break;
			case offsetof(APU_Registers, RNDFreqReg): noise->Set_FreqReg(val); break;
			case offsetof(APU_Registers, RNDVolumeReg): noise->env_unit->Debug_Set_VolumeReg(val); break;
			case offsetof(APU_Registers, RNDDecayCounter): noise->env_unit->Debug_Set_DecayCounter(val); break;
			case offsetof(APU_Registers, RNDEnvelopeCounter): noise->env_unit->Debug_Set_EnvCounter(val); break;
			case offsetof(APU_Registers, DPCMFreqReg): dpcm->Set_FreqReg(val); break;
			case offsetof(APU_Registers, DPCMSampleReg): dpcm->Set_SampleReg(val); break;
			case offsetof(APU_Registers, DPCMSampleCounter): dpcm->Set_SampleCounter(val); break;
			case offsetof(APU_Registers, DPCMSampleBuffer): dpcm->Set_SampleBuffer(val); break;
			case offsetof(APU_Registers, DPCMSampleBitCounter): dpcm->Set_SampleBitCounter(val); break;
			case offsetof(APU_Registers, DPCMAddressReg): dpcm->Set_AddressReg(val); break;
			case offsetof(APU_Registers, DPCMAddressCounter): dpcm->Set_AddressCounter(val); break;
			case offsetof(APU_Registers, DPCMOutput): dpcm->Set_Output(val); break;
			case offsetof(APU_Registers, DMABuffer): dma->Set_DMABuffer(val); break;
			case offsetof(APU_Registers, DMAAddress): dma->Set_DMAAddress(val); break;
			case offsetof(APU_Registers, Status): Dbg_SetStatus((uint8_t)val); break;

			default:
				break;
		}
	}

	uint8_t APU::Dbg_GetStatus()
	{
		uint8_t val = 0;

		val |= (lc[0]->Debug_GetEnable() << 0);
		val |= (lc[1]->Debug_GetEnable() << 1);
		val |= (lc[2]->Debug_GetEnable() << 2);
		val |= (lc[3]->Debug_GetEnable() << 3);
		val |= (dpcm->GetDpcmEnable() << 4);

		return val;
	}

	void APU::Dbg_SetStatus(uint8_t val)
	{
		lc[0]->Debug_SetEnable(((val >> 0) & 1) != 0);
		lc[1]->Debug_SetEnable(((val >> 1) & 1) != 0);
		lc[2]->Debug_SetEnable(((val >> 2) & 1) != 0);
		lc[3]->Debug_SetEnable(((val >> 3) & 1) != 0);
		dpcm->SetDpcmEnable(((val >> 4) & 1) != 0);
	}
}
