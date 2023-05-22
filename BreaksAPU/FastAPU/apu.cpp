#include "pch.h"

namespace FastAPU
{
	FastAPU::FastAPU(M6502Core::M6502* core, APUSim::Revision rev)
	{

	}

	FastAPU::~FastAPU()
	{

	}

	void FastAPU::sim(int inputs[], int outputs[], uint8_t* data, uint16_t* addr, APUSim::AudioOutSignal& AUX)
	{

	}

	void FastAPU::SetRAWOutput(bool enable)
	{

	}

	void FastAPU::SetNormalizedOutput(bool enable)
	{

	}

	size_t FastAPU::GetACLKCounter()
	{

	}

	void FastAPU::ResetACLKCounter()
	{

	}

	size_t FastAPU::GetPHICounter()
	{

	}

	void FastAPU::ResetPHICounter()
	{

	}

	void FastAPU::GetSignalFeatures(APUSim::AudioSignalFeatures& features)
	{

	}

	int FastAPU::GetDBBit(size_t n)
	{

	}

	void FastAPU::SetDBBit(size_t n, int bit_val)
	{

	}

	void FastAPU::sim_CoreIntegration()
	{

	}

	void FastAPU::sim_SoundGenerators()
	{

	}
}
