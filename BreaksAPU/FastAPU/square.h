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

		BaseLogic::TriState n_sum[11]{};
		BaseLogic::TriState S[11]{};
		BaseLogic::TriState SR[3]{};
		BaseLogic::TriState BS[12]{};
		BaseLogic::TriState DEC = BaseLogic::TriState::X;
		BaseLogic::TriState INC = BaseLogic::TriState::X;
		BaseLogic::TriState n_COUT = BaseLogic::TriState::X;
		BaseLogic::TriState SW_UVF = BaseLogic::TriState::X;	// Sweep underflow
		BaseLogic::TriState FCO = BaseLogic::TriState::X;
		BaseLogic::TriState FLOAD = BaseLogic::TriState::X;
		BaseLogic::TriState DO_SWEEP = BaseLogic::TriState::X;
		BaseLogic::TriState SW_OVF = BaseLogic::TriState::X;	// Sweep overflow
		BaseLogic::TriState DUTY = BaseLogic::TriState::X;
		BaseLogic::TriState Vol[4]{};

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

		void sim_FreqReg(BaseLogic::TriState WR2, BaseLogic::TriState WR3);
		void sim_ShiftReg(BaseLogic::TriState WR1);
		void sim_BarrelShifter();
		void sim_Adder();
		void sim_FreqCounter();
		void sim_Sweep(BaseLogic::TriState WR1, BaseLogic::TriState NOSQ);
		void sim_Duty(BaseLogic::TriState WR0, BaseLogic::TriState WR3);
		void sim_Output(BaseLogic::TriState NOSQ, BaseLogic::TriState* SQ_Out);

	public:
		SquareChan(FastAPU* parent, SquareChanCarryIn carry_routing);
		~SquareChan();

		void sim(BaseLogic::TriState WR0, BaseLogic::TriState WR1, BaseLogic::TriState WR2, BaseLogic::TriState WR3, BaseLogic::TriState NOSQ, BaseLogic::TriState* SQ_Out);
		BaseLogic::TriState get_LC();
	};
}
