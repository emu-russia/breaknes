// It just so happens that not all methods are purely abstract.
#include "pch.h"

namespace Breaknes
{
	Board::Board(APUSim::Revision apu_rev, PPUSim::Revision ppu_rev, Mappers::ConnectorType p1)
	{
		p1_type = p1;
		pal = new RGB_Triplet[8 * 64];
	}

	Board::~Board()
	{
		delete pal;
	}

	int Board::InsertCartridge(uint8_t* nesImage, size_t nesImageSize)
	{
		Mappers::CartridgeFactory cf(p1_type, nesImage, nesImageSize);
		cart = cf.GetInstance();

		if (!cart)
			return -1;

		if (!cart->Valid())
		{
			delete cart;
			cart = nullptr;

			dbg_hub->DisposeCartDebugInfo();
			dbg_hub->DisposeCartMemMap();

			return -2;
		}

		return 0;
	}

	void Board::EjectCartridge()
	{
		if (cart)
		{
			delete cart;
			cart = nullptr;

			dbg_hub->DisposeCartDebugInfo();
			dbg_hub->DisposeCartMemMap();
		}
	}

	void Board::Reset()
	{
	}

	bool Board::InResetState()
	{
		return false;
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
			// There are 2 resistors (12k and 20k) on the motherboard that equalize the AUX A/B levels and then mix.
			// Although the internal resistance of the AUX A/B terminals inside the APU we counted with the 100 ohm pull-ups -- the above 2 resistors are part of the "Board", so they count here.

			*sample = (aux.normalized.a * 0.4f /* 20k resistor */ + aux.normalized.b /* 12k resistor */) / 2.0f;

			// This is essentially "muting" AUX A, because the level of AUX A at its peak is about 300 mV, and the level of AUX B at its peak is about 1100 mV.
			// Accordingly, if you do just (A+B)/2, the square channels will be "overshoot".
		}
	}

	void Board::LoadNSFData(uint8_t* data, size_t data_size, uint16_t load_address)
	{
	}

	void Board::EnableNSFBanking(bool enable)
	{
	}

	void Board::LoadRegDump(uint8_t* data, size_t data_size)
	{
	}

	void Board::GetApuSignalFeatures(APUSim::AudioSignalFeatures* features)
	{
		APUSim::AudioSignalFeatures feat{};
		apu->GetSignalFeatures(feat);
		*features = feat;
	}

	size_t Board::GetPCLKCounter()
	{
		return ppu->GetPCLKCounter();
	}

	void Board::SampleVideoSignal(PPUSim::VideoOutSignal* sample)
	{
		if (sample != nullptr)
		{
			*sample = vidSample;
		}
	}

	size_t Board::GetHCounter()
	{
		return ppu->GetHCounter();
	}

	size_t Board::GetVCounter()
	{
		return ppu->GetVCounter();
	}

	void Board::RenderAlwaysEnabled(bool enable)
	{
		ppu->Dbg_RenderAlwaysEnabled(enable);
	}

	void Board::GetPpuSignalFeatures(PPUSim::VideoSignalFeatures* features)
	{
		PPUSim::VideoSignalFeatures feat{};
		ppu->GetSignalFeatures(feat);
		*features = feat;
	}

	void Board::ConvertRAWToRGB(uint16_t raw, uint8_t* r, uint8_t* g, uint8_t* b)
	{
		if (!pal_cached)
		{
			PPUSim::VideoOutSignal rawIn{}, rgbOut{};

			// 8 Emphasis bands, each with 64 colors.

			for (size_t n = 0; n < (8 * 64); n++)
			{
				rawIn.RAW.raw = (uint16_t)n;
				ppu->ConvertRAWToRGB(rawIn, rgbOut);
				pal[n].r = rgbOut.RGB.RED;
				pal[n].g = rgbOut.RGB.GREEN;
				pal[n].b = rgbOut.RGB.BLUE;
			}

			pal_cached = true;
		}

		size_t n = raw & 0b111'11'1111;

		*r = pal[n].r;
		*g = pal[n].g;
		*b = pal[n].b;
	}

	void Board::SetRAWColorMode(bool enable)
	{
		ppu->SetRAWOutput(enable);
	}

	void Board::SetOamDecayBehavior(PPUSim::OAMDecayBehavior behavior)
	{
		ppu->SetOamDecayBehavior(behavior);
	}

	void Board::SetNoiseLevel(float volts)
	{
		ppu->SetCompositeNoise(volts);
	}

	void Board::GetAllCoreDebugInfo(M6502Core::DebugInfo* info)
	{
		core->getDebug(info);
	}
}
