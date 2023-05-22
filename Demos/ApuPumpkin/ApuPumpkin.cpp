// Compare the time of the simulated APU with the real time.

#include "pch.h"

using namespace BaseLogic;

#define ONE_SECOND 21'477272		// 2A03-G

// How long to simulate

#ifdef _DEBUG
#define SIMULATE_MSEC 10
#else
#define SIMULATE_MSEC 1000
#endif

M6502Core::M6502* core = nullptr;
#if FAST_APU
FastAPU::FastAPU* apu = nullptr;
#else
APUSim::APU* apu = nullptr;
#endif

static bool ApuMegaCyclesTest(size_t desired_clk)
{
	char text[0x100]{};
	TriState CLK = TriState::Zero;

	uint8_t data_bus = 0;
	uint16_t addr_bus = 0;
	APUSim::AudioOutSignal aux{};

	TriState inputs[(size_t)APUSim::APU_Input::Max]{};
	TriState outputs[(size_t)APUSim::APU_Output::Max]{};

	inputs[(size_t)APUSim::APU_Input::DBG] = TriState::Zero;
	inputs[(size_t)APUSim::APU_Input::n_IRQ] = TriState::One;
	inputs[(size_t)APUSim::APU_Input::n_NMI] = TriState::One;

	// Reset APU

	inputs[(size_t)APUSim::APU_Input::n_RES] = TriState::Zero;

	for (size_t n = 0; n < 256; n++)
	{
		data_bus = 0;
		inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
		apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);
		CLK = NOT(CLK);

		data_bus = 0;
		inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
		apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);
		CLK = NOT(CLK);
	}

	// Continue

	inputs[(size_t)APUSim::APU_Input::n_RES] = TriState::One;

	auto stamp1 = GetTickCount64();

	for (size_t n = 0; n < desired_clk; n++)
	{
		data_bus = 0;
		inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
		apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);
		CLK = NOT(CLK);

		data_bus = 0;
		inputs[(size_t)APUSim::APU_Input::CLK] = CLK;
		apu->sim(inputs, outputs, &data_bus, &addr_bus, aux);
		CLK = NOT(CLK);
	}

	auto stamp2 = GetTickCount64();
	int delta = (int)(stamp2 - stamp1);

	printf("APU+Core executed %zd cycles (%d simulated msec) in real %d msec\n", desired_clk, SIMULATE_MSEC, delta);

	if (delta > SIMULATE_MSEC)
	{
		printf("You're %.2f times slower :(\n", (float)delta / (float)SIMULATE_MSEC);
	}
	else
	{
		printf("You're %.2f times faster :)\n", (float)SIMULATE_MSEC / (float)delta);
	}

	return delta <= SIMULATE_MSEC;
}

int main()
{
	core = new M6502Core::M6502(true, true);
#if FAST_APU
	apu = new FastAPU::FastAPU(core, APUSim::Revision::RP2A03G);
#else
	apu = new APUSim::APU(core, APUSim::Revision::RP2A03G);
#endif

	size_t clks = (ONE_SECOND / 1000) * SIMULATE_MSEC;

	std::cout << "ApuPumpkin Start\n";
	bool res = ApuMegaCyclesTest(clks);
	std::cout << "ApuPumpkin Stop\n";

	delete apu;
	delete core;
	return res ? 0 : -1;
}
