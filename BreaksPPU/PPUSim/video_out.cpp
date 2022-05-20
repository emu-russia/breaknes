// Video Signal Generator

#include "pch.h"

using namespace BaseLogic;

namespace PPUSim
{
	VideoOut::VideoOut(PPU* parent)
	{
		ppu = parent;

		for (size_t n = 0; n < 6; n++)
		{
			sr[n] = new VideoOutSRBit(ppu);
		}

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
				LToV[10] = 0.756f;		// Emphasis attenuation factor
				composite = true;
				break;

			// TBD: others (RGB, clones)
		}
	}

	VideoOut::~VideoOut()
	{
		for (size_t n = 0; n < 6; n++)
		{
			delete sr[n];
		}
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

		sim_PhaseShifter();
		sim_ChromaDecoder();
		sim_OutputLatches();
		sim_LumaDecoder();
		sim_Emphasis();

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

		cc_burst_latch.set(BURST, n_PCLK);

		for (size_t n = 0; n < 4; n++)
		{
			cc_latch1[n].set(ppu->wire.n_CC[n], PCLK);
			cc_latch2[n].set(cc_latch1[n].nget(), n_PCLK);
		}
	}

	void VideoOut::sim_PhaseShifter()
	{
		// TBD: PAL

		PZ[6] = NOT(sr[2]->getn_ShiftOut());
		sr[0]->sim(NOT(PZ[6]));
		PZ[3] = NOT(sr[1]->getn_ShiftOut());
		PZ[1] = NOR(sr[0]->getn_ShiftOut(), NOR(NOT(PZ[6]), PZ[3]));
		sr[1]->sim(PZ[1]);
		PZ[3] = NOT(sr[1]->getn_ShiftOut());
		sr[2]->sim(PZ[3]);
		PZ[6] = NOT(sr[2]->getn_ShiftOut());
		sr[3]->sim(PZ[6]);
		PZ[8] = NOT(sr[3]->getn_ShiftOut());
		sr[4]->sim(PZ[8]);
		PZ[10] = NOT(sr[4]->getn_ShiftOut());
		sr[5]->sim(PZ[10]);
		PZ[12] = NOT(sr[5]->getn_ShiftOut());

		PZ[0] = sr[0]->getn_Out();
		PZ[2] = sr[1]->getn_Out();
		PZ[5] = sr[2]->getn_Out();
		PZ[7] = sr[3]->getn_Out();
		PZ[9] = sr[4]->getn_Out();
		PZ[11] = sr[5]->getn_Out();

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

		P123 = NOR3(dmxin[1], dmxin[2], dmxin[3]);

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

		TriState pz[13]{};

		for (size_t n = 0; n < 13; n++)
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
		TriState HSYNC = ppu->fsm.HSYNC;
		TriState n_PICTURE = VidOut_n_PICTURE;

		// If /POUT = 1 - then the visible part of the signal is not output at all

		pic_out_latch.set(NOR(P123, n_PICTURE), n_PCLK);
		n_POUT = pic_out_latch.nget();

		// For DAC

		sync_latch.set(NOT(HSYNC), n_PCLK);
		black_latch.set(NOT(NOR3(NOR(P123, n_PICTURE), HSYNC, BURST)), n_PCLK);
		cb_latch.set(BURST, n_PCLK);
	}

	void VideoOut::sim_LumaDecoder()
	{
		n_LU[3] = NOT(NOR3(n_POUT, ppu->wire.n_LL[0], ppu->wire.n_LL[1]));
		n_LU[2] = NOT(NOR3(n_POUT, NOT(ppu->wire.n_LL[0]), ppu->wire.n_LL[1]));
		n_LU[1] = NOT(NOR3(n_POUT, ppu->wire.n_LL[0], NOT(ppu->wire.n_LL[1])));
		n_LU[0] = NOT(NOR3(n_POUT, NOT(ppu->wire.n_LL[0]), NOT(ppu->wire.n_LL[1])));
	}

	void VideoOut::sim_Emphasis()
	{
		TriState n[3]{};
		n[0] = NOR3(n_POUT, n_PB, ppu->wire.n_TB);
		n[1] = NOR3(n_POUT, n_PG, ppu->wire.n_TG);
		n[2] = NOR3(n_POUT, n_PR, ppu->wire.n_TR);
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
		}
		else
		{
			vout.RAW.raw = 0;
		}
	}

	void VideoOutSRBit::sim(TriState shift_in)
	{
		latch1.set(shift_in, ppu->wire.CLK);
		latch2.set(getn_Out(), ppu->wire.n_CLK);
	}

	TriState VideoOutSRBit::getn_Out()
	{
		return NOR(latch1.get(), ppu->wire.RES);
	}

	TriState VideoOutSRBit::getn_ShiftOut()
	{
		return latch2.get();
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

	}
}
