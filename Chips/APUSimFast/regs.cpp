// Register Decoder

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
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
		const uint64_t key = uint64_t((apu->CPU_Addr & 0xffe0) == 0x4000) |
			(uint64_t(apu->Ax & 0x1f) << 1) | (uint64_t(apu->wire.RnW) << 8) |
			(uint64_t(apu->wire.PHI1) << 16) | (uint64_t(apu->wire.DBG) << 24);
		if (!decoded || key != decoded_key || decoded_epoch != apu->mutation_epoch)
		{
			sim_Predecode();
			sim_Decoder();
			sim_RegOps();
			decoded_key = key;
			decoded_epoch = apu->mutation_epoch;
			decoded = true;
		}
		sim_DebugLock();
	}

	void RegsDecoder::sim_Predecode()
	{
		// CPU high-address qualification and muxed low-address selection are
		// intentionally independent: OAM/DMC may own Ax while the CPU is stopped.
		const TriState outside = (apu->CPU_Addr & 0xffe0) == 0x4000 ? TriState::Zero : TriState::One;
		nREGWR = NOT(NOR(outside, apu->wire.RnW));
		nREGRD = NOT(NOR(outside, NOT(apu->wire.RnW)));
		apu->wire.n_DBGRD = NAND(apu->wire.DBG, NOT(nREGRD));
	}

	void RegsDecoder::sim_Decoder()
	{
		static constexpr uint8_t addresses[29] = { 24,25,26,26,21,3,2,7,1,4,5,12,6,0,8,21,10,17,11,15,14,23,19,22,18,22,16,23,20 };
		static constexpr uint32_t reads = (1u<<0)|(1u<<1)|(1u<<2)|(1u<<4)|(1u<<21)|(1u<<23);
		for (unsigned n = 0; n < 29; ++n)
			pla[n] = NOR((reads & (1u << n)) != 0 ? nREGRD : nREGWR,
				(apu->Ax & 31) == addresses[n] ? TriState::Zero : TriState::One);
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
		TriState ACLK1 = apu->wire.ACLK1;
		TriState RES = apu->wire.RES;
		TriState W401A = apu->wire.W401A;

		TriState d = MUX(W401A, MUX(ACLK1, TriState::Z, NOR(lock_latch.nget(), RES)), apu->GetDBBit(7));
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
