// H/V Decoder

#include "pch.h"

using namespace BaseLogic;

namespace PPUSimFast
{
	HVDecoder::HVDecoder(PPU* parent)
	{
		ppu = parent;

		// Select the number of outputs. Among the PPUs studied there are some that differ in the number of outputs.

		switch (ppu->rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
			case Revision::RP2C04_0003:
				vpla_outputs = 9;
				break;
			case Revision::RP2C07_0:
			case Revision::UMC_UA6538:
				// An additional output is used for modified EVEN/ODD logic.
				vpla_outputs = 10;
				break;
		}

		// Set matrix

		size_t* h_bitmask = nullptr;
		size_t* v_bitmask = nullptr;

		switch (ppu->rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
			case Revision::RP2C04_0003:
			{
				static size_t RP2C02G_HDecoder[] = {		// Undefined behavior in GCC if you do not specify static
					0b01101010011001010100,
					0b01101010101010101000,
					0b10100110101010100101,
					0b00101010101000000000,
					0b10000000000000000010,
					0b01100110011010010101,
					0b10101001010101010101,
					0b00010101010101010101,
					0b10101000000000000001,
					0b01101000000000000001,
					0b10000000000000000011,
					0b00000000000010100001,
					0b00000000000001010000,
					0b00000000000001100000,
					0b01000110100000000001,
					0b10000000000000000001,
					0b00000000000010010000,
					0b01101010100101011000,
					0b01100110100110101000,
					0b01101010011001010100,
					0b01101001011010101000,
					0b01100110101010010100,
					0b01101001011001101000,
					0b01100110011001101000,
				};

				static size_t RP2C02G_VDecoder[] = {
					0b000101010110010101,
					0b000101010110011010,
					0b011010101010011001,
					0b000101010110101001,
					0b000101010110101001,
					0b101010101010101010,
					0b000101010110101010,
					0b011010101010011001,
					0b011010101010011001,
				};

				h_bitmask = RP2C02G_HDecoder;
				v_bitmask = RP2C02G_VDecoder;
			}
			break;

			case Revision::RP2C07_0:
			{
				static size_t RP2C07_HDecoder[] = {
					0b01101010011001100100,
					0b01101010101010101000,
					0b10100110101010100101,
					0b00101010101000000000,
					0b10000000000000000010,
					0b01100110011010010101,
					0b10101001010101010101,
					0b00010101010101010101,
					0b10101000000000000001,
					0b01101000000000000001,
					0b10000000000000000011,
					0b00000000000010100001,
					0b00000000000001010000,
					0b00000000000001100000,
					0b01000110100000000001,
					0b10000000000000000001,
					0b00000000000010010000,
					0b01101010101010101000,
					0b10101010101001101000,
					0b01101010011001100100,
					0b01101001100101011000,
					0b01100110101010100100,
					0b01101001011010011000,
					0b01100110011001101000,
				};

				static size_t RP2C07_VDecoder[] = {
					0b011010100110101010,
					0b011010101001011001,
					0b101010101010101001,
					0b000101010110101010,
					0b000101010110101001,
					0b101010101010101010,
					0b000101010110101010,
					0b011010010110010101,
					0b011010010110010101,
					0b011010101001101001,
				};

				h_bitmask = RP2C07_HDecoder;
				v_bitmask = RP2C07_VDecoder;
			}
			break;

			case Revision::UMC_UA6538:
			{
				static size_t UMC_UA6538_HDecoder[] = {		// Same as PAL PPU
					0b01101010011001100100,
					0b01101010101010101000,
					0b10100110101010100101,
					0b00101010101000000000,
					0b10000000000000000010,
					0b01100110011010010101,
					0b10101001010101010101,
					0b00010101010101010101,
					0b10101000000000000001,
					0b01101000000000000001,
					0b10000000000000000011,
					0b00000000000010100001,
					0b00000000000001010000,
					0b00000000000001100000,
					0b01000110100000000001,
					0b10000000000000000001,
					0b00000000000010010000,
					0b01101010101010101000,
					0b10101010101001101000,
					0b01101010011001100100,
					0b01101001100101011000,
					0b01100110101010100100,
					0b01101001011010011000,
					0b01100110011001101000,
				};

				static size_t UMC_UA6538_VDecoder[] = {
					0b011010100110101010,
					0b011010101001011001,
					0b101010101010101001,
					0b000101010110101010,
					0b011010011010100101,		// triggered 50 lines later
					0b101010101010101010,
					0b000101010110101010,
					0b011010010110010101,
					0b011010010110010101,
					0b011010101001101001,
				};

				h_bitmask = UMC_UA6538_HDecoder;
				v_bitmask = UMC_UA6538_VDecoder;
			}
			break;

			// TBD: Add PLA for the rest of the PPU studied.
		}

		// SetMatrix reverses each literal's bits before evaluating its NOR cone.
		auto reversed = [](size_t value, unsigned count) {
			uint32_t result = 0;
			for (unsigned n = 0; n < count; n++) result = (result << 1) | ((value >> n) & 1);
			return result;
		};
		uint32_t hm[24]{}, vm[10]{};
		for (unsigned n = 0; n < 24; n++) hm[n] = h_bitmask ? reversed(h_bitmask[n], 20) : 0;
		for (unsigned n = 0; n < vpla_outputs; n++) vm[n] = v_bitmask ? reversed(v_bitmask[n], 18) : 0;
		for (unsigned counter = 0; counter < 512; counter++)
		{
			uint32_t rails = 0;
			for (unsigned n = 0; n < 9; n++) rails |= ((counter >> n) & 1 ? 1U : 2U) << (2 * (8 - n));
			for (unsigned context = 0; context < 4; context++)
			{
				const uint32_t input = rails | (context << 18);
				uint32_t events = 0;
				for (unsigned n = 0; n < 24; n++) if ((input & hm[n]) == 0) events |= 1U << n;
				h_events[counter | (context << 9)] = events;
			}
			for (unsigned n = 0; n < vpla_outputs; n++) if ((rails & vm[n]) == 0) v_events[counter] |= uint16_t(1U << n);
		}
	}

	HVDecoder::~HVDecoder() {}
	uint32_t HVDecoder::sim_HDecoderPacked(TriState VB, TriState BLNK)
	{
		return h_events[ppu->h->get() | (VB == TriState::One ? 512 : 0) | (BLNK == TriState::One ? 1024 : 0)];
	}
	uint16_t HVDecoder::sim_VDecoderPacked() { return v_events[ppu->v->get()]; }
	void HVDecoder::sim_HDecoder(TriState VB, TriState BLNK, TriState** outputs)
	{
		const auto events = sim_HDecoderPacked(VB, BLNK);
		for (unsigned n = 0; n < 24; n++) h_legacy[n] = static_cast<TriState>((events >> n) & 1);
		*outputs = h_legacy;
	}
	void HVDecoder::sim_VDecoder(TriState** outputs)
	{
		const auto events = sim_VDecoderPacked();
		for (unsigned n = 0; n < vpla_outputs; n++) v_legacy[n] = static_cast<TriState>((events >> n) & 1);
		*outputs = v_legacy;
	}
}
