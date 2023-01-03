// Register Decoder

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	RegsDecoder::RegsDecoder(APU* parent)
	{
		apu = parent;
	}

	RegsDecoder::~RegsDecoder()
	{
	}

	void RegsDecoder::sim()
	{
		sim_Predecode();
		sim_Decoder();
		sim_RegOps();
		sim_DebugLock();
	}

	void RegsDecoder::sim_Predecode()
	{
		TriState CpuA[16]{};

		for (size_t n = 0; n < 16; n++)
		{
			CpuA[n] = ((apu->CPU_Addr >> n) & 1) != 0 ? TriState::One : TriState::Zero;
		}

		auto REGWR = NOR12 (CpuA[5], CpuA[6], CpuA[7], CpuA[8], CpuA[9], CpuA[10], CpuA[11], CpuA[12], CpuA[13], NOT(CpuA[14]), CpuA[15], apu->wire.RnW);
		nREGWR = NOT(REGWR);

		auto REGRD = NOR12 (CpuA[5], CpuA[6], CpuA[7], CpuA[8], CpuA[9], CpuA[10], CpuA[11], CpuA[12], CpuA[13], NOT(CpuA[14]), CpuA[15], NOT(apu->wire.RnW));
		nREGRD = NOT(REGRD);

		apu->wire.n_DBGRD = NAND(apu->wire.DBG, NOT(nREGRD));
	}

	void RegsDecoder::sim_Decoder()
	{
		TriState A[5]{};
		TriState nA[5]{};

		// Decoder

		for (size_t n = 0; n < 5; n++)
		{
			A[n] = ((apu->Ax >> n) & 1) != 0 ? TriState::One : TriState::Zero;
		}

		for (size_t n = 0; n < 5; n++)
		{
			nA[n] = NOT(A[n]);
		}

		pla[0] = NOR6 (nREGRD, A[0], A[1], A[2], nA[3], nA[4]);
		pla[1] = NOR6 (nREGRD, nA[0], A[1], A[2], nA[3], nA[4]);
		pla[2] = NOR6 (nREGRD, A[0], nA[1], A[2], nA[3], nA[4]);
		pla[3] = NOR6 (nREGWR, A[0], nA[1], A[2], nA[3], nA[4]);

		pla[4] = NOR6 (nREGRD, nA[0], A[1], nA[2], A[3], nA[4]);
		pla[5] = NOR6 (nREGWR, nA[0], nA[1], A[2], A[3], A[4]);
		pla[6] = NOR6 (nREGWR, A[0], nA[1], A[2], A[3], A[4]);
		pla[7] = NOR6 (nREGWR, nA[0], nA[1], nA[2], A[3], A[4]);
		pla[8] = NOR6 (nREGWR, nA[0], A[1], A[2], A[3], A[4]);

		pla[9] = NOR6 (nREGWR, A[0], A[1], nA[2], A[3], A[4]);
		pla[10] = NOR6 (nREGWR, nA[0], A[1], nA[2], A[3], A[4]);
		pla[11] = NOR6 (nREGWR, A[0], A[1], nA[2], nA[3], A[4]);
		pla[12] = NOR6 (nREGWR, A[0], nA[1], nA[2], A[3], A[4]);
		pla[13] = NOR6 (nREGWR, A[0], A[1], A[2], A[3], A[4]);

		pla[14] = NOR6 (nREGWR, A[0], A[1], A[2], nA[3], A[4]);
		pla[15] = NOR6 (nREGWR, nA[0], A[1], nA[2], A[3], nA[4]);
		pla[16] = NOR6 (nREGWR, A[0], nA[1], A[2], nA[3], A[4]);
		pla[17] = NOR6 (nREGWR, nA[0], A[1], A[2], A[3], nA[4]);
		pla[18] = NOR6 (nREGWR, nA[0], nA[1], A[2], nA[3], A[4]);

		pla[19] = NOR6 (nREGWR, nA[0], nA[1], nA[2], nA[3], A[4]);
		pla[20] = NOR6 (nREGWR, A[0], nA[1], nA[2], nA[3], A[4]);
		pla[21] = NOR6 (nREGRD, nA[0], nA[1], nA[2], A[3], nA[4]);
		pla[22] = NOR6 (nREGWR, nA[0], nA[1], A[2], A[3], nA[4]);
		pla[23] = NOR6 (nREGRD, A[0], nA[1], nA[2], A[3], nA[4]);

		pla[24] = NOR6 (nREGWR, A[0], nA[1], A[2], A[3], nA[4]);
		pla[25] = NOR6 (nREGWR, A[0], nA[1], nA[2], A[3], nA[4]);
		pla[26] = NOR6 (nREGWR, A[0], A[1], A[2], A[3], nA[4]);
		pla[27] = NOR6 (nREGWR, nA[0], nA[1], nA[2], A[3], nA[4]);
		pla[28] = NOR6 (nREGWR, A[0], A[1], nA[2], A[3], nA[4]);
	}

	void RegsDecoder::sim_RegOps()
	{
		TriState PHI1 = apu->wire.PHI1;
		TriState DBG = apu->wire.DBG;

		// Select a register index.
		// Note that during PHI1 all write operations are disabled.

		apu->wire.n_R4018 = NAND(DBG, pla[0]);
		apu->wire.n_R401A = NAND(DBG, pla[2]);
		
		apu->wire.n_R4015 = NOT(pla[4]);
		apu->wire.W4002 = NOR(PHI1, NOT(pla[6]));
		apu->wire.W4001 = NOR(PHI1, NOT(pla[8]));
		apu->wire.W4005 = NOR(PHI1, NOT(pla[10]));
		apu->wire.W4006 = NOR(PHI1, NOT(pla[12]));
		apu->wire.W4008 = NOR(PHI1, NOT(pla[14]));
		apu->wire.W400A = NOR(PHI1, NOT(pla[16]));
		apu->wire.W400B = NOR(PHI1, NOT(pla[18]));
		apu->wire.W400E = NOR(PHI1, NOT(pla[20]));
		apu->wire.W4013 = NOR(PHI1, NOT(pla[22]));
		apu->wire.W4012 = NOR(PHI1, NOT(pla[24]));
		apu->wire.W4010 = NOR(PHI1, NOT(pla[26]));
		apu->wire.W4014 = NOR(PHI1, NOT(pla[28]));

		apu->wire.n_R4019 = NAND(DBG, pla[1]);
		apu->wire.W401A = NOR(PHI1, NAND(DBG, pla[3]));
		
		apu->wire.W4003 = NOR(PHI1, NOT(pla[5]));
		apu->wire.W4007 = NOR(PHI1, NOT(pla[7]));
		apu->wire.W4004 = NOR(PHI1, NOT(pla[9]));
		apu->wire.W400C = NOR(PHI1, NOT(pla[11]));
		apu->wire.W4000 = NOR(PHI1, NOT(pla[13]));
		apu->wire.W4015 = NOR(PHI1, NOT(pla[15]));
		apu->wire.W4011 = NOR(PHI1, NOT(pla[17]));
		apu->wire.W400F = NOR(PHI1, NOT(pla[19]));
		apu->wire.n_R4017 = NOT(pla[21]);
		apu->wire.n_R4016 = NOT(pla[23]);
		apu->wire.W4016 = NOR(PHI1, NOT(pla[25]));
		apu->wire.W4017 = NOR(PHI1, NOT(pla[27]));
	}

	void RegsDecoder::sim_DebugLock()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;
		TriState W401A = apu->wire.W401A;

		TriState d = MUX(W401A, MUX(n_ACLK, TriState::Z, NOR(lock_latch.nget(), RES)), apu->GetDBBit(7));
		lock_latch.set(d, TriState::One);
		apu->wire.LOCK = NOR(lock_latch.nget(), RES);
	}

	void RegsDecoder::sim_DebugRegisters()
	{
		if (apu->wire.DBG == TriState::Zero)
		{
			return;
		}

		for (size_t n = 0; n < 4; n++)
		{
			apu->SetDBBit(n, NOT(apu->wire.n_R4018) ? apu->SQA_Out[n] : TriState::Z);
			apu->SetDBBit(n + 4, NOT(apu->wire.n_R4018) ? apu->SQB_Out[n] : TriState::Z);
			apu->SetDBBit(n, NOT(apu->wire.n_R4019) ? apu->TRI_Out[n] : TriState::Z);
			apu->SetDBBit(n + 4, NOT(apu->wire.n_R4019) ? apu->RND_Out[n] : TriState::Z);
		}

		for (size_t n = 0; n < 7; n++)
		{
			apu->SetDBBit(n, NOT(apu->wire.n_R401A) ? apu->DMC_Out[n] : TriState::Z);
		}
	}
}
