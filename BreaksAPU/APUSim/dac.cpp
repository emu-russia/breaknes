// Obtaining the analog value of the AUX A/B signals from the digital outputs of the generators.

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	DAC::DAC(APU* parent)
	{
		apu = parent;
		gen_DACTabs();
	}

	DAC::~DAC()
	{
	}

	void DAC::sim(AudioOutSignal& AUX)
	{
		sim_DACA(AUX);
		sim_DACB(AUX);
	}

	void DAC::sim_DACA(AudioOutSignal& aout)
	{
		if (raw_mode)
		{
			aout.RAW.sqa = PackNibble(apu->SQA_Out);
			aout.RAW.sqb = PackNibble(apu->SQB_Out);
		}
		else
		{
			size_t v = ((size_t)PackNibble(apu->SQB_Out) << 4) | PackNibble(apu->SQA_Out);
			if (norm_mode)
			{
				aout.normalized.a = auxa_norm[v];
			}
			else
			{
				aout.AUX.a = auxa_mv[v];
			}
		}
	}

	void DAC::sim_DACB(AudioOutSignal& aout)
	{
		if (raw_mode)
		{
			aout.RAW.tri = PackNibble(apu->TRI_Out);
			aout.RAW.rnd = PackNibble(apu->RND_Out);
			aout.RAW.dmc = Pack(apu->DMC_Out);
		}
		else
		{
			size_t v = ((size_t)Pack(apu->DMC_Out) << 8) | ((size_t)PackNibble(apu->RND_Out) << 4) | PackNibble(apu->TRI_Out);
			if (norm_mode)
			{
				aout.normalized.b = auxb_norm[v];
			}
			else
			{
				aout.AUX.b = auxb_mv[v];
			}
		}
	}

	void DAC::SetRAWOutput(bool enable)
	{
		raw_mode = enable;
	}

	void DAC::SetNormalizedOutput(bool enable)
	{
		norm_mode = enable;
	}

	void DAC::gen_DACTabs()
	{
		for (size_t n = 0; n < 0x100; n++)
		{
			auxa_mv[n] = 0.0f;
			auxa_norm[n] = 0.0f;
		}

		for (size_t n = 0; n < 32768; n++)
		{
			auxb_mv[n] = 0.0f;
			auxb_norm[n] = 0.0f;
		}
	}
}
