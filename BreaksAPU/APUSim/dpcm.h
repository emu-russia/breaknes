// Differential Pulse-code Modulation (DPCM)

#pragma once

namespace APUSim
{
	class DPCM_LFSRBit
	{
		BaseLogic::DLatch in_latch{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState load, BaseLogic::TriState step, BaseLogic::TriState val, BaseLogic::TriState sin);
		BaseLogic::TriState get_sout();
	};

	class DPCM_SRBit
	{
		BaseLogic::DLatch in_latch{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState clear, BaseLogic::TriState load, BaseLogic::TriState step, BaseLogic::TriState n_val, BaseLogic::TriState sin);
		BaseLogic::TriState get_sout();
	};

	class DpcmChan
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

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
		RegisterBitRes2 ena_ff{};
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

		RegisterBit freq_reg[4]{};
		RegisterBit loop_reg{};
		RegisterBit irq_reg{};
		DPCM_LFSRBit lfsr[9]{};
		RegisterBit scnt_reg[8]{};
		DownCounterBit scnt[12]{};
		CounterBit sbcnt[3]{};
		RegisterBit buf_reg[8]{};
		DPCM_SRBit shift_reg[8]{};
		RegisterBit addr_reg[8]{};
		CounterBit addr_lo[8]{};
		CounterBit addr_hi[7]{};
		RevCounterBit out_cnt[6]{};
		RegisterBit out_reg{};

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
		DpcmChan(APU* parent);
		~DpcmChan();

		void sim();

		uint32_t Get_FreqReg();
		uint32_t Get_SampleReg();
		uint32_t Get_SampleCounter();
		uint32_t Get_SampleBuffer();
		uint32_t Get_SampleBitCounter();
		uint32_t Get_AddressReg();
		uint32_t Get_AddressCounter();
		uint32_t Get_Output();

		void Set_FreqReg(uint32_t value);
		void Set_SampleReg(uint32_t value);
		void Set_SampleCounter(uint32_t value);
		void Set_SampleBuffer(uint32_t value);
		void Set_SampleBitCounter(uint32_t value);
		void Set_AddressReg(uint32_t value);
		void Set_AddressCounter(uint32_t value);
		void Set_Output(uint32_t value);

		bool GetDpcmEnable();
		void SetDpcmEnable(bool enable);
	};
}
