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
		if (ppu_regdump)
			delete ppu_regdump;
		if (apu_regdump)
			delete apu_regdump;
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

	void Board::LoadRegDump(uint8_t* data, size_t data_size)
	{
	}

	void Board::EnablePpuRegDump(bool enable, char* regdump_dir)
	{
		if (enable) {

			char filename[0x200]{};
			sprintf(filename, "%s/ppu.regdump", regdump_dir);

			if (ppu_regdump) {
				delete ppu_regdump;
				ppu_regdump = nullptr;
			}
			ppu_regdump = new RegDumper(GetPHICounter(), filename);
			prev_phi_counter_for_ppuregdump = GetPHICounter();
			phi_flush_counter_ppuregdump = 0;

			printf("PPU regdump enabled to file: %s\n", filename);
		}
		else {
			if (ppu_regdump) {
				delete ppu_regdump;
				ppu_regdump = nullptr;
			}

			printf("PPU regdump disabled\n");
		}
	}

	void Board::EnableApuRegDump(bool enable, char* regdump_dir)
	{
		if (enable) {

			char filename[0x200]{};
			sprintf(filename, "%s/apu.regdump", regdump_dir);

			if (apu_regdump) {
				delete apu_regdump;
				apu_regdump = nullptr;
			}
			apu_regdump = new RegDumper(GetPHICounter(), filename);
			prev_phi_counter_for_apuregdump = GetPHICounter();
			phi_flush_counter_apuregdump = 0;

			printf("APU regdump enabled to file: %s\n", filename);
		}
		else {
			if (apu_regdump) {
				delete apu_regdump;
				apu_regdump = nullptr;
			}

			printf("APU regdump disabled\n");
		}
	}

	/// <summary>
	/// Check that the 6502 core is accessing the mapped APU/PPU registers and add an entry to regdump if necessary.
	/// </summary>
	void Board::TreatCoreForRegdump(uint16_t addr_bus, uint8_t data_bus, BaseLogic::TriState m2, BaseLogic::TriState rnw)
	{
		if (apu_regdump && (addr_bus & ~MappedAPUMask) == MappedAPUBase) {

			uint64_t phi_now = GetPHICounter();
			if (prev_phi_counter_for_apuregdump != phi_now) {

				if (rnw)
					apu_regdump->LogRegRead(phi_now, addr_bus & MappedAPUMask);
				else
					apu_regdump->LogRegWrite(phi_now, addr_bus & MappedAPUMask, data_bus);
		
				phi_flush_counter_apuregdump += (phi_now - prev_phi_counter_for_apuregdump);
				if (phi_flush_counter_apuregdump >= JustAboutOneSecond) {
					phi_flush_counter_apuregdump = 0;
					apu_regdump->Flush();
				}

				prev_phi_counter_for_apuregdump = phi_now;
			}
		}
		if (ppu_regdump && (addr_bus & ~MappedPPUMask) == MappedPPUBase) {

			uint64_t phi_now = GetPHICounter();
			if (prev_phi_counter_for_ppuregdump != phi_now) {

				if (rnw)
					ppu_regdump->LogRegRead(phi_now, addr_bus & MappedPPUMask);
				else
					ppu_regdump->LogRegWrite(phi_now, addr_bus & MappedPPUMask, data_bus);

				phi_flush_counter_ppuregdump += (phi_now - prev_phi_counter_for_ppuregdump);
				if (phi_flush_counter_ppuregdump >= JustAboutOneSecond) {
					phi_flush_counter_ppuregdump = 0;
					ppu_regdump->Flush();
				}

				prev_phi_counter_for_ppuregdump = phi_now;
			}
		}
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
