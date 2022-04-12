// H/V Decoder

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	HVDecoder::HVDecoder(PPU* parent)
	{
		char hplaName[0x20] = { 0 };
		char vplaName[0x20] = { 0 };

		ppu = parent;

		// Select the number of outputs. Among the PPUs studied there are some that differ in the number of outputs.

		switch (ppu->rev)
		{
			case Revision::RP2C02G:
				vpla_outputs = 9;
				break;
			case Revision::RP2C07_0:
				// An additional output is used for modified EVEN/ODD logic.
				vpla_outputs = 10;
				break;
		}

		// Generate PLA dump name according to the PPU revision.

		sprintf_s(hplaName, sizeof(hplaName), "HPLA_%s.bin", ppu->RevisionToStr(ppu->rev));
		sprintf_s(vplaName, sizeof(vplaName), "VPLA_%s.bin", ppu->RevisionToStr(ppu->rev));

		// Create PLA instances

		hpla = new PLA(hpla_inputs, hpla_outputs, hplaName);
		vpla = new PLA(vpla_inputs, vpla_outputs, vplaName);

		// Set matrix
		// https://github.com/emu-russia/breaks/blob/master/BreakingNESWiki_DeepL/PPU/hv_dec.md

		size_t* h_bitmask = nullptr;
		size_t* v_bitmask = nullptr;

		switch (ppu->rev)
		{
			case Revision::RP2C02G:
			{
				size_t RP2C02G_HDecoder[] = {
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

				size_t RP2C02G_VDecoder[] = {
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

			// TBD: When everything is working, add PLA for the rest of the PPU studied.
		}

		if (h_bitmask != nullptr)
		{
			hpla->SetMatrix(h_bitmask);
		}

		if (v_bitmask != nullptr)
		{
			vpla->SetMatrix(v_bitmask);
		}
	}

	HVDecoder::~HVDecoder()
	{
		delete hpla;
		delete vpla;
	}

	void HVDecoder::sim_HDecoder(TriState VB, TriState BLNK, TriState** outputs)
	{
		HDecoderInput input{};

		input.H8 = ppu->h->getBit(8);
		input.n_H8 = NOT(ppu->h->getBit(8));
		input.H7 = ppu->h->getBit(7);
		input.n_H7 = NOT(ppu->h->getBit(7));
		input.H6 = ppu->h->getBit(6);
		input.n_H6 = NOT(ppu->h->getBit(6));
		input.H5 = ppu->h->getBit(5);
		input.n_H5 = NOT(ppu->h->getBit(5));
		input.H4 = ppu->h->getBit(4);
		input.n_H4 = NOT(ppu->h->getBit(4));
		input.H3 = ppu->h->getBit(3);
		input.n_H3 = NOT(ppu->h->getBit(3));
		input.H2 = ppu->h->getBit(2);
		input.n_H2 = NOT(ppu->h->getBit(2));
		input.H1 = ppu->h->getBit(1);
		input.n_H1 = NOT(ppu->h->getBit(1));
		input.H0 = ppu->h->getBit(0);
		input.n_H0 = NOT(ppu->h->getBit(0));

		input.VB = VB == TriState::One ? 1 : 0;
		input.BLNK = BLNK == TriState::One ? 1 : 0;

		hpla->sim(input.packed_bits, outputs);
	}

	void HVDecoder::sim_VDecoder(TriState** outputs)
	{
		VDecoderInput input{};

		input.V8 = ppu->v->getBit(8);
		input.n_V8 = NOT(ppu->v->getBit(8));
		input.V7 = ppu->v->getBit(7);
		input.n_V7 = NOT(ppu->v->getBit(7));
		input.V6 = ppu->v->getBit(6);
		input.n_V6 = NOT(ppu->v->getBit(6));
		input.V5 = ppu->v->getBit(5);
		input.n_V5 = NOT(ppu->v->getBit(5));
		input.V4 = ppu->v->getBit(4);
		input.n_V4 = NOT(ppu->v->getBit(4));
		input.V3 = ppu->v->getBit(3);
		input.n_V3 = NOT(ppu->v->getBit(3));
		input.V2 = ppu->v->getBit(2);
		input.n_V2 = NOT(ppu->v->getBit(2));
		input.V1 = ppu->v->getBit(1);
		input.n_V1 = NOT(ppu->v->getBit(1));
		input.V0 = ppu->v->getBit(0);
		input.n_V0 = NOT(ppu->v->getBit(0));

		vpla->sim(input.packed_bits, outputs);
	}
}
