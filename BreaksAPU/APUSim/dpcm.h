// Differential Pulse-code Modulation (DPCM)

#pragma once

namespace APUSim
{
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

		void sim_IntControl();
		void sim_EnableControl();
		void sim_DMAControl();
		void sim_SampleCounterControl();
		void sim_SampleBufferControl();
		void sim_ControlReg();
		void sim_Decoder();
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
