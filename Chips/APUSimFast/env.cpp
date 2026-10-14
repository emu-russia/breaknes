// Envelope Unit
// Shared between the square channels and the noise generator.

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
{
	EnvelopeUnit::EnvelopeUnit(APU* parent)
	{
		apu = parent;
	}

	EnvelopeUnit::~EnvelopeUnit()
	{
	}

	void EnvelopeUnit::sim(TriState V[4], TriState WR_Reg, TriState WR_LC)
	{
		TriState ACLK1 = apu->wire.ACLK1;
		TriState n_LFO1 = apu->wire.n_LFO1;
		TriState RES = apu->wire.RES;

		// Controls

		TriState RLOAD = NOR(n_LFO1, rco_latch.get());
		TriState RSTEP = NOR(n_LFO1, rco_latch.nget());
		TriState EIN = NAND(eco_latch.get(), get_LC());
		TriState eco_reload = NOR(eco_latch.get(), reload_latch.get());
		TriState ESTEP = NOR(NOT(RLOAD), NOT(eco_reload));
		TriState ERES = NOR(NOT(RLOAD), eco_reload);

		// Reg/counters

		envdis_reg.sim(ACLK1, WR_Reg, apu->GetDBBit(4));
		lc_reg.sim(ACLK1, WR_Reg, apu->GetDBBit(5));

		if ((ACLK1 & 1) == 0 && (WR_Reg & 1) != 0) vol_reg = apu->DB & 15;
		decay_cnt.sim(RES, RLOAD, RSTEP, ACLK1, vol_reg);
		env_cnt.sim(RES, ERES, ESTEP, ACLK1, EIN == TriState::One ? 15 : 0);
		const TriState RCO = decay_cnt.get_carry();
		const TriState ECO = env_cnt.get_carry();

		EnvReload.set(NOR(NOR(EnvReload.get(), NOR(n_LFO1, erld_latch.get())), WR_LC));
		TriState RELOAD = EnvReload.nget();

		erld_latch.set(EnvReload.get(), ACLK1);
		reload_latch.set(RELOAD, ACLK1);
		rco_latch.set(NOR(RCO, RELOAD), ACLK1);
		eco_latch.set(AND(ECO, NOT(RELOAD)), ACLK1);

		TriState ENVDIS = envdis_reg.get();
		for (size_t n = 0; n < 4; n++)
		{
			V[n] = (((ENVDIS & 1) != 0 ? vol_reg : env_cnt.get()) >> n) & 1 ? TriState::One : TriState::Zero;
		}
	}

	TriState EnvelopeUnit::get_LC()
	{
		return lc_reg.nget();
	}

	void EnvelopeUnit::Debug_Get(uint32_t& VolumeReg, uint32_t& DecayCounter, uint32_t& EnvCounter)
	{
		VolumeReg = Debug_Get_VolumeReg();
		DecayCounter = Debug_Get_DecayCounter();
		EnvCounter = Debug_Get_EnvCounter();
	}

	uint32_t EnvelopeUnit::Debug_Get_VolumeReg() { return vol_reg; }
	uint32_t EnvelopeUnit::Debug_Get_DecayCounter() { return decay_cnt.get(); }
	uint32_t EnvelopeUnit::Debug_Get_EnvCounter() { return env_cnt.get(); }
	void EnvelopeUnit::Debug_Set_VolumeReg(uint32_t val) { vol_reg = val & 15; }
	void EnvelopeUnit::Debug_Set_DecayCounter(uint32_t val) { decay_cnt.set(uint16_t(val)); }
	void EnvelopeUnit::Debug_Set_EnvCounter(uint32_t val) { env_cnt.set(uint16_t(val)); }
}
