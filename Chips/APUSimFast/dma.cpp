// OAM DMA

#include "pch.h"
#include <cstring>
#ifdef FASTSIM_SKIP_ORACLE
#include <cstdlib>
#endif

using namespace BaseLogic;

namespace APUSimFast
{
	static constexpr uint16_t OamDMATransition(unsigned state, unsigned input)
	{
		const unsigned a1 = input & 1u, a2 = (input >> 1) & 1u;
		const unsigned reset = (input >> 2) & 1u, write = (input >> 3) & 1u;
		const unsigned dmc = (input >> 4) & 1u;
		unsigned direction = state & 1u, start = (state >> 1) & 1u;
		unsigned dospr = (state >> 2) & 1u, stop = (state >> 3) & 1u;
		unsigned nospr_bank = (state >> 4) & 1u;
		const unsigned carry = (state >> 5) & 1u;
		direction = (a1 ^ 1u) & (a2 | direction);
		const unsigned nospr = nospr_bank ^ 1u; // Sampled before the bank changes.
		start = nospr & (reset ^ 1u) & (write | start);
		if (a2) dospr = start ^ 1u;
		const unsigned active = (dospr ^ 1u) & (((input >> 5) & 1u) ^ 1u) & ((input >> 6) & 1u);
		const unsigned step = (nospr ^ 1u) & (dmc ^ 1u) & a2;
		stop = ((step & carry) ^ 1u) & (reset ^ 1u) & (active | stop);
		if (a1) nospr_bank = stop;
		const unsigned ready = nospr & (start ^ 1u);
		const unsigned owner = (nospr ^ 1u) & (dmc ^ 1u);
		return uint16_t(direction | (start << 1) | (dospr << 2) | (stop << 3) |
			(nospr_bank << 4) | (carry << 5) | (nospr << 6) | (active << 7) |
			(step << 8) | (ready << 9) | ((ready & ((input >> 7) & 1u)) << 10) |
			((owner & direction) << 11) | ((owner & (direction ^ 1u)) << 12));
	}
	static constexpr std::array<uint16_t, 16384> MakeOamDMATable()
	{
		std::array<uint16_t, 16384> result{};
		for (unsigned i = 0; i < result.size(); ++i) result[i] = OamDMATransition(i & 63u, i >> 6);
		return result;
	}
	#ifdef APUSIMFAST_TABLE_DMA_CONTROL
	static const auto OamDMATable = MakeOamDMATable();
	#endif

	DMA::DMA(APU* parent)
	{
		apu = parent;
	}

	DMA::~DMA()
	{
	}

	void DMA::sim()
	{
		const auto& w = apu->wire;
		// Two bits per TriState: X and Z stay distinct from 0 and 1 and cannot spill into the
		// neighbouring fields (round 2; the binary keys are unchanged up to bit position).
		const uint32_t key = (unsigned(w.ACLK1) & 3u) | ((unsigned(w.nACLK2) & 3u) << 2) |
			((unsigned(w.RES) & 3u) << 4) | ((unsigned(w.W4014) & 3u) << 6) | ((unsigned(w.RUNDMC) & 3u) << 8) |
			((unsigned(w.PHI1) & 3u) << 10) | ((unsigned(w.RnW) & 3u) << 12) | ((unsigned(w.DMCRDY) & 3u) << 14) |
			(uint32_t((w.W4014 & 1) != 0 ? apu->DB : 0) << 16);
		#if !defined(APUSIMFAST_EVALUATE_EVERY_CALL) && !defined(APUSIMFAST_DMA_EVERY_CALL)
		if (!phase.begin(*this, key, apu->mutation_epoch))
		{
#ifdef FASTSIM_SKIP_ORACLE
			sim_SkipOracle();
#endif
			return;
		}
		#endif
		const uint8_t control_before = control;
		const bool binary = sim_DMA_Control();
		sim_DMA_Address();
		#if !defined(APUSIMFAST_EVALUATE_EVERY_CALL) && !defined(APUSIMFAST_DMA_EVERY_CALL)
		// Fixed point (round 2): with binary inputs a replay is the identity when ACLK1 is Zero,
		// or when ACLK1 is One, the counter did not step (SPRS Zero) and the two cells the
		// controller reads before the evaluation rewrites them under ACLK1 (nospr_bank, which
		// takes stop, and the carry bit, which takes SPRE) did not change. The block has no
		// temporary read before it is written. With ACLK1 low those cells and the counter's carry
		// bank are frozen; with ACLK1 high they were rewritten with their own values, and the
		// carry bank is recomputed from an unchanged main bank. The register loads (spr_hi, the
		// main bank's load/clear) rewrite the same values, and the controller equations are
		// absorbing in their own value: direction' = !a1 & (a2 | direction), start' = nospr &
		// !reset & (write | start), stop' = mask & (active | stop), dospr follows start,
		// ready/owner/active/step are functions of those. The W4014 log line fires on an edge of
		// prev_W4014 only, which the first evaluation already consumed.
		const bool fixed_point = binary && (w.ACLK1 == TriState::Zero ||
			(w.ACLK1 == TriState::One && SPRS == TriState::Zero && ((control ^ control_before) & 0x30u) == 0));
		phase.end(*this, fixed_point);
		#endif
	}

#ifdef FASTSIM_SKIP_ORACLE
	// A skipped evaluation must be a replay: re-run it and abort if the block, the wires or the
	// OAM address would have changed.
	void DMA::sim_SkipOracle()
	{
		unsigned char before_block[sizeof(DMA)];
		std::memcpy(before_block, this, sizeof(DMA));
		const auto before_wires = apu->wire;
		const uint16_t spr = apu->SPR_Addr;
		sim_DMA_Control();
		sim_DMA_Address();
		const bool same = std::memcmp(before_block, this, sizeof(DMA)) == 0 &&
			std::memcmp(&before_wires, &apu->wire, sizeof(before_wires)) == 0 && spr == apu->SPR_Addr;
		if (!same)
		{
			std::fprintf(stderr, "FASTSIM_SKIP_ORACLE: OAM DMA skipped evaluation was not a replay (SPR %04x->%04x)\n", spr, apu->SPR_Addr);
			std::abort();
		}
	}
#endif

	void DMA::sim_DMA_Address()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4014 = apu->wire.W4014;	// Load / SetAddr
		TriState RES = apu->wire.RES;		// Clear

		spr_lo.sim(RES, W4014, SPRS, ACLK1, 0);
		SPRE = spr_lo.get_carry();
		if (ACLK1 == TriState::One) control = uint8_t((control & 31u) | (unsigned(SPRE) << 5));
		// RegisterBit's MUX tests low bits, unlike the counter latch enable.
		if ((ACLK1 & 1) == 0 && (W4014 & 1) != 0) spr_hi = apu->DB;
		apu->SPR_Addr = uint16_t((uint16_t(spr_hi) << 8) | spr_lo.get());

		// OAM DMA start ($4014 write): one line per write strobe.

		if (W4014 == TriState::One && prev_W4014 != TriState::One)
		{
			LOG_APU(Cat_DMA, "OAM DMA start: $%02X00", (int)((apu->SPR_Addr >> 8) & 0xFF));
		}
		prev_W4014 = W4014;
	}

	bool DMA::sim_DMA_Control()
	{
		const auto& w = apu->wire;
		const unsigned input = unsigned(w.ACLK1) | (unsigned(NOT(w.nACLK2)) << 1) |
			(unsigned(w.RES) << 2) | (unsigned(w.W4014) << 3) | (unsigned(w.RUNDMC) << 4) |
			(unsigned(w.PHI1) << 5) | (unsigned(w.RnW) << 6) | (unsigned(w.DMCRDY) << 7);
		if (input >= 256 || w.ACLK1 > TriState::One)
		{
			const TriState a2 = NOT(w.nACLK2);
			TriState direction = TriState(control & 1u), start = TriState((control >> 1) & 1u);
			TriState active = TriState((control >> 2) & 1u), stop = TriState((control >> 3) & 1u);
			TriState nospr_bank = TriState((control >> 4) & 1u);
			const TriState carry = TriState((control >> 5) & 1u);
			direction = NOR(w.ACLK1, NOR(a2, direction));
			NOSPR = NOT(nospr_bank);
			start = NOR3(NOT(NOSPR), w.RES, NOR(w.W4014, start));
			if (a2 == TriState::One) active = NOT(start);
			DOSPR = NOR(active, NAND(NOT(w.PHI1), w.RnW));
			SPRS = NOR3(NOSPR, w.RUNDMC, NOT(a2));
			stop = NOR3(AND(SPRS, carry), w.RES, NOR(DOSPR, stop));
			if (w.ACLK1 == TriState::One) nospr_bank = stop;
			control = uint8_t(unsigned(direction) | (unsigned(start) << 1) | (unsigned(active) << 2) |
				(unsigned(stop) << 3) | (unsigned(nospr_bank) << 4) | (unsigned(carry) << 5));
			sprdma_rdy = NOR(NOT(NOSPR), start);
			apu->wire.RDY = AND(sprdma_rdy, w.DMCRDY);
			apu->wire.SPR_PPU = NOR3(NOSPR, w.RUNDMC, NOT(direction));
			apu->wire.SPR_CPU = NOR3(NOSPR, w.RUNDMC, direction);
			return false;
		}
		#ifndef APUSIMFAST_TABLE_DMA_CONTROL
		const uint16_t result = OamDMATransition(control, input);
		#else
		const uint16_t result = OamDMATable[(input << 6) | control];
		#endif
		control = result & 63u;
		NOSPR = TriState((result >> 6) & 1u);
		DOSPR = TriState((result >> 7) & 1u);
		SPRS = TriState((result >> 8) & 1u);
		sprdma_rdy = TriState((result >> 9) & 1u);
		apu->wire.RDY = TriState((result >> 10) & 1u);
		apu->wire.SPR_PPU = TriState((result >> 11) & 1u);
		apu->wire.SPR_CPU = TriState((result >> 12) & 1u);
		return true;
	}

	void DMA::sim_DMA_Buffer()
	{
		apu->wire.RW = NOR(apu->wire.SPR_PPU, NOT(apu->wire.RnW));
		apu->wire.RD = apu->wire.RW;
		apu->wire.WR = NAND3(apu->wire.n_R4015, apu->wire.n_DBGRD, apu->wire.RW);

		if (apu->wire.PHI2 == TriState::One) spr_buf = apu->DB;
		if (apu->wire.SPR_PPU) apu->DB = spr_buf;
	}

	void DMA::sim_AddressMux()
	{
		TriState DMC_AB = NOT(apu->wire.n_DMC_AB);
		TriState CPU_AB = NOR3(apu->wire.SPR_CPU, apu->wire.SPR_PPU, DMC_AB);

		// Only one tristate can be active

		if (DMC_AB == TriState::One)
		{
			apu->Ax = apu->DMC_Addr;
		}
		if (apu->wire.SPR_CPU == TriState::One)
		{
			apu->Ax = apu->SPR_Addr;
		}
		if (apu->wire.SPR_PPU == TriState::One)
		{
			apu->Ax = PPU_Addr;
		}
		if (CPU_AB == TriState::One)
		{
			apu->Ax = apu->CPU_Addr;
		}
	}

#pragma region "Debug"

	uint32_t DMA::Get_DMABuffer() { return spr_buf; }
	uint32_t DMA::Get_DMAAddress() { return spr_lo.get() | (uint32_t(spr_hi) << 8); }
	void DMA::Set_DMABuffer(uint32_t value) { spr_buf = uint8_t(value); }
	void DMA::Set_DMAAddress(uint32_t value)
	{
		spr_lo.set(uint16_t(value));
		spr_hi = uint8_t(value >> 8);
	}

#pragma endregion "Debug"
}
