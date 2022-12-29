// Differential Pulse-code Modulation (DPCM)

#pragma once

namespace APUSim
{
	class DPCM_LFSRBit
	{
		BaseLogic::DLatch in_latch{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim();
	};

	class DPCM_SRBit
	{
		BaseLogic::DLatch in_latch{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim();
	};

	class DpcmChan
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

		BaseLogic::TriState LOOPMode;
		BaseLogic::TriState n_IRQEN;
		BaseLogic::TriState DSLOAD;
		BaseLogic::TriState DSSTEP;
		BaseLogic::TriState BLOAD;
		BaseLogic::TriState BSTEP;
		BaseLogic::TriState NSTEP;
		BaseLogic::TriState DSTEP;
		BaseLogic::TriState PCM;
		BaseLogic::TriState DOUT;
		BaseLogic::TriState NOUT;
		BaseLogic::TriState SOUT;
		BaseLogic::TriState DFLOAD;
		BaseLogic::TriState n_BOUT;
		BaseLogic::TriState DPA[8]{};
		BaseLogic::TriState DSC[8]{};
		BaseLogic::TriState Fx[4]{};
		BaseLogic::TriState FR[9]{};
		BaseLogic::TriState Dec1_out[16]{};

		BaseLogic::FF int_ff{};
		BaseLogic::DLatch sout_latch{};
		//RegisterBitRes2 ena_ff;
		BaseLogic::DLatch run_latch1{};
		BaseLogic::DLatch run_latch2{};
		BaseLogic::FF start_ff{};
		BaseLogic::FF rdy_ff{};
		BaseLogic::DLatch fin_latch{};
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

		RegisterBit f_reg[4]{};
		RegisterBit loop_reg{};
		RegisterBit irq_reg{};
		DPCM_LFSRBit lfsr[9]{};
		RegisterBit scnt_reg[8]{};
		DownCounterBit scnt[12]{};
		CounterBit sbcnt[3]{};
		RegisterBit buf_reg{};
		DPCM_SRBit shift_reg[8]{};
		RegisterBit addr_reg[8]{};
		CounterBit addr_lo[8]{};
		CounterBit addr_hi[7]{};
		RevCounterBit out_cnt[6]{};
		RegisterBit out_reg{};

		void sim_IntControl();
		void sim_EnableControl();
		void sim_DMAControl();
		void sim_SampleCounterControl();
		void sim_SampleBufferControl();
		void sim_ControlReg();
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

	public:
		DpcmChan(APU* parent);
		~DpcmChan();

		void sim();
	};
}
