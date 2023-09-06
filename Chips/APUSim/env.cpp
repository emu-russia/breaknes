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

		TriState RCO{};
		TriState ECO{};
		RCO = TriState::One;
		ECO = TriState::One;
		for (size_t n = 0; n < 4; n++)
		{
			vol_reg[n].sim(ACLK1, WR_Reg, apu->GetDBBit(n));
			RCO = decay_cnt[n].sim(RCO, RES, RLOAD, RSTEP, ACLK1, vol_reg[n].get());
			ECO = env_cnt[n].sim(ECO, RES, ERES, ESTEP, ACLK1, EIN);
		}

		EnvReload.set(NOR(NOR(EnvReload.get(), NOR(n_LFO1, erld_latch.get())), WR_LC));
		TriState RELOAD = EnvReload.nget();

		erld_latch.set(EnvReload.get(), ACLK1);
		reload_latch.set(RELOAD, ACLK1);
		rco_latch.set(NOR(RCO, RELOAD), ACLK1);
		eco_latch.set(AND(ECO, NOT(RELOAD)), ACLK1);

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
		VolumeReg = Debug_Get_VolumeReg();
		DecayCounter = Debug_Get_DecayCounter();
		EnvCounter = Debug_Get_EnvCounter();
	}

	uint32_t EnvelopeUnit::Debug_Get_VolumeReg()
	{
		TriState val[4]{};
		for (size_t n = 0; n < 4; n++)
		{
			val[n] = vol_reg[n].get();
		}
		return PackNibble(val);
	}

	uint32_t EnvelopeUnit::Debug_Get_DecayCounter()
	{
		TriState val[4]{};
		for (size_t n = 0; n < 4; n++)
		{
			val[n] = decay_cnt[n].get();
		}
		return PackNibble(val);
	}

	uint32_t EnvelopeUnit::Debug_Get_EnvCounter()
	{
		TriState val[4]{};
		for (size_t n = 0; n < 4; n++)
		{
			val[n] = env_cnt[n].get();
		}
		return PackNibble(val);
	}

	void EnvelopeUnit::Debug_Set_VolumeReg(uint32_t val)
	{
		TriState unpacked[4]{};
		UnpackNibble(val, unpacked);
		for (size_t n = 0; n < 4; n++)
		{
			vol_reg[n].set(unpacked[n]);
		}
	}

	void EnvelopeUnit::Debug_Set_DecayCounter(uint32_t val)
	{
		TriState unpacked[4]{};
		UnpackNibble(val, unpacked);
		for (size_t n = 0; n < 4; n++)
		{
			decay_cnt[n].set(unpacked[n]);
		}
	}

	void EnvelopeUnit::Debug_Set_EnvCounter(uint32_t val)
	{
		TriState unpacked[4]{};
		UnpackNibble(val, unpacked);
		for (size_t n = 0; n < 4; n++)
		{
			env_cnt[n].set(unpacked[n]);
		}
	}
}
