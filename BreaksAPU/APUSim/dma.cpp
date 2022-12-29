// Sprite DMA

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
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
		sim_DMA_Address();
		sim_DMA_Control();
		sim_DMA_Buffer();
		sim_AddressMux();
	}

	void DMA::sim_DMA_Address()
	{
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState W4014 = apu->wire.W4014;	// Load / SetAddr
		TriState RES = apu->wire.RES;		// Clear

		// Low

		TriState Carry = TriState::One;

		for (size_t n = 0; n < 8; n++)
		{
			Carry = spr_lo[n].sim(Carry, RES, W4014, SPRS, n_ACLK, TriState::Zero);
		}

		SPRE = Carry;

		// High

		for (size_t n = 0; n < 8; n++)
		{
			// In this place, for simplicity, the FF based on n_ACLK is not simulated.

			if (W4014 == TriState::One)
			{
				TriState Dx = (apu->DB >> n) & 1 ? TriState::One : TriState::Zero;

				spr_hi[n].set(Dx, TriState::One);
			}
		}

		// SPR_Addr

		apu->SPR_Addr = 0;

		for (size_t n = 0; n < 8; n++)
		{
			apu->SPR_Addr |= ((spr_lo[n].get() == TriState::One ? 1 : 0) << n);
			apu->SPR_Addr |= ((spr_hi[n].get() == TriState::One ? 1 : 0) << (8 + n));
		}
	}

	void DMA::sim_DMA_Control()
	{
		TriState n_ACLK2 = NOT(apu->wire.ACLK);
		TriState n_ACLK = apu->wire.n_ACLK;
		TriState RES = apu->wire.RES;
		TriState W4014 = apu->wire.W4014;
		TriState RUNDMC = apu->wire.RUNDMC;
		TriState PHI1 = apu->wire.PHI1;
		TriState RnW = apu->wire.RnW;
		TriState DMCRDY = apu->wire.DMCRDY;

		// TBD: Check that the circuit is stabilized correctly on the uroboros associated with the NOSPR signal.
		// Also check that the DMA DirToggle goes back and forth properly.

		DMADirToggle.set(NOR(n_ACLK, NOR(n_ACLK2, DMADirToggle.get())));

		spre_latch.set(SPRE, n_ACLK);
		TriState NOSPR = nospr_latch.nget();

		StartDMA.set(NOR(W4014, NOR3(NOT(NOSPR), RES, StartDMA.get())));
		dospr_latch.set(StartDMA.get(), n_ACLK2);
		TriState DOSPR = NOR(dospr_latch.get(), NAND(NOT(PHI1), RnW));

		SPRS = NOR3(NOSPR, RUNDMC, NOT(n_ACLK2));
		StopDMA.set(NOR3(AND(SPRS, spre_latch.get()), RES, NOR(DOSPR, StopDMA.get())));
		nospr_latch.set(StopDMA.get(), n_ACLK);

		apu->wire.RDY = NOT(NAND(NOR(NOT(NOSPR), NOT(StartDMA.get())), DMCRDY));
		apu->wire.SPR_PPU = NOR3(NOSPR, RUNDMC, NOT(DMADirToggle.get()));
		apu->wire.SPR_CPU = NOR3(NOSPR, RUNDMC, DMADirToggle.get());
	}

	void DMA::sim_DMA_Buffer()
	{
		apu->wire.RW = NOR(apu->wire.SPR_PPU, NOT(apu->wire.RnW));

		for (size_t n = 0; n < 8; n++)
		{
			TriState Dx = (apu->DB >> n) & 1 ? TriState::One : TriState::Zero;

			spr_buf[n].set (Dx, apu->wire.PHI2);
		}

		if (apu->wire.SPR_PPU)
		{
			uint8_t val = 0;

			for (size_t n = 0; n < 8; n++)
			{
				val |= (NOT(spr_buf[n].nget()) == TriState::One ? 1 : 0) << n;
			}

			if (apu->DB_Dirty)
			{
				apu->DB &= val;
			}
			else
			{
				apu->DB = val;
				apu->DB_Dirty = true;
			}
		}
	}

	void DMA::sim_AddressMux()
	{
		TriState DMC_AB = NOT(apu->wire.n_DMC_AB);
		TriState CPU_AB = NOR3(apu->wire.SPR_CPU, apu->wire.SPR_PPU, DMC_AB);

		// Only one tristate can be active

		if (DMC_AB)
		{
			apu->Ax = apu->DMC_Addr;
		}
		if (apu->wire.SPR_CPU)
		{
			apu->Ax = apu->SPR_Addr;
		}
		if (apu->wire.SPR_PPU)
		{
			apu->Ax = PPU_Addr;
		}
		if (CPU_AB)
		{
			apu->Ax = apu->CPU_Addr;
		}
	}
}
