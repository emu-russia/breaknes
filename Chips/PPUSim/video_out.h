// Video Signal Generator

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

	class RGB_SEL12x3
	{
		BaseLogic::FF ff[3]{};

	public:
		void sim(BaseLogic::TriState PCLK, BaseLogic::TriState n_Tx, BaseLogic::TriState col_in[12], BaseLogic::TriState lum_in[2]);

		void getOut(BaseLogic::TriState col_out[3]);
	};

	class VideoOut
	{
		friend PPUSimUnitTest::UnitTest;
		PPU* ppu = nullptr;

		float SyncLevel[2]{};
		float BurstLevel[2]{};
		float LumaLevel[4][2]{};
		float EmphasizedLumaLevel[4][2]{};

		DacLevels ext_levels;		// Externally set level values
		bool use_ext_levels;		// true: use ext_levels instead of level values set in the class constructor

		BaseLogic::DLatch cc_latch1[4]{};
		BaseLogic::DLatch cc_latch2[4]{};
		BaseLogic::DLatch cc_burst_latch;
		BaseLogic::DLatch sync_latch;
		BaseLogic::DLatch pic_out_latch;
		BaseLogic::DLatch black_latch;
		BaseLogic::DLatch cb_latch;

		// For PAL PPU
		BaseLogic::DLatch npicture_latch1;
		BaseLogic::DLatch npicture_latch2;
		BaseLogic::DLatch v0_latch;
		const size_t chroma_decoder_inputs = 10;
		const size_t chroma_decoder_outputs = 12 * 2 + 1;	// 12*2 phases + 1 for grays
		BaseLogic::PLA* chroma_decoder = nullptr;

		// For RGB PPU
		BaseLogic::DLatch rgb_sync_latch[3]{};
		BaseLogic::DLatch rgb_red_latch[8]{};
		BaseLogic::DLatch rgb_green_latch[8]{};
		BaseLogic::DLatch rgb_blue_latch[8]{};
		BaseLogic::TriState *rgb_output;
		RGB_SEL12x3 red_sel;
		RGB_SEL12x3 green_sel;
		RGB_SEL12x3 blue_sel;

		// RGB PPU Color Matrix
		const size_t color_matrix_inputs = 16;
		const size_t color_matrix_outputs = 12*3;	// R->G->B
		BaseLogic::PLA* color_matrix = nullptr;

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
		void sim_PhaseShifter(BaseLogic::TriState n_CLK, BaseLogic::TriState CLK, BaseLogic::TriState RES);
		void sim_ChromaDecoder();
		void sim_ChromaDecoder_PAL();
		void sim_ChromaDecoder_NTSC();
		void sim_OutputLatches();
		void sim_LumaDecoder(BaseLogic::TriState n_LL[4]);
		void sim_Emphasis(BaseLogic::TriState n_TR, BaseLogic::TriState n_TG, BaseLogic::TriState n_TB);
		void sim_CompositeDAC(VideoOutSignal& vout);
		void sim_RGB_DAC(VideoOutSignal& vout);
		void sim_nPICTURE();
		void sim_RAWOutput(VideoOutSignal& vout);
		void sim_ColorMatrix();
		void sim_Select12To3();

		float PhaseSwing(float v, BaseLogic::TriState sel, float level_from, float level_to);

		bool composite = false;
		bool raw = false;
		
		bool noise_enable = false;
		float noise = 0.0f;
		float GetNoise();

		float Clamp(float val, float min, float max);

		void SetupColorMatrix();
		void SetupChromaDecoderPAL();

		void ConvertRAWToRGB_Composite(VideoOutSignal& rawIn, VideoOutSignal& rgbOut);
		void ConvertRAWToRGB_Component(VideoOutSignal& rawIn, VideoOutSignal& rgbOut);

	public:

		VideoOut(PPU* parent);
		~VideoOut();
		
		void sim(VideoOutSignal& vout);

		void GetSignalFeatures(VideoSignalFeatures& features);

		void SetRAWOutput(bool enable);

		void ConvertRAWToRGB(VideoOutSignal& rawIn, VideoOutSignal& rgbOut);

		bool IsComposite();

		void SetCompositeNoise(float volts);

		void UseExternalDacLevels(bool use, DacLevels& tab);
	};

	union PALChromaInputs
	{
		struct
		{
			unsigned n_V0 : 1;
			unsigned V0 : 1;
			unsigned n_CC0 : 1;
			unsigned CC0 : 1;
			unsigned n_CC1 : 1;
			unsigned CC1 : 1;
			unsigned n_CC2 : 1;
			unsigned CC2 : 1;
			unsigned n_CC3 : 1;
			unsigned CC3 : 1;
		};
		size_t packed_bits;
	};
}
