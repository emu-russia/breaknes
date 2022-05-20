// Video Signal Generator

#pragma once

namespace PPUSim
{
	/// <summary>
	/// Implementation of a single bit of the shift register.
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

		VideoOutSRBit *sr[6];		// Individual bits of the shift register for the phase shifter.

		BaseLogic::TriState n_PZ = BaseLogic::TriState::X;
		BaseLogic::TriState n_POUT = BaseLogic::TriState::X;
		BaseLogic::TriState n_LU[4]{};
		BaseLogic::TriState TINT = BaseLogic::TriState::X;
		BaseLogic::TriState n_PR = BaseLogic::TriState::X;
		BaseLogic::TriState n_PG = BaseLogic::TriState::X;
		BaseLogic::TriState n_PB = BaseLogic::TriState::X;
		BaseLogic::TriState P[13]{};
		BaseLogic::TriState PZ[13]{};
		BaseLogic::TriState P123 = BaseLogic::TriState::X;

		void sim_PhaseShifter();
		void sim_ChromaDecoder();
		void sim_LumaDecoder();
		void sim_Emphasis();
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

		void GetSignalFeatures(VideoSignalFeatures& features);
	};
}
