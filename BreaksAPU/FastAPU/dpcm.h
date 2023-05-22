// Differential Pulse-code Modulation (DPCM) Channel

#pragma once

namespace FastAPU
{
	class DpcmChan
	{
		FastAPU* apu = nullptr;

		int LOOPMode{};	// 1: DPCM looped playback
		int n_IRQEN{};	// 0: Enable interrupt from DPCM
		int DSLOAD{};	// Load value into Sample Counter and simultaneously into DPCM Address Counter
		int DSSTEP{};	// Perform Sample Counter decrement and DPCM Address Counter increment simultaneously
		int BLOAD{};	// Load value into Sample Buffer
		int BSTEP{};	// Perform a Sample Buffer bit shift
		int NSTEP{};	// Perform Sample Bit Counter increment
		int DSTEP{};	// Increment/decrement the DPCM Output counter
		int PCM{};		// Load new sample value into Sample Buffer. The signal is active when PHI1 = 0 and the address bus is captured (imitating CPU reading)
		int DOUT{};		// DPCM Out counter has finished counting
		int n_NOUT{};	// 0: Sample Bit Counter has finished counting
		int SOUT{};		// Sample Counter has finished counting
		int DFLOAD{};	// Frequency LFSR finished counting and reloaded itself
		int n_BOUT{};	// The next bit value pushed out of the Sample Buffer shift register (inverted value)
		int Fx[4]{};
		int FR[9]{};
		int Dec1_out[16]{};

		int ED1{};		// 1: Assert interrupt
		int ED2{};		// 1: DMC enable ($4015[4])
		int DMC1{};		// 1: PCM (DMA fetch) done
		int DMC2{};		// 1: DMC finish
		int CTRL1{};	// 0: Stop DMA
		int CTRL2{};	// 0: DMC enable delay

		int ACLK2{};

		int int_ff{};
		int sout_latch{};
		int ena_ff{};
		int run_latch1{};
		int run_latch2{};
		int start_ff{};
		int rdy_ff{};
		int en_latch1{};
		int en_latch2{};
		int en_latch3{};
		int step_ff{};
		int stop_ff{};
		int pcm_ff{};
		int dout_latch{};
		int dstep_latch{};
		int stop_latch{};
		int pcm_latch{};
		int nout_latch{};

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

		int get_CTRL1();
		int get_CTRL2();
		int get_DMC1();

	public:
		DpcmChan(FastAPU* parent);
		~DpcmChan();

		void sim();
	};
}
