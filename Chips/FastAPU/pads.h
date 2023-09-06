// Simulation of APU chip terminals and everything related to them.

#pragma once

namespace FastAPU
{
	class Pads
	{
		FastAPU* apu = nullptr;

		int n_irq{};
		int n_nmi{};
		uint8_t data_bus{};
		uint8_t n_in{};
		uint8_t out{};

		uint8_t OUT_Signal{};
		uint8_t out_reg{};
		uint8_t out_latch{};

		void sim_OutReg();

	public:
		Pads(FastAPU* parent);
		~Pads();

		void sim_InputPads(BaseLogic::TriState inputs[]);
		void sim_OutputPads(BaseLogic::TriState outputs[], uint16_t* addr);

		void sim_DataBusInput(uint8_t* data);
		void sim_DataBusOutput(uint8_t* data);
	};
}
