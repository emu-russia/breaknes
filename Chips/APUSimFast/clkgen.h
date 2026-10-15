// Timing Generator

#pragma once

namespace APUSimFast
{
	class SoftCLK_SRBit
	{
		BaseLogic::DLatch in_latch{};
		BaseLogic::DLatch out_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState F1, BaseLogic::TriState F2, BaseLogic::TriState shift_in);

		BaseLogic::TriState get_sout();
		BaseLogic::TriState get_nsout();
	};

	struct CLKGenState
	{
		// PHI1, PHI2, mode register/latch, mask, IRQ/status, Z/first/second.
		uint16_t control = 0;

		BaseLogic::TriState shift_in = BaseLogic::TriState::X;
		BaseLogic::TriState F1 = BaseLogic::TriState::X;
		BaseLogic::TriState F2 = BaseLogic::TriState::X;
		BaseLogic::TriState Z1 = BaseLogic::TriState::X;
		BaseLogic::TriState Z2 = BaseLogic::TriState::X;
		BaseLogic::TriState mode = BaseLogic::TriState::X;
		BaseLogic::TriState n_mode = BaseLogic::TriState::X;
		BaseLogic::TriState pla[6]{};

		uint16_t lfsr_input = 0;
		uint16_t lfsr_output = 0x7fff;

	};

	class CLKGen : private CLKGenState
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

		uint32_t phase_key = 0;
		uint64_t phase_epoch = 0;
		bool phase_stable = false;
		bool clock_changed = false;		// this call's evaluation moved ACLK1 or nACLK2
		uint64_t feedback_state() const
		{
			return uint64_t(control) | (uint64_t(lfsr_input) << 10) | (uint64_t(lfsr_output) << 25);
		}

		void sim_ACLK();
		void sim_SoftCLK_Mode();
		void sim_SoftCLK_Control();
		void sim_SoftCLK_PLA();
		void sim_SoftCLK_LFSR();
		void sim_Evaluate();
		bool IsFixedPoint() const;
#ifdef FASTSIM_SKIP_ORACLE
		void sim_SkipOracle(uint8_t bus);
#endif

	public:
		CLKGen(APU* parent);
		~CLKGen();

		void sim();

		BaseLogic::TriState GetINTFF();
		bool IsStable() const { return phase_stable; }
		// True when this call's evaluation changed ACLK1 or nACLK2. The DPCM and OAM DMA blocks
		// run before this block and consume the clocks one call later, so the next call must be a
		// full call (idle-class arming).
		bool ClockChanged() const { return clock_changed; }
	};
}
