// Video Signal Generator

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	VideoOut::VideoOut(PPU* parent)
	{
		ppu = parent;

		switch (ppu->rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
			case Revision::RP2C07_0:
				LToV[0] = 0.781f;		// Synch
				LToV[1] = 1.000f;		// Colorburst L
				LToV[2] = 1.131f;		// Color 0D
				LToV[3] = 1.300f;		// Color 1D (black)
				LToV[4] = 1.712f;		// Colorburst H
				LToV[5] = 1.743f;		// Color 2D
				LToV[6] = 1.875f;		// Color 00
				LToV[7] = 2.287f;		// Color 10
				LToV[8] = 2.331f;		// Color 3D
				LToV[9] = 2.743f;		// Color 20 / 30
				LToV[10] = 0.746f;		// Emphasis attenuation factor
				composite = true;
				break;

			// TBD: others (RGB, clones)
		}
	}

	VideoOut::~VideoOut()
	{
	}

	void VideoOut::sim(VideoOutSignal& vout)
	{
		if (DebugRandomize)
		{
			sim_RandomizeChromaLuma();
		}
		else if (DebugFixed)
		{
			sim_FixedChromaLuma();
		}

		sim_nPICTURE();
		sim_InputLatches();
		
		if (raw)
		{
			// Raw color can be produced immediately after simulating the input latches.

			sim_RAWOutput(vout);
			return;
		}

		sim_PhaseShifter(ppu->wire.n_CLK, ppu->wire.CLK, ppu->wire.RES);
		sim_ChromaDecoder();
		sim_OutputLatches();
		sim_LumaDecoder(ppu->wire.n_LL);
		sim_Emphasis(ppu->wire.n_TR, ppu->wire.n_TG, ppu->wire.n_TB);

		if (composite)
		{
			sim_CompositeDAC(vout);
		}
		else
		{
			sim_RGB_DAC(vout);
		}
	}

	/// <summary>
	/// Memorize the signal values on all the auxiliary latches.
	/// </summary>
	void VideoOut::sim_InputLatches()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState BURST = ppu->fsm.BURST;
		TriState SYNC = ppu->fsm.SYNC;

		cc_burst_latch.set(BURST, n_PCLK);

		for (size_t n = 0; n < 4; n++)
		{
			cc_latch1[n].set(ppu->wire.n_CC[n], PCLK);
			cc_latch2[n].set(cc_latch1[n].nget(), n_PCLK);
		}

		sync_latch.set(NOT(SYNC), n_PCLK);
		cb_latch.set(BURST, n_PCLK);
	}

	void VideoOut::sim_PhaseShifter(TriState n_CLK, TriState CLK, TriState RES)
	{
		TriState unused{};
		TriState sr0_sout{};

		// TBD: PAL

		// SR Bit5

		auto sr3_sout = NOT(sr[3].getn_ShiftOut());
		sr[5].sim(sr3_sout, n_CLK, CLK, RES, sr0_sout, unused, PZ[0]);
		PZ[1] = NOR(sr0_sout, NOR(sr[4].getn_ShiftOut(), sr3_sout));

		// SR Bit4

		sr[4].sim(PZ[1], n_CLK, CLK, RES, unused, PZ[3], PZ[2]);

		// SR Bit3

		sr[3].sim(PZ[3], n_CLK, CLK, RES, unused, PZ[6], PZ[5]);

		// SR Bit2

		sr[2].sim(PZ[6], n_CLK, CLK, RES, unused, PZ[8], PZ[7]);

		// SR Bit1

		sr[1].sim(PZ[8], n_CLK, CLK, RES, unused, PZ[10], PZ[9]);

		// SR Bit0

		sr[0].sim(PZ[10], n_CLK, CLK, RES, unused, PZ[12], PZ[11]);

		n_PR = PZ[0];
		n_PG = PZ[9];
		n_PB = PZ[5];
	}

	void VideoOut::sim_ChromaDecoder()
	{
		// TBD: PAL

		TriState dmxin[4]{};
		TriState dmxout[16]{};
		dmxin[0] = cc_latch2[0].nget();
		dmxin[1] = cc_latch2[1].nget();
		dmxin[2] = cc_latch2[2].nget();
		dmxin[3] = NOR(cc_latch2[3].get(), cc_burst_latch.get());
		DMX4(dmxin, dmxout);

		PBLACK = NOR3(dmxin[1], dmxin[2], dmxin[3]);

		P[0] = dmxout[3];
		P[1] = dmxout[10];
		P[2] = dmxout[5];
		P[3] = dmxout[12];
		P[4] = dmxout[15];
		P[5] = dmxout[7];
		P[6] = dmxout[14];
		P[7] = dmxout[9];
		P[8] = dmxout[4];
		P[9] = dmxout[11];
		P[10] = dmxout[6];
		P[11] = dmxout[13];
		P[12] = dmxout[8];

		TriState pz[_countof(P)]{};

		for (size_t n = 0; n < _countof(P); n++)
		{
			if (n != 4)
			{
				pz[n] = AND(P[n], NOT(PZ[n]));
			}
			else
			{
				pz[n] = P[n];
			}
		}

		n_PZ = NOR13(pz);
	}

	/// <summary>
	/// Call after Chroma Decoder.
	/// </summary>
	void VideoOut::sim_OutputLatches()
	{
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState BURST = ppu->fsm.BURST;
		TriState SYNC = ppu->fsm.SYNC;
		TriState n_PICTURE = VidOut_n_PICTURE;

		// If /POUT = 1 - then the visible part of the signal is not output at all

		pic_out_latch.set(NOR(PBLACK, n_PICTURE), n_PCLK);
		n_POUT = pic_out_latch.nget();

		// For DAC

		black_latch.set(NOT(NOR3(NOR(PBLACK, n_PICTURE), SYNC, BURST)), n_PCLK);
	}

	void VideoOut::sim_LumaDecoder(TriState n_LL[4])
	{
		n_LU[3] = NOT(NOR3(n_POUT, n_LL[0], n_LL[1]));
		n_LU[2] = NOT(NOR3(n_POUT, NOT(n_LL[0]), n_LL[1]));
		n_LU[1] = NOT(NOR3(n_POUT, n_LL[0], NOT(n_LL[1])));
		n_LU[0] = NOT(NOR3(n_POUT, NOT(n_LL[0]), NOT(n_LL[1])));
	}

	void VideoOut::sim_Emphasis(TriState n_TR, TriState n_TG, TriState n_TB)
	{
		TriState n[3]{};
		n[0] = NOR3(n_POUT, n_PB, n_TB);
		n[1] = NOR3(n_POUT, n_PG, n_TG);
		n[2] = NOR3(n_POUT, n_PR, n_TR);
		TINT = NOT(NOR3(n[0], n[1], n[2]));
	}

	void VideoOut::sim_CompositeDAC(VideoOutSignal& vout)
	{
		TriState tmp = TriState::Zero;
		float v = 3.0f;		// White level

		// Synch Level

		if (sync_latch.nget() == TriState::One)
		{
			v = std::min(v, LToV[0]);
		}

		// Black Level

		if (black_latch.nget() == TriState::One)
		{
			v = std::min(v, LToV[3]);
		}

		// Colorburst phase level

		v = PhaseLevel(v, cb_latch.nget(), 1, 4);

		// Luminance phase levels

		v = PhaseLevel(v, n_LU[0], 2, 6);
		v = PhaseLevel(v, n_LU[1], 3, 7);
		v = PhaseLevel(v, n_LU[2], 5, 9);
		v = PhaseLevel(v, n_LU[3], 8, 9);

		// Emphasis is calculated by scaling voltage with additional resistance.

		if (TINT == TriState::One)
		{
			v *= LToV[10];
		}

		vout.composite = v;
	}

	void VideoOut::sim_RGB_DAC(VideoOutSignal& vout)
	{
		// TBD.

		vout.RGB.r = 0;
		vout.RGB.g = 0;
		vout.RGB.b = 0;
		vout.RGB.syncLevel = 0;
	}

	float VideoOut::PhaseLevel(float v, TriState sel, size_t level_from, size_t level_to)
	{
		auto tmp = NOR(sel, n_PZ);
		if (tmp == TriState::One)
		{
			v = std::min(v, LToV[level_to]);
		}

		tmp = NOR(sel, tmp);
		if (tmp == TriState::One)
		{
			v = std::min(v, LToV[level_from]);
		}
		return v;
	}

	void VideoOut::sim_RAWOutput(VideoOutSignal& vout)
	{
		if (VidOut_n_PICTURE == TriState::Zero)
		{
			vout.RAW.CC0 = ToByte(cc_latch2[0].get());
			vout.RAW.CC1 = ToByte(cc_latch2[1].get());
			vout.RAW.CC2 = ToByte(cc_latch2[2].get());
			vout.RAW.CC3 = ToByte(cc_latch2[3].get());
			vout.RAW.LL0 = ToByte(NOT(ppu->wire.n_LL[0]));
			vout.RAW.LL1 = ToByte(NOT(ppu->wire.n_LL[1]));
			vout.RAW.TR = ToByte(NOT(ppu->wire.n_TR));
			vout.RAW.TG = ToByte(NOT(ppu->wire.n_TG));
			vout.RAW.TB = ToByte(NOT(ppu->wire.n_TB));
			vout.RAW.Sync = ToByte(sync_latch.nget());
		}
		else
		{
			vout.RAW.raw = 0;
		}
	}

	void VideoOutSRBit::sim(TriState n_shift_in, TriState n_CLK, TriState CLK, TriState RES,
		TriState& shift_out, TriState& n_shift_out, TriState& val)
	{
		in_latch.set(n_shift_in, CLK);
		val = get_Out(RES);
		out_latch.set(val, n_CLK);
		shift_out = out_latch.get();
		n_shift_out = out_latch.nget();
	}

	TriState VideoOutSRBit::get_Out(TriState RES)
	{
		return NOR(in_latch.get(), RES);
	}
	
	TriState VideoOutSRBit::get_ShiftOut()
	{
		return out_latch.get();
	}

	TriState VideoOutSRBit::getn_ShiftOut()
	{
		return out_latch.nget();
	}

	/// <summary>
	/// Always produce a random color for PICTURE debugging.
	/// </summary>
	void VideoOut::sim_RandomizeChromaLuma()
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> dist(0, 0x3f); // distribution in range [0, 0x3f]

		auto res = dist(rng);

		ppu->wire.n_CC[0] = (res & 1) != 0 ? TriState::One : TriState::Zero;
		ppu->wire.n_CC[1] = (res & 2) != 0 ? TriState::One : TriState::Zero;
		ppu->wire.n_CC[2] = (res & 4) != 0 ? TriState::One : TriState::Zero;
		ppu->wire.n_CC[3] = (res & 8) != 0 ? TriState::One : TriState::Zero;

		ppu->wire.n_LL[0] = (res & 16) != 0 ? TriState::One : TriState::Zero;
		ppu->wire.n_LL[1] = (res & 32) != 0 ? TriState::One : TriState::Zero;
	}

	/// <summary>
	/// Always output some neutral color for PICTURE debugging.
	/// </summary>
	void VideoOut::sim_FixedChromaLuma()
	{
		ppu->wire.n_CC[0] = TriState::Zero;
		ppu->wire.n_CC[1] = TriState::One;
		ppu->wire.n_CC[2] = TriState::One;
		ppu->wire.n_CC[3] = TriState::Zero;

		ppu->wire.n_LL[0] = TriState::Zero;
		ppu->wire.n_LL[1] = TriState::One;
	}

	void VideoOut::Dbg_RandomizePicture(bool enable)
	{
		DebugRandomize = enable;
	}

	void VideoOut::Dbg_FixedPicture(bool enable)
	{
		DebugFixed = enable;
	}

	/// <summary>
	/// The PAL PPU contains an additional DLatch chain for /PICTURE signal propagation.
	/// </summary>
	/// <returns></returns>
	void VideoOut::sim_nPICTURE()
	{
		TriState PCLK = ppu->wire.PCLK;
		TriState n_PCLK = ppu->wire.n_PCLK;
		TriState n_PICTURE = ppu->fsm.n_PICTURE;

		if (ppu->rev == Revision::RP2C07_0)
		{
			npicture_latch1.set(NOT(n_PICTURE), n_PCLK);
			npicture_latch2.set(npicture_latch1.nget(), PCLK);
			n_PICTURE = npicture_latch2.get();
		}

		VidOut_n_PICTURE = n_PICTURE;
	}

	void VideoOut::GetSignalFeatures(VideoSignalFeatures& features)
	{
		switch (ppu->rev)
		{
			case Revision::RP2C02G:
			case Revision::RP2C02H:
				features.SamplesPerPCLK = 8;
				features.PixelsPerScan = 341;
				features.ScansPerField = 262;
				features.Composite = true;
				features.BlankLevel = 1.3f;
				features.V_pk_pk = 2.0f;
				break;
		}
	}

	void VideoOut::SetRAWOutput(bool enable)
	{
		raw = enable;
	}

	void VideoOut::ConvertRAWToRGB(VideoOutSignal& rawIn, VideoOutSignal& rgbOut)
	{
		VideoOut out(ppu);

		VideoSignalFeatures features{};

		GetSignalFeatures(features);

		out.sync_latch.set(TriState::One, TriState::One);
		out.black_latch.set(TriState::One, TriState::One);
		out.cb_latch.set(TriState::One, TriState::One);
		out.n_POUT = TriState::One;

		// Warm up the phase shifter. It is needed to get the Emphasis complement.

		TriState CLK = TriState::Zero;

		for (size_t n = 0; n < _countof(out.PZ); n++)
		{
			out.sim_PhaseShifter(NOT(CLK), CLK, TriState::Zero);
			CLK = NOT(CLK);
		}

		// Get Color Burst Phase number

		// Get the color phase number and determine the phase shift / hue

		int hue = 7 - rawIn.RAW.raw & 0xf;

		// If necessary, make an Emphasis.

		out.sim_Emphasis(
			NOT(FromByte(rawIn.RAW.TR)),
			NOT(FromByte(rawIn.RAW.TG)),
			NOT(FromByte(rawIn.RAW.TB)));

		// TBD: DEBUG
		out.TINT = TriState::Zero;

		// Get the luminance / saturation

		TriState n_LL[2]{};
		n_LL[0] = NOT(FromByte(rawIn.RAW.LL0));
		n_LL[1] = NOT(FromByte(rawIn.RAW.LL1));
		out.sim_LumaDecoder(n_LL);

		VideoOutSignal bot{}, top{};

		out.n_PZ = TriState::One;
		out.sim_CompositeDAC(bot);

		out.n_PZ = TriState::Zero;
		out.sim_CompositeDAC(top);

		// Based on: https://github.com/DragWx/PalGen/blob/master/palgen.js

		float Y = ((top.composite + bot.composite) / 2) / (features.BlankLevel + features.V_pk_pk);
		float sat = (top.composite - bot.composite) / (features.BlankLevel + features.V_pk_pk);

		float satAdj = 0.7f;
		float con = 1.2f;
		sat *= satAdj * con;

		float hueAdj = -0.25f;
		float irange = 0.599f;
		float qrange = 0.525f;

		float I = sin(((hue / 12.f) * 6.2832f) + 2.5656f + hueAdj) * irange;
		float Q = cos(((hue / 12.f) * 6.2832f) + 2.5656f + hueAdj) * qrange;
		I *= sat;
		Q *= sat;

		float R = Y + (0.9469f * I) + (0.6236f * Q);
		float G = Y - (0.2748f * I) - (0.6357f * Q);
		float B = Y - (1.1085f * I) + (1.709f * Q);

		rgbOut.RGB.r = (uint8_t)(Clamp(R, 0.f, 1.f) * 255);
		rgbOut.RGB.g = (uint8_t)(Clamp(G, 0.f, 1.f) * 255);
		rgbOut.RGB.b = (uint8_t)(Clamp(B, 0.f, 1.f) * 255);
	}

	float VideoOut::Clamp(float val, float min, float max)
	{
		return std::min(max, std::max(val, min));
	}
}
