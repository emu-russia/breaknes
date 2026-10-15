// Video Signal Generator

#pragma once

namespace PPUSimFast
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

		DacLevels ext_levels{};		// Externally set level values
		bool use_ext_levels = false;		// true: use ext_levels instead of level values set in the class constructor
		float dac_samples[512]{};
		void RebuildDacSamples();

		uint8_t cc_input = 0, cc_output = 0;
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
		uint8_t rgb_red = 0, rgb_green = 0, rgb_blue = 0;
		BaseLogic::TriState *rgb_output;
		RGB_SEL12x3 red_sel;
		RGB_SEL12x3 green_sel;
		RGB_SEL12x3 blue_sel;

		// RGB PPU Color Matrix
		const size_t color_matrix_inputs = 16;
		const size_t color_matrix_outputs = 12*3;	// R->G->B
		BaseLogic::PLA* color_matrix = nullptr;

		uint8_t phase_state = 0;
		uint8_t get_PhaseInput() const;
		uint8_t get_PhaseOutput() const;
		uint16_t phase_lines = 0;
		uint32_t pal_phase_gate = 0;
		uint32_t pal_colors[32]{};
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

		// The generator body (sim() and sim_Edge()): Phase is the PCLK polarity when it is a
		// compile-time constant (edge calls), -1 when the inputs carry the live lines.
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_T(const VideoInputsT<Phase>& in, VideoOutSignal& vout);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_InputLatches(const VideoInputsT<Phase>& in);
		void sim_InputLatches();	// field-driven entry point for the kernel tests
		void sim_PhaseShifter(BaseLogic::TriState n_CLK, BaseLogic::TriState CLK, BaseLogic::TriState RES);	// n_PCLK from the wire (kernel tests)
		PPUSIM_FAST_EDGE_INLINE void sim_PhaseShifter(BaseLogic::TriState n_CLK, BaseLogic::TriState CLK, BaseLogic::TriState RES, BaseLogic::TriState n_PCLK);
		void sim_ChromaDecoder();
		void sim_ChromaDecoder_PAL();
		void sim_ChromaDecoder_NTSC();
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_OutputLatches(const VideoInputsT<Phase>& in);
		void sim_LumaDecoder(BaseLogic::TriState n_LL0, BaseLogic::TriState n_LL1);
		void sim_Emphasis(BaseLogic::TriState n_TR, BaseLogic::TriState n_TG, BaseLogic::TriState n_TB);
		void sim_CompositeDAC(VideoOutSignal& vout);
		void sim_RGB_DAC(VideoOutSignal& vout);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_nPICTURE(const VideoInputsT<Phase>& in);
		void sim_RAWOutput(VideoOutSignal& vout);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_ColorMatrix(const VideoInputsT<Phase>& in);
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_Select12To3(const VideoInputsT<Phase>& in);

		float PhaseSwing(float v, BaseLogic::TriState sel, float level_from, float level_to);

		bool composite = false;
		bool raw = false;
		bool mode_dirty = true;
		uint8_t dac_context = 0;

		bool noise_enable = false;
		float noise = 0.0f;
		float GetNoise();

		float Clamp(float val, float min, float max);

		void SetupColorMatrix();
		void SetupChromaDecoderPAL();

		void ConvertRAWToRGB_Composite(VideoOutSignal& rawIn, VideoOutSignal& rgbOut);
		void ConvertRAWToRGB_Component(VideoOutSignal& rawIn, VideoOutSignal& rgbOut);

		// v4 lean idle path (track PPU): the settled pixel context folded into masks at arming time,
		// so an idle call is one phase-table step, a few mask tests and one DAC table load.
		struct IdleVideo
		{
			uint32_t pz_mask = 0;			// tested against the phase lines (NTSC) or the PAL phase gate
			uint32_t tint_neg = 0;			// phase lines whose Zero enables emphasis (gated by /POUT)
			uint32_t tint_pos = 0;			// phase lines whose One enables emphasis (PAL /PR, /PG)
			VideoOutSignal rgb{};			// RGB revisions: the settled output sample
			uint16_t dac = 0;				// dac_context
			uint8_t pz_xor = 0, pr_shift = 0, pg_shift = 0, prg_xor = 0;
			bool pal = false;
		} idle{};

	public:

		VideoOut(PPU* parent);
		~VideoOut();

		void sim(VideoOutSignal& vout);
		// The edge call's generator: sim() with the pixel context re-evaluated (pixel_invoked), the
		// phase a compile-time constant and the sampled commands/luma taken from the edge context.
		template<int Phase> PPUSIM_FAST_EDGE_INLINE void sim_Edge(const EdgeContextT<Phase>& c, VideoOutSignal& vout);

		// Idle-call path (v4): the composite generator between pixel edges. Only the phase ring
		// advances; the chroma/emphasis selection and the DAC sample are recomputed from the
		// settled pixel context. Preconditions: IdleEligible(), no pixel edge, not the first call
		// after an edge, RES low.
		void sim_IdleComposite(VideoOutSignal& vout);
		// Lean form of the same evaluation from the context captured by ArmIdle() at the end of
		// the arming call (RGB revisions: the settled sample).
		void ArmIdle(const VideoOutSignal& vout);
		void sim_Idle(VideoOutSignal& vout);
		// Composite noise draws from the process RNG per sample; it stays on the full path so the
		// idle call is call-free (noise is a diagnostic feature, off in every benchmark).
		bool IdleEligible() const { return !raw && !mode_dirty && !noise_enable; }
		bool NoiseEnabled() const { return noise_enable; }

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
