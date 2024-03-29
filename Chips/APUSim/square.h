// Square Channels

#pragma once

namespace APUSim
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

	class FreqRegBit
	{
		BaseLogic::DLatch transp_latch{};
		BaseLogic::DLatch sum_latch{};

	public:
		void sim(BaseLogic::TriState ACLK3, BaseLogic::TriState ACLK1, BaseLogic::TriState WR, BaseLogic::TriState DB_in, BaseLogic::TriState ADDOUT, BaseLogic::TriState n_sum);
		BaseLogic::TriState get_nFx(BaseLogic::TriState ADDOUT);
		BaseLogic::TriState get_Fx(BaseLogic::TriState ADDOUT);
		BaseLogic::TriState get();
		void set(BaseLogic::TriState value);
	};

	class AdderBit
	{
	public:
		void sim(BaseLogic::TriState F, BaseLogic::TriState nF, BaseLogic::TriState S, BaseLogic::TriState nS, BaseLogic::TriState C, BaseLogic::TriState nC, 
			BaseLogic::TriState& cout, BaseLogic::TriState& n_cout, BaseLogic::TriState& n_sum);
	};

	class SquareChan
	{
		friend APUSimUnitTest::UnitTest;
		friend APU;

		APU* apu = nullptr;
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

		RegisterBit dir_reg{};
		FreqRegBit freq_reg[11]{};
		RegisterBit sr_reg[3]{};
		AdderBit adder[11]{};
		BaseLogic::DLatch fco_latch{};
		DownCounterBit freq_cnt[11]{};
		RegisterBit swdis_reg{};
		BaseLogic::DLatch reload_latch{};
		BaseLogic::DLatch sco_latch{};
		BaseLogic::FF reload_ff{};
		RegisterBit sweep_reg[3]{};
		DownCounterBit sweep_cnt[3]{};
		RegisterBit duty_reg[2]{};
		DownCounterBit duty_cnt[3]{};
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
		SquareChan(APU* parent, SquareChanCarryIn carry_routing);
		~SquareChan();

		void sim(BaseLogic::TriState WR0, BaseLogic::TriState WR1, BaseLogic::TriState WR2, BaseLogic::TriState WR3, BaseLogic::TriState NOSQ, BaseLogic::TriState* SQ_Out);
		BaseLogic::TriState get_LC();

		uint32_t Get_FreqReg();
		uint32_t Get_ShiftReg();
		uint32_t Get_FreqCounter();
		uint32_t Get_SweepReg();
		uint32_t Get_SweepCounter();
		uint32_t Get_DutyCounter();

		void Set_FreqReg(uint32_t value);
		void Set_ShiftReg(uint32_t value);
		void Set_FreqCounter(uint32_t value);
		void Set_SweepReg(uint32_t value);
		void Set_SweepCounter(uint32_t value);
		void Set_DutyCounter(uint32_t value);
	};
}
