// Envelope Unit
// Shared between the square channels and the noise generator.

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
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
		TriState n_ACLK = apu->wire.n_ACLK;
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

		envdis_reg.sim(n_ACLK, WR_Reg, apu->GetDBBit(4));
		lc_reg.sim(n_ACLK, WR_Reg, apu->GetDBBit(5));
		for (size_t n = 0; n < 4; n++)
		{
			vol_reg[n].sim(n_ACLK, WR_Reg, apu->GetDBBit(n));
		}

		TriState RCO = TriState::One;
		for (size_t n = 0; n < 4; n++)
		{
			RCO = decay_cnt[n].sim(RCO, RES, RLOAD, RSTEP, n_ACLK, vol_reg[n].get());
		}

		TriState ECO = TriState::One;
		for (size_t n = 0; n < 4; n++)
		{
			ECO = env_cnt[n].sim(ECO, RES, ERES, ESTEP, n_ACLK, EIN);
		}

		EnvReload.set(NOR(NOR(EnvReload.get(), NOR(n_LFO1, erld_latch.get())), WR_LC));
		TriState RELOAD = EnvReload.nget();

		erld_latch.set(EnvReload.get(), n_ACLK);
		reload_latch.set(RELOAD, n_ACLK);
		rco_latch.set(NOR(RCO, RELOAD), n_ACLK);
		eco_latch.set(AND(ECO, NOT(RELOAD)), n_ACLK);

		TriState ENVDIS = envdis_reg.get();
		for (size_t n = 0; n < 4; n++)
		{
			V[n] = MUX(ENVDIS, env_cnt[n].get(), vol_reg[n].get());
		}
	}

	TriState EnvelopeUnit::get_LC()
	{
		return lc_reg.nget();
	}

	void EnvelopeUnit::Debug_Get(uint32_t& VolumeReg, uint32_t& DecayCounter, uint32_t& EnvCounter)
	{
		TriState val[4]{};

		for (size_t n = 0; n < 4; n++)
		{
			val[n] = vol_reg[n].get();
		}
		VolumeReg = PackNibble(val);

		for (size_t n = 0; n < 4; n++)
		{
			val[n] = decay_cnt[n].get();
		}
		DecayCounter = PackNibble(val);

		for (size_t n = 0; n < 4; n++)
		{
			val[n] = env_cnt[n].get();
		}
		EnvCounter = PackNibble(val);
	}
}
