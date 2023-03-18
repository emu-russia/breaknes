// Compare the time of the simulated PPU with the real time.

#include "pch.h"

using namespace BaseLogic;
using namespace PPUSim;

#define ONE_SECOND 21'477272		// 2C02-G

// How long to simulate PPU

#ifdef _DEBUG
#define SIMULATE_MSEC 10
#else
#define SIMULATE_MSEC 1000
#endif

PPUSim::PPU* ppu = nullptr;

static bool PpuMegaCyclesTest(size_t desired_clk)
{
	char text[0x100]{};
	TriState CLK = TriState::Zero;

	uint8_t data_bus = 0;
	uint8_t ext_bus = 0;
	uint8_t ad_bus = 0;
	uint8_t addrHi_bus = 0;
	PPUSim::VideoOutSignal vout{};

	TriState inputs[(size_t)PPUSim::InputPad::Max]{};
	TriState outputs[(size_t)PPUSim::OutputPad::Max]{};

	inputs[(size_t)InputPad::n_RES] = TriState::One;		// Not neccessary
	inputs[(size_t)InputPad::n_DBE] = TriState::One;		// CPU I/F disabled
	inputs[(size_t)InputPad::RnW] = TriState::One;
	inputs[(size_t)InputPad::RS0] = TriState::Zero;
	inputs[(size_t)InputPad::RS1] = TriState::Zero;
	inputs[(size_t)InputPad::RS2] = TriState::Zero;

	// Enable forced rendering. With rendering enabled, all blocks of the PPU will be engaged

	ppu->Dbg_RenderAlwaysEnabled(true);

	// Continue

	auto stamp1 = GetTickCount64();

	for (size_t n = 0; n < desired_clk; n++)
	{
		ad_bus = 0;
		inputs[(size_t)InputPad::CLK] = CLK;
		ppu->sim(inputs, outputs, &ext_bus, &data_bus, &ad_bus, &addrHi_bus, vout);
		CLK = NOT(CLK);

		ad_bus = 0;
		inputs[(size_t)InputPad::CLK] = CLK;
		ppu->sim(inputs, outputs, &ext_bus, &data_bus, &ad_bus, &addrHi_bus, vout);
		CLK = NOT(CLK);
	}

	auto stamp2 = GetTickCount64();
	int delta = (int)(stamp2 - stamp1);

	printf ("PPU executed %zd cycles (%d simulated msec) in real %d msec\n", desired_clk, SIMULATE_MSEC, delta);

	if (delta > SIMULATE_MSEC)
	{
		printf ("You're %.2f times slower :(\n", (float)delta / (float)SIMULATE_MSEC);
	}

	return delta <= SIMULATE_MSEC;
}

int main()
{
	ppu = new PPUSim::PPU(PPUSim::Revision::RP2C02G);

	size_t clks = (ONE_SECOND / 1000) * SIMULATE_MSEC;

	std::cout << "PpuPumpkin Start\n";
	bool res = PpuMegaCyclesTest(clks);
	std::cout << "PpuPumpkin Stop\n";

	delete ppu;
	return res ? 0 : -1;
}
