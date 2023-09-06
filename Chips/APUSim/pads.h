// Simulation of APU chip terminals and everything related to them.

#pragma once

namespace APUSim
{
	class BIDIR
	{
		BaseLogic::DLatch in_latch{};		// pad -> terminal circuit
		BaseLogic::DLatch out_latch{};		// terminal circuit -> pad

	public:
		void sim(BaseLogic::TriState pad_in, BaseLogic::TriState to_pad,
			BaseLogic::TriState& from_pad, BaseLogic::TriState& pad_out,
			BaseLogic::TriState rd, BaseLogic::TriState wr);

		void sim_Input(BaseLogic::TriState pad_in, BaseLogic::TriState& from_pad, BaseLogic::TriState rd);
		void sim_Output(BaseLogic::TriState to_pad, BaseLogic::TriState& pad_out, BaseLogic::TriState wr);

		BaseLogic::TriState get_in();
		BaseLogic::TriState get_out();
		void set_in(BaseLogic::TriState val);
		void set_out(BaseLogic::TriState val);
	};

	class Pads
	{
		friend APUSimUnitTest::UnitTest;

		APU* apu = nullptr;

		BIDIR n_irq{};
		BIDIR n_nmi{};
		BIDIR data_bus[8]{};
		BIDIR n_in[2]{};
		BIDIR out[3]{};

		BaseLogic::TriState OUT_Signal[3]{};
		RegisterBit out_reg[3]{};
		BaseLogic::DLatch out_latch[3]{};

		BaseLogic::TriState unused = BaseLogic::TriState::Z;

		void sim_OutReg();

	public:
		Pads(APU* parent);
		~Pads();

		void sim_InputPads(BaseLogic::TriState inputs[]);
		void sim_OutputPads(BaseLogic::TriState outputs[], uint16_t* addr);

		void sim_DataBusInput(uint8_t* data);
		void sim_DataBusOutput(uint8_t* data);

		uint32_t Get_DBOutputLatch();
		uint32_t Get_DBInputLatch();
		uint32_t Get_OutReg();

		void Set_DBOutputLatch(uint32_t value);
		void Set_DBInputLatch(uint32_t value);
		void Set_OutReg(uint32_t value);
	};
}
