// OAM DMA

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
{
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
		const std::array<uint8_t, 9> key = { uint8_t(w.ACLK1), uint8_t(w.nACLK2), uint8_t(w.RES),
			uint8_t(w.W4014), uint8_t(w.RUNDMC), uint8_t(w.PHI1), uint8_t(w.RnW),
			uint8_t(w.DMCRDY), uint8_t((w.W4014 & 1) != 0 ? apu->DB : 0) };
		if (!phase.begin(*this, key, apu->mutation_epoch)) return;
		sim_DMA_Control();
		sim_DMA_Address();
		phase.end(*this);
	}

	void DMA::sim_DMA_Address()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4014 = apu->wire.W4014;	// Load / SetAddr
		TriState RES = apu->wire.RES;		// Clear

		spr_lo.sim(RES, W4014, SPRS, ACLK1, 0);
		SPRE = spr_lo.get_carry();
		spre_latch.set(SPRE, ACLK1);
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

	void DMA::sim_DMA_Control()
	{
		TriState ACLK2 = NOT(apu->wire.nACLK2);
		TriState ACLK1 = apu->wire.ACLK1;
		TriState RES = apu->wire.RES;
		TriState W4014 = apu->wire.W4014;
		TriState RUNDMC = apu->wire.RUNDMC;
		TriState PHI1 = apu->wire.PHI1;
		TriState RnW = apu->wire.RnW;
		TriState DMCRDY = apu->wire.DMCRDY;

		DMADirToggle.set(NOR(ACLK1, NOR(ACLK2, DMADirToggle.get())));

		NOSPR = nospr_latch.nget();

		StartDMA.set(NOR3(NOT(NOSPR), RES, NOR(W4014, StartDMA.get())));
		dospr_latch.set(StartDMA.nget(), ACLK2);
		DOSPR = NOR(dospr_latch.get(), NAND(NOT(PHI1), RnW));

		SPRS = NOR3(NOSPR, RUNDMC, NOT(ACLK2));
		StopDMA.set(NOR3(AND(SPRS, spre_latch.get()), RES, NOR(DOSPR, StopDMA.get())));
		nospr_latch.set(StopDMA.get(), ACLK1);

		sprdma_rdy = NOR(NOT(NOSPR), StartDMA.get());
		apu->wire.RDY = AND(sprdma_rdy, DMCRDY);

		apu->wire.SPR_PPU = NOR3(NOSPR, RUNDMC, NOT(DMADirToggle.get()));
		apu->wire.SPR_CPU = NOR3(NOSPR, RUNDMC, DMADirToggle.get());
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
