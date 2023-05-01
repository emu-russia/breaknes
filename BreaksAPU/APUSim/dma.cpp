// Sprite DMA

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	DMA::DMA(APU* parent)
	{
		apu = parent;
		fast_dma = apu->fast;
	}

	DMA::~DMA()
	{
	}

	void DMA::sim()
	{
		sim_DMA_Control();
		sim_DMA_Address();
	}

	void DMA::sim_DMA_Address()
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState W4014 = apu->wire.W4014;	// Load / SetAddr
		TriState RES = apu->wire.RES;		// Clear

		// Low

		if (fast_dma) {
			if (W4014 == TriState::One) {
				fast_spr_lo = 0;
			}
			if (SPRS == TriState::One) {
				fast_spr_lo++;
			}
			if (RES == TriState::One) {
				fast_spr_lo = 0;
			}
			SPRE = fast_spr_lo == 0xff ? TriState::One : TriState::Zero;
		}
		else
		{
			TriState carry = TriState::One;
			for (size_t n = 0; n < 8; n++)
			{
				carry = spr_lo[n].sim(carry, RES, W4014, SPRS, ACLK1, TriState::Zero);
			}
			SPRE = carry;
		}

		spre_latch.set(SPRE, ACLK1);

		// High

		for (size_t n = 0; n < 8; n++)
		{
			spr_hi[n].sim(ACLK1, W4014, apu->GetDBBit(n));
		}

		// SPR_Addr

		apu->SPR_Addr = 0;

		if (fast_dma) {
			apu->SPR_Addr = fast_spr_lo;
		}

		for (size_t n = 0; n < 8; n++)
		{
			if (!fast_dma)
			{
				apu->SPR_Addr |= ((spr_lo[n].get() == TriState::One ? 1 : 0) << n);
			}
			apu->SPR_Addr |= ((spr_hi[n].get() == TriState::One ? 1 : 0) << (8 + n));
		}
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

		for (size_t n = 0; n < 8; n++)
		{
			spr_buf[n].set (apu->GetDBBit(n), apu->wire.PHI2);
		}

		if (apu->wire.SPR_PPU)
		{
			for (size_t n = 0; n < 8; n++)
			{
				apu->SetDBBit(n, NOT(spr_buf[n].nget()));
			}
		}
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

	uint32_t DMA::Get_DMABuffer()
	{
		TriState val_lo[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = spr_buf[n].get();
		}
		return Pack(val_lo);
	}

	uint32_t DMA::Get_DMAAddress()
	{
		TriState val_lo[8]{};
		TriState val_hi[8]{};
		for (size_t n = 0; n < 8; n++)
		{
			val_lo[n] = spr_lo[n].get();
			val_hi[n] = spr_hi[n].get();
		}
		return (fast_dma ? fast_spr_lo : Pack(val_lo)) | ((uint32_t)Pack(val_hi) << 8);
	}

	void DMA::Set_DMABuffer(uint32_t value)
	{
		TriState val_lo[8]{};
		Unpack(value, val_lo);
		for (size_t n = 0; n < 8; n++)
		{
			spr_buf[n].set(val_lo[n], TriState::One);
		}
	}

	void DMA::Set_DMAAddress(uint32_t value)
	{
		TriState val_lo[8]{};
		TriState val_hi[8]{};
		Unpack(value, val_lo);
		Unpack(value >> 8, val_hi);
		if (fast_dma) {
			fast_spr_lo = (uint8_t)value;
		}
		for (size_t n = 0; n < 8; n++)
		{
			if (!fast_dma) {
				spr_lo[n].set(val_lo[n]);
			}
			spr_hi[n].set(val_hi[n]);
		}
	}

#pragma endregion "Debug"
}
