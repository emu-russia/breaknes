// Obtaining the analog value of the AUX A/B signals from the digital outputs of the generators.

#include "pch.h"

using namespace BaseLogic;

namespace APUSimFast
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
		const uint32_t word = apu->audio_word;
		if (raw_mode)
		{
			// Write only the five active RAW fields, preserving union tail bytes.
			AUX.RAW.sqa = word & 15;
			AUX.RAW.sqb = (word >> 4) & 15;
			AUX.RAW.tri = (word >> 8) & 15;
			AUX.RAW.rnd = (word >> 12) & 15;
			AUX.RAW.dmc = (word >> 16) & 127;
			return;
		}
		if (!cached || cached_word != word)
		{
			cached_a = norm_mode ? auxa_norm[word & 255] : auxa_mv[word & 255];
			cached_b = norm_mode ? auxb_norm[word >> 8] : auxb_mv[word >> 8];
			cached_word = word;
			cached = true;
		}
		// The caller may supply a different AudioOutSignal object each call.
		AUX.AUX.a = cached_a;
		AUX.AUX.b = cached_b;
	}

	// Output-mode changes also disarm the idle class, which re-drives the published sample.
	void DAC::SetRAWOutput(bool enable) { raw_mode = enable; cached = false; apu->idle_armed = false; }
	void DAC::SetNormalizedOutput(bool enable) { norm_mode = enable; cached = false; apu->idle_armed = false; }

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
