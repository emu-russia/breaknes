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
		// AUX A

		float maxv = 0.f;

		for (size_t sqb = 0; sqb < 16; sqb++)
		{
			for (size_t sqa = 0; sqa < 16; sqa++)
			{
				float r = AUX_A_Resistance(sqa, sqb);
				float i = Vdd / (r + ExtRes);
				float v = i * ExtRes * 1000.f;
				if (v > maxv)
				{
					maxv = v;
				}
				auxa_mv[(sqb << 4) | sqa] = v;
			}
		}

		for (size_t sqb = 0; sqb < 16; sqb++)
		{
			for (size_t sqa = 0; sqa < 16; sqa++)
			{
				float r = AUX_A_Resistance(sqa, sqb);
				float i = Vdd / (r + ExtRes);
				float v = i * ExtRes * 1000.f;
				auxa_norm[(sqb << 4) | sqa] = v / maxv;
			}
		}

		// AUX B

		maxv = 0.f;

		for (size_t dmc = 0; dmc < 128; dmc++)
		{
			for (size_t rnd = 0; rnd < 16; rnd++)
			{
				for (size_t tri = 0; tri < 16; tri++)
				{
					float r = AUX_B_Resistance(tri, rnd, dmc);
					float i = Vdd / (r + ExtRes);
					float v = i * ExtRes * 1000.f;
					if (v > maxv)
					{
						maxv = v;
					}
					auxb_mv[(dmc << 8) | (rnd << 4) | tri] = v;
				}
			}
		}

		for (size_t dmc = 0; dmc < 128; dmc++)
		{
			for (size_t rnd = 0; rnd < 16; rnd++)
			{
				for (size_t tri = 0; tri < 16; tri++)
				{
					float r = AUX_B_Resistance(tri, rnd, dmc);
					float i = Vdd / (r + ExtRes);
					float v = i * ExtRes * 1000.f;
					auxb_norm[(dmc << 8) | (rnd << 4) | tri] = v / maxv;
				}
			}
		}
	}

	/// <summary>
	/// Return the internal resistance of the AUX A terminal based on the SQA/SQB digital inputs.
	/// </summary>
	/// <param name="sqa"></param>
	/// <param name="sqb"></param>
	/// <returns></returns>
	float DAC::AUX_A_Resistance(size_t sqa, size_t sqb)
	{
		bool sqa_r_exists[] = { false, false, false, false };
		bool sqb_r_exists[] = { false, false, false, false };
		static float stage_reciprocal[] = { 1.f, 2.f, 4.f, 8.f };
		bool any_exists = false;

		// First determine which resistances are present

		for (size_t n = 0; n < 4; n++)
		{
			if ((sqa >> n) & 1)
			{
				sqa_r_exists[n] = true;
				any_exists = true;
			}
			if ((sqb >> n) & 1)
			{
				sqb_r_exists[n] = true;
				any_exists = true;
			}
		}

		// Calculate the total resistance

		float r = 0.f;

		for (size_t n = 0; n < 4; n++)
		{
			if (sqa_r_exists[n])
				r += 1.f / (IntRes / stage_reciprocal[n]);
			if (sqb_r_exists[n])
				r += 1.f / (IntRes / stage_reciprocal[n]);
		}

		return any_exists ? (1.f / r) : IntUnloaded;
	}

	/// <summary>
	/// Return the internal resistance of the AUX B terminal based on the TRI/RND/DMC digital inputs.
	/// </summary>
	/// <param name="tri"></param>
	/// <param name="rnd"></param>
	/// <param name="dmc"></param>
	/// <returns></returns>
	float DAC::AUX_B_Resistance(size_t tri, size_t rnd, size_t dmc)
	{
		bool tri_r_exists[] = { false, false, false, false };
		bool rnd_r_exists[] = { false, false, false, false };
		bool dmc_r_exists[] = { false, false, false, false, false, false, false };
		static float tri_reciprocal[] = { 2.f, 4.f, 8.f, 16.f };
		static float rnd_reciprocal[] = { 1.f, 2.f, 4.f, 8.f };
		static float dmc_reciprocal[] = { 0.5f, 1.f, 2.f, 4.f, 8.f, 16.f, 32.f };
		bool any_exists = false;

		// First determine which resistances are present

		for (size_t n = 0; n < 4; n++)
		{
			if ((tri >> n) & 1)
			{
				tri_r_exists[n] = true;
				any_exists = true;
			}
			if ((rnd >> n) & 1)
			{
				rnd_r_exists[n] = true;
				any_exists = true;
			}
		}
		for (size_t n = 0; n < 7; n++)
		{
			if ((dmc >> n) & 1)
			{
				dmc_r_exists[n] = true;
				any_exists = true;
			}
		}

		// Calculate the total resistance

		float r = 0.f;

		for (size_t n = 0; n < 4; n++)
		{
			if (tri_r_exists[n])
				r += 1.f / (IntRes / tri_reciprocal[n]);
			if (rnd_r_exists[n])
				r += 1.f / (IntRes / rnd_reciprocal[n]);
		}
		for (size_t n = 0; n < 7; n++)
		{
			if (dmc_r_exists[n])
				r += 1.f / (IntRes / dmc_reciprocal[n]);
		}

		return any_exists ? (1.f / r) : IntUnloaded;
	}
}
