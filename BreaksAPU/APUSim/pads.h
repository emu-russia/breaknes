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

		BaseLogic::TriState get_in();
		BaseLogic::TriState get_out();
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

		BaseLogic::TriState OUT[3]{};
		RegisterBit out_reg[3]{};
		BaseLogic::DLatch out_latch[3]{};

		BaseLogic::TriState unused = BaseLogic::TriState::Z;

		void sim_OutReg();

	public:
		Pads(APU* parent);
		~Pads();

		void sim_InputPads(BaseLogic::TriState inputs[], uint8_t* data);
		void sim_OutputPads(BaseLogic::TriState outputs[], uint8_t* data, uint16_t* addr);

		void Debug_Get(APU_Registers* info);
	};
}
