// Differential Pulse-code Modulation (DPCM) Channel

#pragma once

namespace FastAPU
{
	class DpcmChan
	{
		FastAPU* apu = nullptr;

		BaseLogic::TriState LOOPMode = BaseLogic::TriState::X;	// 1: DPCM looped playback
		BaseLogic::TriState n_IRQEN = BaseLogic::TriState::X;	// 0: Enable interrupt from DPCM
		BaseLogic::TriState DSLOAD = BaseLogic::TriState::X;	// Load value into Sample Counter and simultaneously into DPCM Address Counter
		BaseLogic::TriState DSSTEP = BaseLogic::TriState::X;	// Perform Sample Counter decrement and DPCM Address Counter increment simultaneously
		BaseLogic::TriState BLOAD = BaseLogic::TriState::X;		// Load value into Sample Buffer
		BaseLogic::TriState BSTEP = BaseLogic::TriState::X;		// Perform a Sample Buffer bit shift
		BaseLogic::TriState NSTEP = BaseLogic::TriState::X;		// Perform Sample Bit Counter increment
		BaseLogic::TriState DSTEP = BaseLogic::TriState::X;		// Increment/decrement the DPCM Output counter
		BaseLogic::TriState PCM = BaseLogic::TriState::X;		// Load new sample value into Sample Buffer. The signal is active when PHI1 = 0 and the address bus is captured (imitating CPU reading)
		BaseLogic::TriState DOUT = BaseLogic::TriState::X;		// DPCM Out counter has finished counting
		BaseLogic::TriState n_NOUT = BaseLogic::TriState::X;	// 0: Sample Bit Counter has finished counting
		BaseLogic::TriState SOUT = BaseLogic::TriState::X;		// Sample Counter has finished counting
		BaseLogic::TriState DFLOAD = BaseLogic::TriState::X;	// Frequency LFSR finished counting and reloaded itself
		BaseLogic::TriState n_BOUT = BaseLogic::TriState::X;	// The next bit value pushed out of the Sample Buffer shift register (inverted value)
		BaseLogic::TriState Fx[4]{};
		BaseLogic::TriState FR[9]{};
		BaseLogic::TriState Dec1_out[16]{};

		BaseLogic::TriState ED1 = BaseLogic::TriState::X;		// 1: Assert interrupt
		BaseLogic::TriState ED2 = BaseLogic::TriState::X;		// 1: DMC enable ($4015[4])
		BaseLogic::TriState DMC1 = BaseLogic::TriState::X;		// 1: PCM (DMA fetch) done
		BaseLogic::TriState DMC2 = BaseLogic::TriState::X;		// 1: DMC finish
		BaseLogic::TriState CTRL1 = BaseLogic::TriState::X;		// 0: Stop DMA
		BaseLogic::TriState CTRL2 = BaseLogic::TriState::X;		// 0: DMC enable delay

		BaseLogic::TriState ACLK2 = BaseLogic::TriState::X;

		BaseLogic::FF int_ff{};
		BaseLogic::DLatch sout_latch{};
		int ena_ff{};
		BaseLogic::DLatch run_latch1{};
		BaseLogic::DLatch run_latch2{};
		BaseLogic::FF start_ff{};
		BaseLogic::FF rdy_ff{};
		BaseLogic::DLatch en_latch1{};
		BaseLogic::DLatch en_latch2{};
		BaseLogic::DLatch en_latch3{};
		BaseLogic::FF step_ff{};
		BaseLogic::FF stop_ff{};
		BaseLogic::FF pcm_ff{};
		BaseLogic::DLatch dout_latch{};
		BaseLogic::DLatch dstep_latch{};
		BaseLogic::DLatch stop_latch{};
		BaseLogic::DLatch pcm_latch{};
		BaseLogic::DLatch nout_latch{};

		uint8_t freq_reg{};
		int loop_reg{};
		int irq_reg{};
		uint16_t lfsr{};
		uint8_t scnt_reg{};
		uint16_t scnt{};
		uint8_t sbcnt{};
		uint8_t buf_reg{};
		uint8_t shift_reg{};
		uint8_t addr_reg{};
		uint8_t addr_lo{};
		uint8_t addr_hi{};
		uint8_t out_cnt{};
		int out_reg{};

		void sim_ControlReg();
		void sim_IntControl();
		void sim_EnableControl();
		void sim_DMAControl();
		void sim_SampleCounterControl();
		void sim_SampleBufferControl();

		void sim_Decoder1();
		void sim_Decoder2();
		void sim_FreqLFSR();
		void sim_SampleCounterReg();
		void sim_SampleCounter();
		void sim_SampleBitCounter();
		void sim_SampleBuffer();

		void sim_AddressReg();
		void sim_AddressCounter();
		void sim_Output();

		BaseLogic::TriState get_CTRL1();
		BaseLogic::TriState get_CTRL2();
		BaseLogic::TriState get_DMC1();

	public:
		DpcmChan(FastAPU* parent);
		~DpcmChan();

		void sim();
	};
}
