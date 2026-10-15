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
		// All outside-window inputs produce the same inactive strobes. The CPU
		// high-address qualifier remains independent of the DMA-muxed low address.
		const bool inside = (apu->CPU_Addr & 0xffe0) == 0x4000;
		#ifdef APUSIMFAST_UNCANONICAL_REG_KEY
		const uint64_t key = uint64_t(inside) | (uint64_t(apu->Ax & 0x1f) << 1) |
			(uint64_t(apu->wire.RnW) << 8) | (uint64_t(apu->wire.PHI1) << 16) |
			(uint64_t(apu->wire.DBG) << 24);
		#else
		const uint64_t key = inside ? 1u | (uint64_t(apu->Ax & 0x1f) << 1) |
			(uint64_t(apu->wire.RnW) << 8) | (uint64_t(apu->wire.PHI1) << 16) |
			(uint64_t(apu->wire.DBG) << 24) : 0;
		#endif
		strobes_changed = false;
		if (!decoded || key != decoded_key || decoded_epoch != apu->mutation_epoch)
		{
			#ifdef APUSIMFAST_LEGACY_REG_DECODE
			sim_Predecode();
			sim_Decoder();
			sim_RegOps();
			strobes_changed = true;
			#else
			sim_PackedOps();
			#endif
			decoded_key = key;
			decoded_epoch = apu->mutation_epoch;
			decoded = true;
		}
		sim_DebugLock();
	}

	void RegsDecoder::sim_PackedOps()
	{
		sim_Predecode();
		const uint32_t select = uint32_t(1) << (apu->Ax & 31);
		const uint32_t writes = nREGWR == TriState::Zero && (apu->wire.PHI1 & 1) == 0 ? select : 0;
		const uint32_t reads = nREGRD == TriState::Zero ? select : 0;
		auto& w = apu->wire;
		// Every strobe below is a function of these three values (and n_DBGRD of DBG and reads).
		strobes_changed = !decoded || writes != last_writes || reads != last_reads || w.DBG != last_dbg;
		last_writes = writes; last_reads = reads; last_dbg = w.DBG;
		#define WRITE_STROBE(name, bit) w.name = TriState((writes >> bit) & 1u)
		WRITE_STROBE(W4000, 0); WRITE_STROBE(W4001, 1);
		WRITE_STROBE(W4002, 2); WRITE_STROBE(W4003, 3);
		WRITE_STROBE(W4004, 4); WRITE_STROBE(W4005, 5);
		WRITE_STROBE(W4006, 6); WRITE_STROBE(W4007, 7);
		WRITE_STROBE(W4008, 8); WRITE_STROBE(W400A, 10);
		WRITE_STROBE(W400B, 11); WRITE_STROBE(W400C, 12);
		WRITE_STROBE(W400E, 14); WRITE_STROBE(W400F, 15);
		WRITE_STROBE(W4010, 16); WRITE_STROBE(W4011, 17);
		WRITE_STROBE(W4012, 18); WRITE_STROBE(W4013, 19);
		WRITE_STROBE(W4014, 20); WRITE_STROBE(W4015, 21);
		WRITE_STROBE(W4016, 22); WRITE_STROBE(W4017, 23);
		#undef WRITE_STROBE
		w.W401A = TriState(((writes >> 26) & 1u) & (w.DBG & 1u));
		w.n_R4015 = TriState(1u ^ ((reads >> 21) & 1u));
		w.n_R4016 = TriState(1u ^ ((reads >> 22) & 1u));
		w.n_R4017 = TriState(1u ^ ((reads >> 23) & 1u));
		w.n_R4018 = TriState(1u ^ (((reads >> 24) & 1u) & (w.DBG & 1u)));
		w.n_R4019 = TriState(1u ^ (((reads >> 25) & 1u) & (w.DBG & 1u)));
		w.n_R401A = TriState(1u ^ (((reads >> 26) & 1u) & (w.DBG & 1u)));
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

		// Preserve the original driver order, including simultaneous diagnostics.
		if (apu->wire.n_R4018 == TriState::Zero) apu->DB = uint8_t(apu->audio_word);
		if (apu->wire.n_R4019 == TriState::Zero) apu->DB = uint8_t(apu->audio_word >> 8);
		if (apu->wire.n_R401A == TriState::Zero)
			apu->DB = uint8_t((apu->DB & 0x80u) | ((apu->audio_word >> 16) & 0x7fu));
	}
}
