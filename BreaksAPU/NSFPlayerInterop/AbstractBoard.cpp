// It just so happens that not all methods are purely abstract.
#include "pch.h"

namespace NSFPlayer
{
	int Board::InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
	{
		// The board does not have a cartridge connector.

		return 0;
	}

	void Board::EjectCartridge()
	{
		// The board does not have a cartridge connector.
	}

	size_t Board::GetACLKCounter()
	{
		return apu->GetACLKCounter();
	}

	size_t Board::GetPHICounter()
	{
		return apu->GetPHICounter();
	}

	void Board::SampleAudioSignal(float* sample)
	{
		// NES/Famicom motherboards have some analog circuitry that acts as a LPF/HPF. Now the sound from the simulator comes without filtering, so there may be some ear-unpleasant harmonics.
		// TBD: Add LPF/HPF

		if (sample != nullptr)
		{
			*sample = (aux.normalized.a * 0.4 /* 20k resistor */ + aux.normalized.b /* 12k resistor */) / 2.0f;
		}
	}

	void Board::GetSignalFeatures(APUSim::AudioSignalFeatures* features)
	{
		APUSim::AudioSignalFeatures feat{};
		apu->GetSignalFeatures(feat);
		*features = feat;
	}
}
