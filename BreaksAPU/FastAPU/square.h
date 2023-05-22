// Square Channels

#pragma once

namespace FastAPU
{
	/// <summary>
	/// Input #carry connection option for the adder.
	/// In the real circuit there is no `AdderCarryMode` signal. For Square0 the input n_carry is connected directly to VDD and for Square1 it is connected to INC.
	/// But we cheat a little bit here for convenience by making the connection using multiplexer.
	/// </summary>
	enum SquareChanCarryIn
	{
		Unknown = 0,
		Vdd,
		Inc,
	};

	class SquareChan
	{
		friend FastAPU;

		FastAPU* apu = nullptr;
		SquareChanCarryIn cin_type = SquareChanCarryIn::Unknown;

		uint16_t n_sum{};
		uint16_t S{};
		uint8_t SR{};
		uint16_t BS{};
		int DEC{};
		int INC{};
		int n_COUT{};
		int SW_UVF{};	// Sweep underflow
		int FCO{};
		int FLOAD{};
		int DO_SWEEP{};
		int SW_OVF{};	// Sweep overflow
		int DUTY{};
		uint8_t Vol{};

		int dir_reg{};
		uint16_t freq_reg{};
		uint8_t sr_reg{};
		uint16_t adder{};
		BaseLogic::DLatch fco_latch{};
		uint16_t freq_cnt{};
		int swdis_reg{};
		BaseLogic::DLatch reload_latch{};
		BaseLogic::DLatch sco_latch{};
		BaseLogic::FF reload_ff{};
		uint8_t sweep_reg{};
		uint8_t sweep_cnt{};
		uint8_t duty_reg{};
		uint8_t duty_cnt{};
		BaseLogic::DLatch sqo_latch{};

		EnvelopeUnit* env_unit = nullptr;

		void sim_FreqReg(int WR2, int WR3);
		void sim_ShiftReg(int WR1);
		void sim_BarrelShifter();
		void sim_Adder();
		void sim_FreqCounter();
		void sim_Sweep(int WR1, int NOSQ);
		void sim_Duty(int WR0, int WR3);
		void sim_Output(int NOSQ, int* SQ_Out);

	public:
		SquareChan(FastAPU* parent, SquareChanCarryIn carry_routing);
		~SquareChan();

		void sim(int WR0, int WR1, int WR2, int WR3, int NOSQ, int* SQ_Out);
		int get_LC();
	};
}
