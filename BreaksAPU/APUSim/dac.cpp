// Obtaining the analog value of the AUX A/B signals from the digital outputs of the generators.

#include "pch.h"

using namespace BaseLogic;

namespace APUSim
{
	DAC::DAC(APU* parent)
	{
		apu = parent;
	}

	DAC::~DAC()
	{
	}

	void DAC::sim(float* AUX_A, float* AUX_B)
	{
		*AUX_A = 0.0f;
		*AUX_B = 0.0f;
	}
}
