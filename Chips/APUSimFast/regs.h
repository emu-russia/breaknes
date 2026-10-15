// Register Decoder

#pragma once

namespace APUSimFast
{
	class RegsDecoder
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

		BaseLogic::TriState pla[29]{};		// Decoder

		BaseLogic::TriState nREGWR = BaseLogic::TriState::X;
		BaseLogic::TriState nREGRD = BaseLogic::TriState::X;

		BaseLogic::DLatch lock_latch{};
		uint64_t decoded_key = 0;
		uint64_t decoded_epoch = 0;
		bool decoded = false;
		// The strobe words and DBG of the last decode, to tell whether a decode changed any strobe.
		uint32_t last_writes = 0, last_reads = 0;
		BaseLogic::TriState last_dbg = BaseLogic::TriState::X;
		bool strobes_changed = false;	// this call's decode changed a strobe (or ran the legacy path)

		void sim_DebugLock();
		void sim_Predecode();
		void sim_Decoder();
		void sim_RegOps();
		void sim_PackedOps();

	public:
		RegsDecoder(APU *parent);
		~RegsDecoder();

		void sim();
		void sim_DebugRegisters();
		bool IsDecoded() const { return decoded; }
		// True when this call's decode changed a register strobe or the debug read qualifier. The
		// DPCM, OAM DMA and clock/frame blocks run before the decoder and consume the strobes one
		// call later, as does the data-bus WR driver, so the next call must be a full call
		// (idle-class arming).
		bool StrobesChanged() const { return strobes_changed; }
	};
}
