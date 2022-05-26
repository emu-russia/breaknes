// Video Signal Generator

// TBD: Once everything is working, add output support for RGB-like PPUs.

#pragma once

namespace PPUSim
{
	/// <summary>
	/// Implementation of a single bit of the shift register.
	/// </summary>
	class VideoOutSRBit
	{
		BaseLogic::DLatch in_latch;
		BaseLogic::DLatch out_latch;

	public:
		void sim(BaseLogic::TriState n_shift_in, BaseLogic::TriState n_CLK, BaseLogic::TriState CLK, BaseLogic::TriState RES,
			BaseLogic::TriState& shift_out, BaseLogic::TriState& n_shift_out, BaseLogic::TriState& val);

		BaseLogic::TriState get_Out(BaseLogic::TriState RES);
		BaseLogic::TriState get_ShiftOut();
		BaseLogic::TriState getn_ShiftOut();
	};

	class VideoOut
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		float LToV[16]{};

		BaseLogic::DLatch cc_latch1[4];
		BaseLogic::DLatch cc_latch2[4];
		BaseLogic::DLatch cc_burst_latch;
		BaseLogic::DLatch sync_latch;
		BaseLogic::DLatch pic_out_latch;
		BaseLogic::DLatch black_latch;
		BaseLogic::DLatch cb_latch;

		// For PAL PPU
		BaseLogic::DLatch npicture_latch1;
		BaseLogic::DLatch npicture_latch2;

		VideoOutSRBit sr[6]{};		// Individual bits of the shift register for the phase shifter.

		BaseLogic::TriState n_PZ = BaseLogic::TriState::X;
		BaseLogic::TriState n_POUT = BaseLogic::TriState::X;
		BaseLogic::TriState n_LU[4]{};
		BaseLogic::TriState TINT = BaseLogic::TriState::X;
		BaseLogic::TriState n_PR = BaseLogic::TriState::X;
		BaseLogic::TriState n_PG = BaseLogic::TriState::X;
		BaseLogic::TriState n_PB = BaseLogic::TriState::X;
		BaseLogic::TriState P[13]{};		// Color decoder outputs. The numbering is topological, from left to right starting from 0.
		BaseLogic::TriState PZ[13]{};		// Phase shifter bits corresponding to the Color decoder outputs. The numbering for the Color 0 output is skipped in order to keep the order.
		BaseLogic::TriState PBLACK = BaseLogic::TriState::X;	// Makes colors 14-15 forced "Black".
		BaseLogic::TriState VidOut_n_PICTURE = BaseLogic::TriState::X;	// Local /PICTURE signal

		void sim_InputLatches();
		void sim_PhaseShifter();
		void sim_ChromaDecoder();
		void sim_OutputLatches();
		void sim_LumaDecoder();
		void sim_Emphasis();
		void sim_CompositeDAC(VideoOutSignal& vout);
		void sim_RGB_DAC(VideoOutSignal& vout);
		void sim_nPICTURE();
		void sim_RAWOutput(VideoOutSignal& vout);

		bool DebugRandomize = false;
		bool DebugFixed = false;

		void sim_RandomizeChromaLuma();
		void sim_FixedChromaLuma();

		float PhaseLevel(float v, BaseLogic::TriState sel, size_t level_from, size_t level_to);

		bool composite = false;
		bool raw = false;

		void hsl2rgb(float h, float s, float l, uint8_t& r, uint8_t& g, uint8_t& b);
		float hue2rgb(float p, float q, float t);
		float Clamp(float val, float min, float max);

	public:

		VideoOut(PPU* parent);
		~VideoOut();
		
		void sim(VideoOutSignal& vout);

		void Dbg_RandomizePicture(bool enable);
		void Dbg_FixedPicture(bool enable);

		void GetSignalFeatures(VideoSignalFeatures& features);

		void SetRAWOutput(bool enable);

		void ConvertRAWToRGB(VideoOutSignal& rawIn, VideoOutSignal& rgbOut);
	};
}
