// Video Signal Generator

#pragma once

namespace PPUSim
{
	/// <summary>
	/// Implementation of a single bit of the shift register.
	/// TODO: Switch to generalized implementation.
	/// </summary>
	class VideoOutSRBit
	{
		PPU* ppu = nullptr;

		BaseLogic::DLatch latch1;
		BaseLogic::DLatch latch2;

	public:

		VideoOutSRBit(PPU* parent) { ppu = parent; }

		void sim(BaseLogic::TriState shift_in);

		BaseLogic::TriState getn_Out();
		BaseLogic::TriState getn_ShiftOut();
	};

	class VideoOut
	{
		PPU* ppu = nullptr;

		float LToV[16]{};

		BaseLogic::DLatch cc_latch1[4];
		BaseLogic::DLatch cc_latch2[4];
		BaseLogic::DLatch cc_burst_latch;
		BaseLogic::DLatch sync_latch;		// Previously dac_latch1
		BaseLogic::DLatch pic_out_latch;	// Previously dac_latch2
		BaseLogic::DLatch black_latch;		// Previously dac_latch3
		BaseLogic::DLatch cb_latch;		// Previously dac_latch4

		// For PAL PPU
		BaseLogic::DLatch npicture_latch1;
		BaseLogic::DLatch npicture_latch2;

		VideoOutSRBit *sr[6];		// Individual bits of the shift register for the phase shifter.

		BaseLogic::TriState n_PZ = BaseLogic::TriState::X;
		BaseLogic::TriState n_POUT = BaseLogic::TriState::X;
		BaseLogic::TriState n_LU[4]{};
		BaseLogic::TriState TINT = BaseLogic::TriState::X;

		void sim_DAC(VideoOutSignal& vout);

		bool DebugRandomize = false;
		bool DebugFixed = false;

		void sim_RandomizeChromaLuma();
		void sim_FixedChromaLuma();

		BaseLogic::TriState sim_nPICTURE();

	public:

		VideoOut(PPU* parent);
		~VideoOut();

		// TBD: Once everything is working, add output support for RGB-like PPUs.
		
		void sim(VideoOutSignal& vout);

		void Dbg_RandomizePicture(bool enable);
		void Dbg_FixedPicture(bool enable);
	};
}
